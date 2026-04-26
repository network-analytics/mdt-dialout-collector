// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include "cfg_handler.h"

#include <initializer_list>


std::map<std::string, std::string> logs_cfg_parameters;
std::map<std::string, std::string> main_cfg_parameters;
std::map<std::string, std::string> data_manipulation_cfg_parameters;
std::map<std::string, std::string> kafka_delivery_cfg_parameters;
std::map<std::string, std::string> zmq_delivery_cfg_parameters;


namespace {

using ParamsMap = std::map<std::string, std::string>;

// Reads a string-typed setting. If the key is absent returns true and
// leaves *out unchanged. On type error logs and returns false.
bool ReadStringSetting(libconfig::Config &cfg, const char *key,
    const std::string &logger, std::string *out)
{
    if (!cfg.exists(key)) {
        return true;
    }
    try {
        *out = (const char *)cfg.lookup(key);
        return true;
    } catch (const libconfig::SettingTypeException &e) {
        spdlog::get(logger)->error(
            "[{}] configuration issue: {}", key, e.what());
        return false;
    }
}

// Optional key with default. Inserts the parsed value if present and
// non-empty, else the default. Empty value with the key present is an
// error. Set logger to "multi-logger-boot" before LogsHandler swaps in
// the post-cfg sinks; "multi-logger" afterwards.
bool LoadOptional(libconfig::Config &cfg, ParamsMap &params,
    const char *key, const char *default_value,
    const std::string &logger = "multi-logger")
{
    std::string value;
    if (!ReadStringSetting(cfg, key, logger, &value)) {
        return false;
    }
    if (cfg.exists(key)) {
        if (value.empty()) {
            spdlog::get(logger)->error(
                "[{}] configuration issue: [ {} ] is invalid", key, value);
            return false;
        }
        params.emplace(key, value);
    } else {
        params.emplace(key, default_value);
    }
    return true;
}

// Optional key constrained to a fixed set of accepted values.
bool LoadOptionalEnum(libconfig::Config &cfg, ParamsMap &params,
    const char *key, const char *default_value,
    std::initializer_list<const char *> allowed,
    const std::string &logger = "multi-logger")
{
    std::string value;
    if (!ReadStringSetting(cfg, key, logger, &value)) {
        return false;
    }
    if (cfg.exists(key)) {
        bool ok = false;
        for (const char *a : allowed) {
            if (value == a) { ok = true; break; }
        }
        if (!ok) {
            spdlog::get(logger)->error(
                "[{}] configuration issue: [ {} ] is invalid", key, value);
            return false;
        }
        params.emplace(key, value);
    } else {
        params.emplace(key, default_value);
    }
    return true;
}

// Mandatory key. Returns false if absent, type-error, or empty.
bool LoadMandatory(libconfig::Config &cfg, ParamsMap &params,
    const char *key, const std::string &logger = "multi-logger")
{
    if (!cfg.exists(key)) {
        spdlog::get(logger)->error(
            "[{}] configuration issue: a value is mandatory", key);
        return false;
    }
    std::string value;
    if (!ReadStringSetting(cfg, key, logger, &value)) {
        return false;
    }
    if (value.empty()) {
        spdlog::get(logger)->error(
            "[{}] configuration issue: [ {} ] is invalid", key, value);
        return false;
    }
    params.emplace(key, value);
    return true;
}

// Optional path key (absent → use default; present → must point to an
// existing filesystem entry).
bool LoadOptionalPath(libconfig::Config &cfg, ParamsMap &params,
    const char *key, const char *default_value,
    const std::string &logger = "multi-logger")
{
    std::string value;
    if (!ReadStringSetting(cfg, key, logger, &value)) {
        return false;
    }
    const std::string chosen = cfg.exists(key) ? value : default_value;
    if (chosen.empty() || !std::filesystem::exists(chosen)) {
        spdlog::get(logger)->error(
            "[{}] configuration issue: [ {} ] is invalid", key, chosen);
        return false;
    }
    params.emplace(key, chosen);
    return true;
}

}  // namespace


