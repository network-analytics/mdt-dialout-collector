// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "cfg_handler.h"


// Centralizing config parameters
std::unique_ptr<LogsCfgHandler>
    logs_cfg_handler(new LogsCfgHandler());
std::unique_ptr<MainCfgHandler>
    main_cfg_handler(new MainCfgHandler());
std::unique_ptr<DataManipulationCfgHandler>
    data_manipulation_cfg_handler(new DataManipulationCfgHandler());
std::unique_ptr<KafkaCfgHandler>
    data_delivery_cfg_handler(new KafkaCfgHandler());

std::map<std::string, std::string> logs_cfg_parameters =
    logs_cfg_handler->get_logs_parameters();
std::map<std::string, std::string> main_cfg_parameters =
    main_cfg_handler->get_main_parameters();
std::map<std::string, std::string> data_manipulation_cfg_parameters =
    data_manipulation_cfg_handler->get_data_manipulation_parameters();
std::map<std::string, std::string> data_delivery_cfg_parameters =
    data_delivery_cfg_handler->get_kafka_parameters();

bool LogsCfgHandler::set_boot_spdlog_sinks(
    std::shared_ptr<spdlog::logger> &multi_logger_boot)
{
    std::vector<spdlog::sink_ptr> spdlog_sinks;
    std::string spdlog_level = "debug";

    // Syslog
    const std::string ident = "mdt-dialout-collector";
    try {
        auto spdlog_syslog =
            std::make_shared<spdlog::sinks::syslog_sink_mt>(
                ident, 0, LOG_USER, true);
        spdlog_sinks.push_back(spdlog_syslog);
    } catch (const spdlog::spdlog_ex &sex) {
        std::cout << "spdlog, syslog: " << sex.what() << "\n";
        return false;
    }

    // ConsoleLog
    try {
        auto spdlog_console =
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        spdlog_sinks.push_back(spdlog_console);
    } catch (const spdlog::spdlog_ex &sex) {
        std::cout << "spdlog, console: " << sex.what() << "\n";
        return false;
    }

    multi_logger_boot = std::make_shared<spdlog::logger>
        ("multi-logger-boot", begin(spdlog_sinks), end(spdlog_sinks));
    multi_logger_boot->set_level(spdlog::level::from_str(spdlog_level));
    spdlog::register_logger(multi_logger_boot);

    return true;
}

bool CfgHandler::set_cfg_spdlog_sinks(
    std::shared_ptr<spdlog::logger> &multi_logger)
{
    std::map<std::string, std::string> logs_cfg_parameters =
        logs_cfg_handler->get_logs_parameters();
    std::vector<spdlog::sink_ptr> spdlog_sinks;
    std::string spdlog_level = logs_cfg_parameters.at("spdlog_level");

    // Syslog
    if (logs_cfg_parameters.at("syslog").compare("true") == 0) {
        const std::string ident = "mdt-dialout-collector";
        try {
            auto spdlog_syslog =
                std::make_shared<spdlog::sinks::syslog_sink_mt>(
                    ident, 0, LOG_USER, true);
            spdlog_sinks.push_back(spdlog_syslog);
        } catch (const spdlog::spdlog_ex &sex) {
            std::cout << "spdlog, syslog: " << sex.what() << "\n";
            return false;
        }
    }

    // ConsoleLog
    if (logs_cfg_parameters.at("console_log").compare("true") == 0){
        try {
            auto spdlog_console =
                std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            spdlog_sinks.push_back(spdlog_console);
        } catch (const spdlog::spdlog_ex &sex) {
            std::cout << "spdlog, console: " << sex.what() << "\n";
            return false;
        }
    }

    multi_logger_cfg = std::make_shared<spdlog::logger>
        ("multi-logger-cfg", begin(spdlog_sinks), end(spdlog_sinks));
    multi_logger_cfg->set_level(spdlog::level::from_str(spdlog_level));
    spdlog::drop_all();
    spdlog::register_logger(multi_logger_cfg);

    return true;
}

bool LogsCfgHandler::set_parameters(std::unique_ptr<libconfig::Config> &params,
    const std::string &cfg_path)
{
    try {
        params->readFile(cfg_path.c_str());
    } catch (const libconfig::FileIOException &fioex) {
        // check if the cfg exists
        spdlog::get("multi-logger-boot")->
            error("configuration file issues: {}", fioex.what());
        return false;
    } catch (const libconfig::ParseException &pex) {
        // check it cfg is parsable
        spdlog::get("multi-logger-boot")->
            error("configuration file issues: {}", pex.what());
        return false;
    }

    return true;
}

