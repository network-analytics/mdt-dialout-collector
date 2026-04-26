// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include "grpc_collector_bridge.h"
#include "../core/mdt_dialout_core.h"
#include "../utils/cfg_handler.h"

#include <mutex>
#include <string>
#include <vector>


namespace {
std::mutex             g_workers_mutex;
std::vector<pthread_t> g_active_workers;
}  // namespace

extern "C" {
    Options *InitOptions()
    {
        // Options field names keep the legacy ipv4_socket_* form for
        // pmacct ABI compat — the cfg key was renamed to socket_<vendor>
        // in PR H, hence the asymmetric mapping for the four sockets.
        auto dup = [](const std::string &s) -> char * {
            return strndup(s.c_str(), s.size());
        };
        Options *opts = (Options *) malloc(sizeof(Options));
        opts->writer_id           = dup(main_cfg_parameters.at("writer_id"));
        opts->iface               = dup(main_cfg_parameters.at("iface"));
        opts->ipv4_socket_cisco   = dup(main_cfg_parameters.at("socket_cisco"));
        opts->ipv4_socket_juniper = dup(main_cfg_parameters.at("socket_juniper"));
        opts->ipv4_socket_nokia   = dup(main_cfg_parameters.at("socket_nokia"));
        opts->ipv4_socket_huawei  = dup(main_cfg_parameters.at("socket_huawei"));
        opts->cisco_workers       = dup(main_cfg_parameters.at("cisco_workers"));
        opts->juniper_workers     = dup(main_cfg_parameters.at("juniper_workers"));
        opts->nokia_workers       = dup(main_cfg_parameters.at("nokia_workers"));
        opts->huawei_workers      = dup(main_cfg_parameters.at("huawei_workers"));
        opts->replies_cisco       = dup(main_cfg_parameters.at("replies_cisco"));
        opts->replies_juniper     = dup(main_cfg_parameters.at("replies_juniper"));
        opts->replies_nokia       = dup(main_cfg_parameters.at("replies_nokia"));
        opts->replies_huawei      = dup(main_cfg_parameters.at("replies_huawei"));
        opts->syslog              = dup(logs_cfg_parameters.at("syslog"));
        opts->syslog_facility     = dup(logs_cfg_parameters.at("syslog_facility"));
        opts->syslog_ident        = dup(logs_cfg_parameters.at("syslog_ident"));
        opts->console_log         = dup(logs_cfg_parameters.at("console_log"));
        opts->spdlog_level        = dup(logs_cfg_parameters.at("spdlog_level"));
        opts->enable_cisco_gpbkv2json =
            dup(data_manipulation_cfg_parameters.at("enable_cisco_gpbkv2json"));
        opts->enable_cisco_message_to_json_string = dup(
            data_manipulation_cfg_parameters.at("enable_cisco_message_to_json_string"));
        opts->enable_label_encode_as_map =
            dup(data_manipulation_cfg_parameters.at("enable_label_encode_as_map"));
        opts->enable_label_encode_as_map_ptm = dup(
            data_manipulation_cfg_parameters.at("enable_label_encode_as_map_ptm"));
        return opts;
    }

    void InitGrpcPayload(grpc_payload **pload_, const char *event_type,
        const char *serialization, const char *writer_id,
        const char *telemetry_node, const char *telemetry_port,
        const char *telemetry_data)
    {
        grpc_payload *pload = (grpc_payload *) malloc(sizeof(grpc_payload));

        if (pload == NULL) {
            spdlog::get("multi-logger")->
                error("Unable to allocate memory for grpc_payload");
            std::exit(EXIT_FAILURE);
        }

        pload->event_type = strndup(event_type, strlen(event_type));
        pload->serialization = strndup(serialization, strlen(serialization));
        pload->writer_id = strndup(writer_id, strlen(writer_id));
        pload->telemetry_node = strndup(telemetry_node, strlen(telemetry_node));
        pload->telemetry_port = strndup(telemetry_port, strlen(telemetry_port));
        pload->telemetry_data = strndup(telemetry_data, strlen(telemetry_data));

        if (pload->event_type == NULL || pload->serialization == NULL ||
            pload->writer_id == NULL || pload->telemetry_node == NULL ||
            pload->telemetry_port == NULL || pload->telemetry_data == NULL) {
            spdlog::get("multi-logger")->
                error("Unable to allocate memory for grpc_payload members");
            free_grpc_payload(pload);
            std::exit(EXIT_FAILURE);
        }

        *pload_ = pload;
    }

    void FreeOptions(Options *opts)
    {
        free(opts->writer_id);
        free(opts->iface);
        free(opts->ipv4_socket_cisco);
        free(opts->ipv4_socket_juniper);
        free(opts->ipv4_socket_nokia);
        free(opts->ipv4_socket_huawei);
        free(opts->cisco_workers);
        free(opts->juniper_workers);
        free(opts->nokia_workers);
        free(opts->huawei_workers);
        free(opts->replies_cisco);
        free(opts->replies_juniper);
        free(opts->replies_nokia);
        free(opts->replies_huawei);
        free(opts->syslog);
        free(opts->syslog_facility);
        free(opts->syslog_ident);
        free(opts->console_log);
        free(opts->spdlog_level);
        free(opts->enable_cisco_gpbkv2json);
        free(opts->enable_cisco_message_to_json_string);
        free(opts->enable_label_encode_as_map);
        free(opts->enable_label_encode_as_map_ptm);
        free(opts);
    }

    void free_grpc_payload(grpc_payload *pload)
    {
        if (pload == NULL) {
            return;
        }

        free(pload->event_type);
        pload->event_type = NULL;

        free(pload->serialization);
        pload->serialization = NULL;

        free(pload->writer_id);
        pload->writer_id = NULL;

        free(pload->telemetry_node);
        pload->telemetry_node = NULL;

        free(pload->telemetry_port);
        pload->telemetry_port = NULL;

        free(pload->telemetry_data);
        pload->telemetry_data = NULL;

        free(pload);
    }

    void start_grpc_dialout_collector(const char *cfg_path,
        const char *zmq_uri)
    {
        if (!LoadOptions(cfg_path, zmq_uri)) {
            // logs_handler logged the specifics; bail without spawning
            // workers so the host process keeps running.
            spdlog::get("multi-logger")->
                error("[start_grpc_dialout_collector] cfg load failed; "
                "no workers started");
            return;
        }
        spdlog::get("multi-logger")->debug(
            "sockets found in config:\n socket_cisco: {},\n socket_juniper: {},\n socket_nokia: {},\n socket_huawei: {}\n",
            main_cfg_parameters.at("socket_cisco"),
            main_cfg_parameters.at("socket_juniper"),
            main_cfg_parameters.at("socket_nokia"),
            main_cfg_parameters.at("socket_huawei")
        );
        if (main_cfg_parameters.at("socket_cisco").empty() == true &&
            main_cfg_parameters.at("socket_juniper").empty() == true &&
            main_cfg_parameters.at("socket_nokia").empty() == true &&
            main_cfg_parameters.at("socket_huawei").empty() == true) {
                spdlog::get("multi-logger")->
                    error("[socket_*] configuration issue: "
                    "unable to find at least one valid IPv4 socket where to bind "
                    "the daemon");
                return;
        }

        pthread_t cisco_workers[MAX_WORKERS] = {0};
        pthread_t juniper_workers[MAX_WORKERS] = {0};
        pthread_t nokia_workers[MAX_WORKERS] = {0};
        pthread_t huawei_workers[MAX_WORKERS] = {0};

        LoadThreads(cisco_workers, "socket_cisco", "replies_cisco",
            "cisco_workers");
        LoadThreads(juniper_workers, "socket_juniper", "replies_juniper",
            "juniper_workers");
        LoadThreads(nokia_workers, "socket_nokia", "replies_nokia",
            "nokia_workers");
        LoadThreads(huawei_workers, "socket_huawei", "replies_huawei",
            "huawei_workers");

        // Workers are joinable and tracked in g_active_workers (registered
        // by LoadThreads). Caller can stop them with stop_grpc_dialout_collector().
    }

    int stop_grpc_dialout_collector(void)
    {
        // Drain all live Srv instances (idempotent) and join every worker.
        initiate_shutdown();

        std::vector<pthread_t> snapshot;
        {
            std::lock_guard<std::mutex> lk(g_workers_mutex);
            snapshot.swap(g_active_workers);
        }
        for (pthread_t t : snapshot) {
            pthread_join(t, nullptr);
        }
        return 1;
    }

    int LoadOptions(const char *cfg_path, const char *zmq_uri)
    {
        LogsHandler logs_handler;
        spdlog::get("multi-logger-boot")->debug("main: main()");

        CfgHandler cfg_handler;
        cfg_handler.set_cfg_path(cfg_path);

        LogsCfgHandler logs_cfg_handler;
        if (!logs_cfg_handler.lookup_logs_parameters(
                cfg_handler.get_cfg_path(),
                cfg_handler.get_logs_parameters())) {
            return 0;
        }
        logs_cfg_parameters = cfg_handler.get_logs_parameters();
        logs_handler.set_spdlog_sinks();

        MainCfgHandler main_cfg_handler;
        if (main_cfg_handler.lookup_main_parameters(
            cfg_handler.get_cfg_path(),
            cfg_handler.get_main_parameters()) == false) {
            return 0;
        }
        main_cfg_parameters = cfg_handler.get_main_parameters();

        DataManipulationCfgHandler data_manipulation_cfg_handler;
        if (data_manipulation_cfg_handler.lookup_data_manipulation_parameters(
            cfg_handler.get_cfg_path(),
            cfg_handler.get_data_manipulation_parameters()) == false) {
            return 0;
        }
        data_manipulation_cfg_parameters =
            cfg_handler.get_data_manipulation_parameters();

        KafkaCfgHandler kafka_cfg_handler;
        if (kafka_cfg_handler.lookup_kafka_parameters(
            cfg_handler.get_cfg_path(),
            cfg_handler.get_kafka_parameters()) == false) {
            return 0;
        }
        kafka_delivery_cfg_parameters = cfg_handler.get_kafka_parameters();

        ZmqCfgHandler zmq_cfg_handler;
        if (zmq_cfg_handler.lookup_zmq_parameters(
            zmq_uri,
            cfg_handler.get_zmq_parameters()) == false) {
            return 0;
        }
        zmq_delivery_cfg_parameters = cfg_handler.get_zmq_parameters();

        return 1;
    }

    void *VendorThread(void *socket_str_)
    {
        const char *socket_str = (char *) socket_str_;
        const std::string srv_socket = main_cfg_parameters.at(socket_str);

        if (strstr(socket_str, "cisco") != NULL) {
            Srv cisco_mdt_dialout_collector;
            cisco_mdt_dialout_collector.CiscoBind(srv_socket);
        } else if (strstr(socket_str, "juniper") != NULL) {
            Srv juniper_mdt_dialout_collector;
            juniper_mdt_dialout_collector.JuniperBind(srv_socket);
        } else if (strstr(socket_str, "nokia") != NULL) {
            Srv nokia_mdt_dialout_collector;
            nokia_mdt_dialout_collector.NokiaBind(srv_socket);
        } else if (strstr(socket_str, "huawei") != NULL) {
            Srv huawei_mdt_dialout_collector;
            huawei_mdt_dialout_collector.HuaweiBind(srv_socket);
        }

        return (NULL);
    }

    void LoadThreads(pthread_t *workers_vec,
        const char *socket_str,
        const char *replies_str,
        const char *workers_str)
    {
        if (main_cfg_parameters.at(socket_str).empty() == false) {
            int replies = 0;
            int workers = 0;
            try {
                replies = std::stoi(main_cfg_parameters.at(replies_str));
                workers = std::stoi(main_cfg_parameters.at(workers_str));
            } catch (const std::exception &e) {
                spdlog::get("multi-logger")->
                    error("[{}/{}] configuration issue: non-numeric value: "
                    "{} — skipping vendor", replies_str, workers_str, e.what());
                return;
            }
            if (replies < 0 || replies > 1000) {
                spdlog::get("multi-logger")->
                    error("[{}] configuration issue: the "
                    "allowed amount of replies per session is defined between 10 "
                    "and 1000. (default = 0 => unlimited) — skipping vendor",
                    replies_str);
                return;
            }
            if (workers < 1 || workers > 5) {
                spdlog::get("multi-logger")->
                    error("[{}] configuration issue: the "
                    "allowed amount of workers is defined between 1 "
                    "and 5. (default = 1) — skipping vendor", workers_str);
                return;
            }
            for (int w = 0; w < workers; w++) {
                int res = pthread_create(&workers_vec[w], NULL, VendorThread,
                    (void *)socket_str);
                if (res != 0) {
                    spdlog::get("multi-logger")->
                        error("Failed to create thread: {}", strerror(res));
                    continue;
                }
                std::lock_guard<std::mutex> lk(g_workers_mutex);
                g_active_workers.push_back(workers_vec[w]);
            }
            spdlog::get("multi-logger")->
                info("mdt-dialout-collector listening on {} ",
                main_cfg_parameters.at(socket_str));
        }
    }
}