bool CfgHandler::set_parameters(libconfig::Config &params,
    const std::string &cfg_path)
{
    try {
        params.readFile(cfg_path.c_str());
    } catch (const libconfig::FileIOException &fioex) {
        spdlog::get("multi-logger-boot")->
            error("configuration file issues: {}", fioex.what());
        return false;
    } catch (const libconfig::ParseException &pex) {
        spdlog::get("multi-logger-boot")->
            error("configuration file issues: {}", pex.what());
        return false;
    }

    return true;
}

bool LogsCfgHandler::lookup_logs_parameters(const std::string &cfg_path,
    std::map<std::string, std::string> &params)
{
    libconfig::Config logs_params;
    if (!CfgHandler::set_parameters(logs_params, cfg_path)) {
        return false;
    }
    params.clear();

    // multi-logger isn't registered yet — log via the boot logger.
    const std::string log = "multi-logger-boot";

    if (!LoadOptional(logs_params, params, "syslog", "false", log)) {
        return false;
    }
    if (params.at("syslog") == "true") {
        if (!LoadOptionalEnum(logs_params, params, "syslog_facility",
                "LOG_USER",
                {"LOG_DAEMON", "LOG_USER",
                 "LOG_LOCAL0", "LOG_LOCAL1", "LOG_LOCAL2", "LOG_LOCAL3",
                 "LOG_LOCAL4", "LOG_LOCAL5", "LOG_LOCAL6", "LOG_LOCAL7"},
                log) ||
            !LoadOptional(logs_params, params, "syslog_ident",
                "mdt-dialout-collector", log)) {
            return false;
        }
    } else {
        params.emplace("syslog_facility", "NONE");
        params.emplace("syslog_ident",    "NONE");
    }
    if (!LoadOptional(logs_params, params, "console_log", "true", log)) {
        return false;
    }
    if (!LoadOptionalEnum(logs_params, params, "spdlog_level", "info",
            {"debug", "info", "warn", "error", "off"}, log)) {
        return false;
    }
    return true;
}