bool CfgHandler::set_parameters(std::unique_ptr<libconfig::Config> &params,
    const std::string &cfg_path)
{
    try {
        params->readFile(cfg_path.c_str());
    } catch (const libconfig::FileIOException &fioex) {
        // check if the cfg exists
        spdlog::get("multi-logger-cfg")->
            error("configuration file issues: {}", fioex.what());
        return false;
    } catch (const libconfig::ParseException &pex) {
        // check it cfg is parsable
        spdlog::get("multi-logger-cfg")->
            error("configuration file issues: {}", pex.what());
        return false;
    }

    return true;
}

LogsCfgHandler::LogsCfgHandler()
{
    if (set_boot_spdlog_sinks(
            this->multi_logger_boot) == false) {
        std::cout << "Unable to LogsCfgHandler::set_pre_spdlog_sinks(...)\n";
        std::exit(EXIT_FAILURE);
    } else {
        spdlog::get("multi-logger-boot")->
            info("constructor: LogsCfgHandler()");
        std::unique_ptr<libconfig::Config>
            logs_params(new libconfig::Config());
        if (lookup_logs_parameters(
            this->mdt_dialout_collector_conf,
            this->logs_parameters) == true) {

            //this->syslog = parameters.at("syslog");
            //this->console_log = parameters.at("console_log");
            //this->spdlog_level = parameters.at("log_level");
        } else {
            std::exit(EXIT_FAILURE);
        }
    }
}

CfgHandler::CfgHandler()
{
    if (set_cfg_spdlog_sinks(
            this->multi_logger_cfg) == false) {
        std::cout << "Unable to CfgHandler::set_spdlog_sinks(...)\n";
        std::exit(EXIT_FAILURE);
    } else {
        spdlog::get("multi-logger-cfg")->debug("constructor: CfgHandler()");
    }
}