bool MainCfgHandler::lookup_main_parameters(const std::string &cfg_path,
    std::map<std::string, std::string> &params)
{
    libconfig::Config main_params;
    if (!CfgHandler::set_parameters(main_params, cfg_path)) {
        return false;
    }
    params.clear();

    if (!LoadOptional(main_params, params, "writer_id",
            "mdt-dialout-collector") ||
        !LoadOptionalPath(main_params, params, "core_pid_folder",
            "/var/run/") ||
        !LoadMandatory(main_params, params, "iface") ||
        !LoadOptionalEnum(main_params, params, "so_bindtodevice_check",
            "true", {"true", "false"}) ||
        !LoadOptionalEnum(main_params, params, "enable_tls", "false",
            {"true", "false"})) {
        return false;
    }

    // TLS cert/key are coupled to enable_tls; validate together.
    std::string tls_cert, tls_key;
    if (params.at("enable_tls") == "true") {
        if (!main_params.exists("tls_cert_path") ||
            !main_params.exists("tls_key_path")) {
            spdlog::get("multi-logger")->error(
                "[enable_tls=true] requires tls_cert_path and tls_key_path");
            return false;
        }
        try {
            tls_cert = (const char *)main_params.lookup("tls_cert_path");
            tls_key  = (const char *)main_params.lookup("tls_key_path");
        } catch (const libconfig::SettingTypeException &e) {
            spdlog::get("multi-logger")->error(
                "[tls_cert_path/tls_key_path] type error: {}", e.what());
            return false;
        }
        if (!std::filesystem::exists(tls_cert) ||
            !std::filesystem::exists(tls_key)) {
            spdlog::get("multi-logger")->error(
                "[tls_cert_path/tls_key_path] file not found "
                "(cert='{}' key='{}')", tls_cert, tls_key);
            return false;
        }
    }
    params.emplace("tls_cert_path", tls_cert);
    params.emplace("tls_key_path",  tls_key);

    // Per-vendor listening sockets: absent → empty (vendor disabled).
    // Accepts both IPv4 and IPv6 forms — gRPC parses "0.0.0.0:10007",
    // "[::]:10007" and "[2001:db8::1]:10007" natively. The legacy
    // ipv4_socket_<vendor> name is honored as a deprecated alias when
    // the new socket_<vendor> is not provided.
    struct VendorKeys {
        const char *socket;
        const char *socket_legacy;
        const char *replies;
        const char *workers;
    };
    constexpr VendorKeys vendors[] = {
        {"socket_cisco",   "ipv4_socket_cisco",
            "replies_cisco",   "cisco_workers"},
        {"socket_juniper", "ipv4_socket_juniper",
            "replies_juniper", "juniper_workers"},
        {"socket_nokia",   "ipv4_socket_nokia",
            "replies_nokia",   "nokia_workers"},
        {"socket_huawei",  "ipv4_socket_huawei",
            "replies_huawei",  "huawei_workers"},
    };
    for (const auto &v : vendors) {
        if (main_params.exists(v.socket)) {
            if (!LoadOptional(main_params, params, v.socket, "")) {
                return false;
            }
        } else if (main_params.exists(v.socket_legacy)) {
            // Read legacy key, store under new name; warn once per vendor.
            spdlog::get("multi-logger")->warn(
                "[{}] is deprecated; rename to [{}]",
                v.socket_legacy, v.socket);
            std::string value;
            try {
                value = (const char *)main_params.lookup(v.socket_legacy);
            } catch (const libconfig::SettingTypeException &e) {
                spdlog::get("multi-logger")->error(
                    "[{}] configuration issue: {}", v.socket_legacy, e.what());
                return false;
            }
            params.emplace(v.socket, value);
        } else {
            params.emplace(v.socket, "");
        }
    }

    // Per-vendor replies / workers: only loaded when the corresponding
    // socket_<vendor> is configured.
    for (const auto &v : vendors) {
        if (params.at(v.socket).empty()) continue;
        if (!LoadOptional(main_params, params, v.replies, "0") ||
            !LoadOptional(main_params, params, v.workers, "1")) {
            return false;
        }
    }

    if (!LoadOptionalEnum(main_params, params, "data_delivery_method",
            "kafka", {"kafka", "zmq"})) {
        return false;
    }
    return true;
}

bool DataManipulationCfgHandler::lookup_data_manipulation_parameters(
    const std::string &cfg_path,
    std::map<std::string, std::string> &params)
{
    libconfig::Config dmp;
    if (!CfgHandler::set_parameters(dmp, cfg_path)) {
        return false;
    }
    params.clear();

    if (!LoadOptional(dmp, params, "enable_cisco_message_to_json_string",
            "false") ||
        !LoadOptional(dmp, params, "enable_cisco_gpbkv2json", "true")) {
        return false;
    }
    // The two cisco JSON paths are mutually exclusive — they must differ.
    if (params.at("enable_cisco_message_to_json_string") ==
        params.at("enable_cisco_gpbkv2json")) {
        spdlog::get("multi-logger")->error(
            "[enable_cisco_gpbkv2json] XOR "
            "[enable_cisco_message_to_json_string]");
        return false;
    }

    if (!LoadOptional(dmp, params, "enable_label_encode_as_map", "false")) {
        return false;
    }
    if (params.at("enable_label_encode_as_map") == "true") {
        if (!LoadOptionalPath(dmp, params, "label_map_csv_path",
                "/opt/mdt_dialout_collector/csv/label_map.csv")) {
            return false;
        }
    }

    if (!LoadOptional(dmp, params, "enable_label_encode_as_map_ptm", "false")) {
        return false;
    }
    if (params.at("enable_label_encode_as_map_ptm") == "true") {
        if (!LoadOptionalPath(dmp, params, "label_map_ptm_path",
                "/opt/mdt_dialout_collector/ptm/label_map.ptm")) {
            return false;
        }
    }

    // The two label-map encoders are mutually exclusive — at most one true.
    if (params.at("enable_label_encode_as_map") == "true" &&
        params.at("enable_label_encode_as_map_ptm") == "true") {
        spdlog::get("multi-logger")->error(
            "[enable_label_encode_as_map] XOR "
            "[enable_label_encode_as_map_ptm]");
        return false;
    }
    return true;
}

bool KafkaCfgHandler::lookup_kafka_parameters(const std::string &cfg_path,
    std::map<std::string, std::string> &params)
{
    libconfig::Config kp;
    if (!CfgHandler::set_parameters(kp, cfg_path)) {
        return false;
    }
    params.clear();

    // topic / bootstrap_servers / security_protocol are mandatory when
    // delivery is kafka, but get a dummy when delivery is zmq (downstream
    // code unconditionally reads these keys, so they must always exist).
    const bool kafka_mode =
        main_cfg_parameters.at("data_delivery_method") == "kafka";
    auto load_kafka_only = [&](const char *key, const char *dummy) {
        if (kp.exists(key) || kafka_mode) {
            return LoadMandatory(kp, params, key);
        }
        params.emplace(key, dummy);
        return true;
    };
    if (!load_kafka_only("topic",             "dummy_topic") ||
        !load_kafka_only("bootstrap_servers", "dummy_servers")) {
        return false;
    }

    if (!LoadOptionalEnum(kp, params, "enable_idempotence", "true",
            {"true", "false"}) ||
        !LoadOptional(kp, params, "client_id", "mdt-dialout-collector") ||
        !LoadOptional(kp, params, "log_level", "6")) {
        return false;
    }

    if (!load_kafka_only("security_protocol", "dummy_security_protocol")) {
        return false;
    }
    if (params.at("security_protocol") != "dummy_security_protocol" &&
        params.at("security_protocol") != "ssl" &&
        params.at("security_protocol") != "plaintext") {
        spdlog::get("multi-logger")->error(
            "[security_protocol] configuration issue: [ {} ] is invalid "
            "(ssl or plaintext)", params.at("security_protocol"));
        return false;
    }

    if (params.at("security_protocol") != "ssl") {
        params.emplace("ssl_key_location",         "NULL");
        params.emplace("ssl_key_password",         "NULL");
        params.emplace("ssl_certificate_location", "NULL");
        params.emplace("ssl_ca_location",          "NULL");
        params.emplace("enable_ssl_certificate_verification", "false");
        return true;
    }

    // SSL: key/cert/ca are mandatory; password and verify default safely.
    if (!LoadMandatory(kp, params, "ssl_key_location") ||
        !LoadMandatory(kp, params, "ssl_certificate_location") ||
        !LoadMandatory(kp, params, "ssl_ca_location") ||
        !LoadOptional(kp, params, "ssl_key_password", "NULL")) {
        return false;
    }
    if (kp.exists("enable_ssl_certificate_verification")) {
        if (!LoadOptional(kp, params,
                "enable_ssl_certificate_verification", "true")) {
            return false;
        }
    } else {
        // Secure-by-default when SSL is configured.
        spdlog::get("multi-logger")->warn("[security_protocol] "
            "enable_ssl_certificate_verification not set; defaulting to true");
        params.emplace("enable_ssl_certificate_verification", "true");
    }
    return true;
}

bool ZmqCfgHandler::lookup_zmq_parameters(const std::string &zmq_uri,
    std::map<std::string, std::string> &params)
{
    if (main_cfg_parameters.at("data_delivery_method") != "zmq") {
        params.emplace("zmq_uri", "ipc:///tmp/dummy.sock");
        return true;
    }
    const std::string uri =
        zmq_uri.empty() ? "ipc:///tmp/grpc.sock" : zmq_uri;
    params.emplace("zmq_uri", uri);
    spdlog::get("multi-logger")->info("[zmq_uri] set to {}", uri);
    return true;
}