bool LogsCfgHandler::lookup_logs_parameters(const std::string &cfg_path,
    std::map<std::string, std::string> &params)
{
    params.clear();
    std::unique_ptr<libconfig::Config> logs_params(new libconfig::Config());

    if (LogsCfgHandler::set_parameters(logs_params, cfg_path) == false) {
        return false;
    }

    // Logs parameters evaluation
    bool syslog = logs_params->exists("syslog");
    if (syslog == true) {
        libconfig::Setting &syslog =
            logs_params->lookup("syslog");
        try {
            std::string syslog_s = syslog;
            if (syslog_s.empty() == false) {
                params.insert({"syslog", syslog_s});
            } else {
                spdlog::get("multi-logger-boot")->
                    error("[syslog] configuration "
                    "issue: [ {} ] is invalid", syslog_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-boot")->
                error("[syslog] configuration issue: {}", ste.what());
            return false;
        }
    } else {
        params.insert({"syslog", "false"});
    }

    bool console_log = logs_params->exists("console_log");
    if (console_log == true) {
        libconfig::Setting &console_log =
            logs_params->lookup("console_log");
        try {
            std::string console_log_s = console_log;
            if (console_log_s.empty() == false) {
                params.insert({"console_log", console_log_s});
            } else {
                spdlog::get("multi-logger-boot")->
                    error("[console_log] configuration "
                    "issue: [ {} ] is invalid", console_log_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-boot")->
                error("[console_log] configuration issue: "
                "{}", ste.what());
            return false;
        }
    } else {
        params.insert({"console_log", "true"});
    }

    bool spdlog_level = logs_params->exists("spdlog_level");
    if (spdlog_level == true) {
        libconfig::Setting &spdlog_level =
            logs_params->lookup("spdlog_level");
        try {
            std::string spdlog_level_s = spdlog_level;
            if (spdlog_level_s.empty()          == false &&
               (spdlog_level_s.compare("debug") == 0 ||
                spdlog_level_s.compare("info")  == 0 ||
                spdlog_level_s.compare("warn")  == 0 ||
                spdlog_level_s.compare("error") == 0 ||
                spdlog_level_s.compare("off")   == 0)) {
                params.insert({"spdlog_level", spdlog_level_s});
            } else {
                spdlog::get("multi-logger-boot")->
                    error("[spdlog_level] configuration "
                    "issue: [ {} ] is an invalid severity level",
                    spdlog_level_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-boot")->
                error("[spdlog_level] configuration issue: "
                "{}", ste.what());
            return false;
        }
    } else {
        params.insert({"spdlog_level", "info"});
    }

    return true;
}

MainCfgHandler::MainCfgHandler()
{
    spdlog::get("multi-logger-cfg")->debug("constructor: MainCfgHandler()");
    std::unique_ptr<libconfig::Config> main_params(new libconfig::Config());
    if (lookup_main_parameters(
        this->mdt_dialout_collector_conf,
        this->main_parameters) == true) {

        //this->writer_id = parameters.at("writer_id");
        //this->core_pid_folder = parameters.at("core_pid_folder");
        //this->iface = parameters.at("iface");
        //this->ipv4_socket_cisco = parameters.at("ipv4_socket_cisco");
        //this->ipv4_socket_juniper = parameters.at("ipv4_socket_juniper");
        //this->ipv4_socket_huawei = parameters.at("ipv4_socket_huawei");
        //this->replies_cisco = parameters.at("replies_cisco");
        //this->replies_juniper = parameters.at("replies_juniper");
        //this->replies_huawei = parameters.at("replies_huawei");
        //this->cisco_workers = parameters.at("cisco_workers");
        //this->juniper_workers = parameters.at("juniper_workers");
        //this->huawei_workers = parameters.at("huawei_workers");
    } else {
        std::exit(EXIT_FAILURE);
    }
}

bool MainCfgHandler::lookup_main_parameters(const std::string &cfg_path,
    std::map<std::string, std::string> &params)
{
    params.clear();
    std::unique_ptr<libconfig::Config> main_params(new libconfig::Config());

    if (CfgHandler::set_parameters(main_params, cfg_path) == false) {
        return false;
    }

    // Main parameters evaluation
    bool writer_id = main_params->exists("writer_id");
    if (writer_id == true) {
        libconfig::Setting &writer_id =
            main_params->lookup("writer_id");
        try {
            std::string writer_id_s = writer_id;
            if (writer_id_s.empty() == false) {
                params.insert({"writer_id", writer_id_s});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[writer_id] configuration issue: "
                    "[ {} ] is an invalid writer_id", writer_id_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-cfg")->
                error("[writer_id] configuration issue: "
                "{}", ste.what());
            return false;
        }
    } else {
        params.insert({"writer_id", "mdt-dialout-collector"});
    }

    // core_pid_folder: hidden option
    bool core_pid_folder =
        main_params->exists("core_pid_folder");
    if (core_pid_folder == true) {
        libconfig::Setting &core_pid_folder =
            main_params->lookup("core_pid_folder");
        try {
            std::string core_pid_folder_s = core_pid_folder;
            if (core_pid_folder_s.empty() == false &&
                std::filesystem::exists(core_pid_folder_s) == true) {
                params.insert({"core_pid_folder", core_pid_folder_s});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[core_pid_folder] configuration issue: "
                    "[ {} ] is an invalid folder", core_pid_folder_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-cfg")->
                error("[core_pid_folder] configuration issue: "
                "{}", ste.what());
            return false;
        }
    } else {
        const std::string default_core_pid_folder = "/var/run/";
        if (std::filesystem::exists(default_core_pid_folder) == true) {
            params.insert({"core_pid_folder", default_core_pid_folder});
        } else {
            spdlog::get("multi-logger-cfg")->
                error("[core_pid_folder] configuration issue: "
                "{} is an invalid folder", default_core_pid_folder);
            return false;
        }
    }

    bool iface = main_params->exists("iface");
    if (iface == true) {
        libconfig::Setting &iface = main_params->lookup("iface");
        try {
            std::string iface_s = iface;
            if (iface_s.empty() == false) {
                params.insert({"iface", iface_s});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[iface] configuration issue: "
                    "[ {} ] is an invalid inteface", iface_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-cfg")->
                error("[iface] configuration issue: "
                "{}", ste.what());
            return false;
        }
    } else {
        spdlog::get("multi-logger-cfg")->error("[iface] configuration issue: "
            "a valid inteface is mandatory");
        return false;
    }

    std::string ipv4_socket_cisco_s;
    bool ipv4_socket_cisco = main_params->exists("ipv4_socket_cisco");
    if (ipv4_socket_cisco == true) {
        libconfig::Setting &ipv4_socket_cisco =
            main_params->lookup("ipv4_socket_cisco");
        try {
            ipv4_socket_cisco_s = ipv4_socket_cisco.c_str();
            if (ipv4_socket_cisco_s.empty() == false) {
                params.insert({"ipv4_socket_cisco", ipv4_socket_cisco_s});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[ipv4_socket_cisco] configuration issue: "
                    "[ {} ] is an invalid socket", ipv4_socket_cisco_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-cfg")->
                error("[ipv4_socket_cisco] configuration issue: "
                "{}", ste.what());
            return false;
        }
    } else {
        params.insert({"ipv4_socket_cisco", ""});
    }

    std::string ipv4_socket_juniper_s;
    bool ipv4_socket_juniper = main_params->exists("ipv4_socket_juniper");
    if (ipv4_socket_juniper == true) {
        libconfig::Setting &ipv4_socket_juniper =
            main_params->lookup("ipv4_socket_juniper");
        try {
            ipv4_socket_juniper_s = ipv4_socket_juniper.c_str();
            if (ipv4_socket_juniper_s.empty() == false) {
                params.insert({"ipv4_socket_juniper", ipv4_socket_juniper_s});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[ipv4_socket_juniper] configuration "
                    "issue: [ {} ] is an invalid socket",
                    ipv4_socket_juniper_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-cfg")->
                error("[ipv4_socket_juniper] configuration issue: "
                "{}", ste.what());
            return false;
        }
    } else {
        params.insert({"ipv4_socket_juniper", ""});
    }

    std::string ipv4_socket_huawei_s;
    bool ipv4_socket_huawei = main_params->exists("ipv4_socket_huawei");
    if (ipv4_socket_huawei == true) {
        libconfig::Setting &ipv4_socket_huawei =
            main_params->lookup("ipv4_socket_huawei");
        try {
            ipv4_socket_huawei_s = ipv4_socket_huawei.c_str();
            if (ipv4_socket_huawei_s.empty() == false) {
                params.insert({"ipv4_socket_huawei", ipv4_socket_huawei_s});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[ipv4_socket_huawei] configuration "
                    "issue: [ {} ] is an invalid socket",
                    ipv4_socket_huawei_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-cfg")->
                error("[ipv4_socket_huawei] configuration issue: "
                "{}", ste.what());
            return false;
        }
    } else {
        params.insert({"ipv4_socket_huawei", ""});
    }

    if (ipv4_socket_cisco_s.empty() == false) {
        bool replies_cisco = main_params->exists("replies_cisco");
        if (replies_cisco == true) {
            libconfig::Setting &replies_cisco =
                main_params->lookup("replies_cisco");
            try {
                std::string replies_cisco_s = replies_cisco;
                if (replies_cisco_s.empty() == false) {
                    params.insert({"replies_cisco", replies_cisco_s});
                } else {
                    spdlog::get("multi-logger-cfg")->
                        error("[replies_cisco] configuration "
                        "issue: [ {} ] is an invalid # of replies",
                        replies_cisco_s);
                    return false;
                }
            } catch (const libconfig::SettingTypeException &ste) {
                spdlog::get("multi-logger-cfg")->
                    error("[replies_cisco] configuration issue: "
                    "{}", ste.what());
                return false;
            }
        } else {
            params.insert({"replies_cisco", "0"});
        }
    }

    if (ipv4_socket_juniper_s.empty() == false) {
        bool replies_juniper = main_params->exists("replies_juniper");
        if (replies_juniper == true) {
            libconfig::Setting &replies_juniper =
                main_params->lookup("replies_juniper");
            try {
                std::string replies_juniper_s = replies_juniper;
                if (replies_juniper_s.empty() == false) {
                    params.insert({"replies_juniper", replies_juniper_s});
                } else {
                    spdlog::get("multi-logger-cfg")->
                        error("[replies_juniper] configuration "
                        "issue: [ {} ] is an invalid # of replies",
                        replies_juniper_s);
                    return false;
                }
            } catch (const libconfig::SettingTypeException &ste) {
                spdlog::get("multi-logger-cfg")->
                    error("[replies_juniper] configuration issue: "
                    "{}", ste.what());
                return false;
            }
        } else {
            params.insert({"replies_juniper", "0"});
        }
    }

    if (ipv4_socket_huawei_s.empty() == false) {
        bool replies_huawei = main_params->exists("replies_huawei");
        if (replies_huawei == true) {
            libconfig::Setting &replies_huawei =
                main_params->lookup("replies_huawei");
            try {
                std::string replies_huawei_s = replies_huawei;
                if (replies_huawei_s.empty() == false) {
                    params.insert({"replies_huawei", replies_huawei_s});
                } else {
                    spdlog::get("multi-logger-cfg")->
                        error("[replies_huawei] configuration "
                        "issue: [ {} ] is an invalid # of replies",
                        replies_huawei_s);
                    return false;
                }
            } catch (const libconfig::SettingTypeException &ste) {
                spdlog::get("multi-logger-cfg")->
                    error("[replies_huawei] configuration issue: "
                    "{}", ste.what());
                return false;
            }
        } else {
            params.insert({"replies_huawei", "0"});
        }
    }

    if (ipv4_socket_cisco_s.empty() == false) {
        bool cisco_workers = main_params->exists("cisco_workers");
        if (cisco_workers == true) {
            libconfig::Setting &cisco_workers =
                main_params->lookup("cisco_workers");
            try {
                std::string cisco_workers_s = cisco_workers;
                if (cisco_workers_s.empty() == false) {
                    params.insert({"cisco_workers", cisco_workers_s});
                } else {
                    spdlog::get("multi-logger-cfg")->
                        error("[cisco_workers] configuration "
                        "issue: [ {} ] is an invalid # of replies",
                        cisco_workers_s);
                    return false;
                }
            } catch (const libconfig::SettingTypeException &ste) {
                spdlog::get("multi-logger-cfg")->
                    error("[cisco_workers] configuration issue: "
                    "{}", ste.what());
                return false;
            }
        } else {
            params.insert({"cisco_workers", "1"});
        }
    }

    if (ipv4_socket_juniper_s.empty() == false) {
        bool juniper_workers = main_params->exists("juniper_workers");
        if (juniper_workers == true) {
            libconfig::Setting &juniper_workers =
                main_params->lookup("juniper_workers");
            try {
                std::string juniper_workers_s = juniper_workers;
                if (juniper_workers_s.empty() == false) {
                    params.insert({"juniper_workers", juniper_workers_s});
                } else {
                    spdlog::get("multi-logger-cfg")->
                        error("[juniper_workers] configuration "
                        "issue: [ {} ] is an invalid # of replies",
                        juniper_workers_s);
                    return false;
                }
            } catch (const libconfig::SettingTypeException &ste) {
                spdlog::get("multi-logger-cfg")->
                    error("[juniper_workers] configuration issue: "
                    "{}", ste.what());
                return false;
            }
        } else {
            params.insert({"juniper_workers", "1"});
        }
    }

    if (ipv4_socket_huawei_s.empty() == false) {
        bool huawei_workers = main_params->exists("huawei_workers");
        if (huawei_workers == true) {
            libconfig::Setting &huawei_workers =
                main_params->lookup("huawei_workers");
            try {
                std::string huawei_workers_s = huawei_workers;
                if (huawei_workers_s.empty() == false) {
                    params.insert({"huawei_workers", huawei_workers_s});
                } else {
                    spdlog::get("multi-logger-cfg")->
                        error("[huawei_workers] configuration "
                        "issue: [ {} ] is an invalid # of replies",
                        huawei_workers_s);
                    return false;
                }
            } catch (const libconfig::SettingTypeException &ste) {
                spdlog::get("multi-logger-cfg")->
                    error("[huawei_workers] configuration issue: "
                    "{}", ste.what());
                return false;
            }
        } else {
            params.insert({"huawei_workers", "1"});
        }
    }

    return true;
}

DataManipulationCfgHandler::DataManipulationCfgHandler()
{
    spdlog::get("multi-logger-cfg")->
        debug("constructor: DataManipulationCfgHandler()");
    if (lookup_data_manipulation_parameters(
        this->mdt_dialout_collector_conf,
        this->data_manipulation_parameters) == true) {

        //this->enable_cisco_message_to_json_string =
        //    parameters.at("enable_cisco_message_to_json_string");
        //this->enable_cisco_gpbkv2json =
        //    parameters.at("enable_cisco_gpbkv2json");
        //this->enable_label_encode_as_map =
        //    parameters.at("enable_label_encode_as_map");
        //this->label_map_csv_path =
        //    parameters.at("label_map_csv_path");
        //this->enable_label_encode_as_map_ptm =
        //    parameters.at("enable_label_encode_as_map_ptm");
        //this->label_map_ptm_path =
        //    parameters.at("label_map_ptm_path");
    } else {
        std::exit(EXIT_FAILURE);
    }
}

bool DataManipulationCfgHandler::lookup_data_manipulation_parameters(
    const std::string &cfg_path,
    std::map<std::string, std::string> &params)
{
    params.clear();
    std::unique_ptr<libconfig::Config>
        data_manipulation_params(new libconfig::Config());

    if (CfgHandler::set_parameters(data_manipulation_params, cfg_path)
        == false) {
            return false;
    }

    // Data manipulation parameters evaluation
    std::string enable_cisco_message_to_json_string_s;
    bool enable_cisco_message_to_json_string =
        data_manipulation_params->exists("enable_cisco_message_to_json_string");
    if (enable_cisco_message_to_json_string == true) {
        libconfig::Setting &enable_cisco_message_to_json_string =
            data_manipulation_params->lookup(
            "enable_cisco_message_to_json_string");
        try {
            enable_cisco_message_to_json_string_s =
                enable_cisco_message_to_json_string.c_str();
            if (enable_cisco_message_to_json_string_s.empty() == false) {
                params.insert({"enable_cisco_message_to_json_string",
                enable_cisco_message_to_json_string_s});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[enable_cisco_message_to_json_string] "
                    "configuration issue: [ {} ] is invalid",
                    enable_cisco_message_to_json_string_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-cfg")->
                error("[enable_cisco_message_to_json_string] "
                "configuration issue: {}", ste.what());
            return false;
        }
    } else {
        params.insert({"enable_cisco_message_to_json_string", "false"});
    }

    std::string enable_cisco_gpbkv2json_s;
    bool enable_cisco_gpbkv2json =
        data_manipulation_params->exists("enable_cisco_gpbkv2json");
    if (enable_cisco_gpbkv2json == true) {
        libconfig::Setting &enable_cisco_gpbkv2json =
            data_manipulation_params->lookup("enable_cisco_gpbkv2json");
        try {
            enable_cisco_gpbkv2json_s =
                enable_cisco_gpbkv2json.c_str();
            if (enable_cisco_gpbkv2json_s.empty() == false) {
                params.insert({"enable_cisco_gpbkv2json",
                enable_cisco_gpbkv2json_s});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[enable_cisco_gpbkv2json] "
                    "configuration issue: [ {} ] is invalid",
                    enable_cisco_gpbkv2json_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-cfg")->
                error("[enable_cisco_gpbkv2json] "
                "configuration issue: {}", ste.what());
            return false;
        }
    } else {
        params.insert({"enable_cisco_gpbkv2json", "true"});
    }

    // the two funcs above are in XOR
    if ((enable_cisco_message_to_json_string_s.compare(
        enable_cisco_gpbkv2json_s) == 0) &&
        (params.at("enable_cisco_message_to_json_string").compare(params.at(
            "enable_cisco_gpbkv2json"))) == 0) {
        spdlog::get("multi-logger-cfg")->
            error("[enable_cisco_gpbkv2json] XOR "
            "[enable_cisco_message_to_json_string]");
        return false;
    }

    std::string enable_label_encode_as_map_s;
    bool enable_label_encode_as_map =
        data_manipulation_params->exists("enable_label_encode_as_map");
    if (enable_label_encode_as_map == true) {
        libconfig::Setting &enable_label_encode_as_map =
            data_manipulation_params->lookup("enable_label_encode_as_map");
        try {
            enable_label_encode_as_map_s =
                enable_label_encode_as_map.c_str();
            if (enable_label_encode_as_map_s.empty() == false) {
                params.insert({"enable_label_encode_as_map",
                enable_label_encode_as_map_s});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[enable_label_encode_as_map] "
                    "configuration issue: [ {} ] is invalid",
                    enable_label_encode_as_map_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-cfg")->
                error("[enable_label_encode_as_map] "
                "configuration issue: {}", ste.what());
            return false;
        }
    } else {
        params.insert({"enable_label_encode_as_map", "false"});
    }

    if (enable_label_encode_as_map_s.compare("true") == 0) {
        bool label_map_csv_path =
            data_manipulation_params->exists("label_map_csv_path");
        if (label_map_csv_path == true) {
            libconfig::Setting &label_map_csv_path =
                data_manipulation_params->lookup("label_map_csv_path");
            try {
                std::string label_map_csv_path_s =
                    label_map_csv_path;
                if (label_map_csv_path_s.empty() == false &&
                    std::filesystem::exists(label_map_csv_path_s) == true) {
                    params.insert({"label_map_csv_path",
                    label_map_csv_path_s});
                } else {
                    spdlog::get("multi-logger-cfg")->
                        error("[label_map_csv_path] "
                        "configuration issue: [ {} ] is invalid",
                        label_map_csv_path_s);
                    return false;
                }
            } catch (const libconfig::SettingTypeException &ste) {
                spdlog::get("multi-logger-cfg")->error("[label_map_csv_path] "
                    "configuration issue: {}", ste.what());
                return false;
            }
        } else {
            const std::string default_label_map_csv_path =
                "/opt/mdt_dialout_collector/csv/label_map.csv";
            if (std::filesystem::exists(default_label_map_csv_path) == true) {
                params.insert({"label_map_csv_path",
                    default_label_map_csv_path});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[label_map_csv_path] configuration issue: {} "
                    "is invalid", default_label_map_csv_path);
                return false;
            }
        }
    }

    std::string enable_label_encode_as_map_ptm_s;
    bool enable_label_encode_as_map_ptm =
        data_manipulation_params->exists("enable_label_encode_as_map_ptm");
    if (enable_label_encode_as_map_ptm == true) {
        libconfig::Setting &enable_label_encode_as_map_ptm =
            data_manipulation_params->lookup("enable_label_encode_as_map_ptm");
        try {
            enable_label_encode_as_map_ptm_s =
                enable_label_encode_as_map_ptm.c_str();
            if (enable_label_encode_as_map_ptm_s.empty() == false) {
                params.insert({"enable_label_encode_as_map_ptm",
                enable_label_encode_as_map_ptm_s});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[enable_label_encode_as_map_ptm] "
                    "configuration issue: [ {} ] is invalid",
                    enable_label_encode_as_map_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-cfg")->
                error("[enable_label_encode_as_map_ptm] "
                "configuration issue: {}", ste.what());
            return false;
        }
    } else {
        params.insert({"enable_label_encode_as_map_ptm", "false"});
    }

    if (enable_label_encode_as_map_ptm_s.compare("true") == 0) {
        bool label_map_ptm_path =
            data_manipulation_params->exists("label_map_ptm_path");
        if (label_map_ptm_path == true) {
            libconfig::Setting &label_map_ptm_path =
                data_manipulation_params->lookup("label_map_ptm_path");
            try {
                std::string label_map_ptm_path_s =
                    label_map_ptm_path;
                if (label_map_ptm_path_s.empty() == false &&
                    std::filesystem::exists(label_map_ptm_path_s) == true) {
                    params.insert({"label_map_ptm_path",
                    label_map_ptm_path_s});
                } else {
                    spdlog::get("multi-logger-cfg")->
                        error("[label_map_ptm_path] "
                        "configuration issue: [ {} ] is invalid",
                        label_map_ptm_path_s);
                    return false;
                }
            } catch (const libconfig::SettingTypeException &ste) {
                spdlog::get("multi-logger-cfg")->error("[label_map_ptm_path] "
                    "configuration issue: {}", ste.what());
                return false;
            }
        } else {
            const std::string default_label_map_ptm_path =
                "/opt/mdt_dialout_collector/ptm/label_map.ptm";
            if (std::filesystem::exists(default_label_map_ptm_path) == true) {
                params.insert({"label_map_ptm_path",
                    default_label_map_ptm_path});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[label_map_ptm_path] configuration issue: {} "
                    "is invalid", default_label_map_ptm_path);
                return false;
            }
        }
    }

    // the two funcs above are in XOR
    if (enable_label_encode_as_map_s.compare("true") == 0 &&
        enable_label_encode_as_map_ptm_s.compare("true") == 0) {
        if ((enable_label_encode_as_map_s.compare(
            enable_label_encode_as_map_ptm_s) == 0) &&
            (params.at("enable_label_encode_as_map").compare(params.at(
                "enable_label_encode_as_map_ptm"))) == 0) {
            spdlog::get("multi-logger-cfg")->
                error("[enable_label_encode_as_map] XOR "
                "[enable_label_encode_as_map_ptm]");
            return false;
        }
    }

    return true;
}

KafkaCfgHandler::KafkaCfgHandler()
{
    spdlog::get("multi-logger-cfg")->debug("constructor: KafkaCfgHandler()");
    if (lookup_kafka_parameters(
        this->mdt_dialout_collector_conf,
        this->kafka_parameters) == true) {

        //this->topic = parameters.at("topic");
        //this->bootstrap_servers = parameters.at("bootstrap_servers");
        //this->enable_idempotence = parameters.at("enable_idempotence");
        //this->client_id = parameters.at("client_id");
        //this->security_protocol = parameters.at("security_protocol");
        //this->ssl_key_location = parameters.at("ssl_key_location");
        //this->ssl_certificate_location =
        //    parameters.at("ssl_certificate_location");
        //this->ssl_ca_location = parameters.at("ssl_ca_location");
        //this->log_level = parameters.at("log_level");
    } else {
        std::exit(EXIT_FAILURE);
    }
}

bool KafkaCfgHandler::lookup_kafka_parameters(const std::string &cfg_path,
    std::map<std::string, std::string> &params)
{
    params.clear();
    std::unique_ptr<libconfig::Config> kafka_params(new libconfig::Config());

    if (CfgHandler::set_parameters(kafka_params, cfg_path) == false) {
        return false;
    }

    // Kafka arameters evaluation
    bool topic = kafka_params->exists("topic");
    if (topic == true) {
        libconfig::Setting &topic = kafka_params->lookup("topic");
        try {
            std::string topic_s = topic.c_str();
            if (topic_s.empty() == false) {
                params.insert({"topic", topic_s});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[topic] configuration issue: [ {} ] "
                    "is invalid", topic_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-cfg")->error("[topic] "
                "configuration issue: {}", ste.what());
            return false;
        }
    } else {
        spdlog::get("multi-logger-cfg")->error("[topic] configuration issue: "
            "a valid topic is mandatory");
        return false;
    }

    bool bootstrap_servers = kafka_params->exists("bootstrap_servers");
    if (bootstrap_servers == true) {
        libconfig::Setting &bootstrap_servers =
            kafka_params->lookup("bootstrap_servers");
        try {
            std::string bootstrap_servers_s = bootstrap_servers.c_str();
            if (bootstrap_servers_s.empty() == false) {
                params.insert({"bootstrap_servers", bootstrap_servers_s});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[bootstrap_servers] configuration issue: "
                    "[ {} ] is invalid", bootstrap_servers_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-cfg")->error("[bootstrap_servers] "
                "configuration issue: {}", ste.what());
            return false;
        }
    } else {
        spdlog::get("multi-logger-cfg")->
            error("[bootstrap_servers] configuration issue: "
            "a valid bootstrap_servers is mandatory");
        return false;
    }

    bool enable_idempotence = kafka_params->exists("enable_idempotence");
    if (enable_idempotence == true) {
        libconfig::Setting &enable_idempotence =
            kafka_params->lookup("enable_idempotence");
        try {
            std::string enable_idempotence_s = enable_idempotence.c_str();
            if (enable_idempotence_s.empty() == false &&
                (enable_idempotence_s.compare("true") == 0 ||
                enable_idempotence_s.compare("false") == 0)) {
                params.insert({"enable_idempotence", enable_idempotence_s});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[enable_idempotence] configuration issue: "
                    "[ {} ] is invalid (true or false)", enable_idempotence_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-cfg")->error("[enable_idempotence] "
                "configuration issue: {}", ste.what());
            return false;
        }
    } else {
        params.insert({"enable_idempotence", "true"});
    }

    bool client_id = kafka_params->exists("client_id");
    if (client_id == true) {
        libconfig::Setting &client_id = kafka_params->lookup("client_id");
        try {
            std::string client_id_s = client_id.c_str();
            if (client_id_s.empty() == false) {
                params.insert({"client_id", client_id_s});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[client_id] configuration issue: "
                    "[ {} ] is invalid", client_id_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-cfg")->error("[client_id] "
                "configuration issue: {}", ste.what());
            return false;
        }
    } else {
        params.insert({"client_id", "mdt-dialout-collector"});
    }

    bool log_level = kafka_params->exists("log_level");
    if (log_level == true) {
        libconfig::Setting &log_level = kafka_params->lookup("log_level");
        try {
            std::string log_level_s = log_level.c_str();
            if (log_level_s.empty() == false) {
                params.insert({"log_level", log_level_s});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[log_level] configuration issue: "
                    "[ {} ] is invalid (0...7)", log_level_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-cfg")->error("[log_level] "
                "configuration issue: {}", ste.what());
            return false;
        }
    } else {
        params.insert({"log_level", "6"});
    }

    bool security_protocol = kafka_params->exists("security_protocol");
    if (security_protocol == true) {
        libconfig::Setting &security_protocol =
            kafka_params->lookup("security_protocol");
        try {
            std::string security_protocol_s = security_protocol.c_str();
            if (security_protocol_s.empty() == false &&
                (security_protocol_s.compare("ssl") == 0 ||
                security_protocol_s.compare("plaintext") == 0)) {
                params.insert({"security_protocol", security_protocol_s});
            } else {
                spdlog::get("multi-logger-cfg")->
                    error("[security_protocol] configuration issue: "
                    "[ {} ] is invalid (ssl or plaintext)", security_protocol_s);
                return false;
            }
        } catch (const libconfig::SettingTypeException &ste) {
            spdlog::get("multi-logger-cfg")->error("[security_protocol] "
                "configuration issue: {}", ste.what());
            return false;
        }
    } else {
        spdlog::get("multi-logger-cfg")->
            error("[security_protocol] configuration issue: "
            "a valid security_protocol is mandatory");
        return false;
    }

    if (params.at("security_protocol").compare("ssl") == 0) {
        bool ssl_key_location = kafka_params->exists("ssl_key_location");
        bool ssl_certificate_location =
            kafka_params->exists("ssl_certificate_location");
        bool ssl_ca_location = kafka_params->exists("ssl_ca_location");

        if (ssl_key_location == true &&
            ssl_certificate_location == true &&
            ssl_ca_location == true) {
            libconfig::Setting &ssl_key_location =
                kafka_params->lookup("ssl_key_location");
            libconfig::Setting &ssl_certificate_location =
                kafka_params->lookup("ssl_certificate_location");
            libconfig::Setting &ssl_ca_location =
                kafka_params->lookup("ssl_ca_location");
            try {
                std::string ssl_key_location_s = ssl_key_location.c_str();
                std::string ssl_certificate_location_s =
                    ssl_certificate_location.c_str();
                std::string ssl_ca_location_s = ssl_ca_location.c_str();
                if (ssl_key_location_s.empty() == false &&
                    ssl_certificate_location_s.empty() == false &&
                    ssl_ca_location_s.empty() == false) {
                    params.insert({"ssl_key_location", ssl_key_location_s});
                    params.insert({"ssl_certificate_location",
                        ssl_certificate_location_s});
                    params.insert({"ssl_ca_location", ssl_ca_location_s});
                } else {
                    spdlog::get("multi-logger-cfg")->
                        error("[security_protocol] "
                        "configuration issue: is invalid");
                    return false;
                }
            } catch (const libconfig::SettingTypeException &ste) {
                spdlog::get("multi-logger-cfg")->error("[security_protocol] "
                    "configuration issue: {}", ste.what());
                return false;
            }
        } else {
            spdlog::get("multi-logger-cfg")->
                error("[security_protocol] configuration issue: "
                "a valid security_protocol is mandatory");
            return false;
        }
    } else {
        params.insert({"ssl_key_location", "NULL"});
        params.insert({"ssl_certificate_location", "NULL"});
        params.insert({"ssl_ca_location", "NULL"});
    }

    return true;
}

