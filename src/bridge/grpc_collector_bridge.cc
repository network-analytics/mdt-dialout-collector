// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include "grpc_collector_bridge.h"
#include "../core/mdt_dialout_core.h"
#include "../cfgWrapper/cfg_wrapper.h"


extern "C" {
    Options *InitOptions()
    {
        CfgWrapper cfg_wrapper;
        cfg_wrapper.BuildCfgWrapper(
            main_cfg_parameters.at("writer_id"),
            main_cfg_parameters.at("iface"),
            main_cfg_parameters.at("ipv4_socket_cisco"),
            main_cfg_parameters.at("ipv4_socket_juniper"),
            main_cfg_parameters.at("ipv4_socket_huawei"),
            /*main_cfg_parameters.at("core_pid_folder"),*/
            main_cfg_parameters.at("cisco_workers"),
            main_cfg_parameters.at("juniper_workers"),
            main_cfg_parameters.at("huawei_workers"),
            main_cfg_parameters.at("replies_cisco"),
            main_cfg_parameters.at("replies_juniper"),
            main_cfg_parameters.at("replies_huawei"),
            logs_cfg_parameters.at("syslog"),
            logs_cfg_parameters.at("syslog_facility"),
            logs_cfg_parameters.at("syslog_ident"),
            logs_cfg_parameters.at("console_log"),
            logs_cfg_parameters.at("spdlog_level"),
            data_manipulation_cfg_parameters.at("enable_cisco_gpbkv2json"),
            data_manipulation_cfg_parameters.at(
                "enable_cisco_message_to_json_string"),
            data_manipulation_cfg_parameters.at("enable_label_encode_as_map"),
            data_manipulation_cfg_parameters.at("enable_label_encode_as_map_ptm"));

        Options *opts = (Options *) malloc(sizeof(Options));

        const char *writer_id = cfg_wrapper.get_writer_id().c_str();
        opts->writer_id = strndup(writer_id, strlen(writer_id));

        const char *iface = cfg_wrapper.get_iface().c_str();
        opts->iface = strndup(iface, strlen(iface));

        const char *ipv4_socket_cisco =
            cfg_wrapper.get_ipv4_socket_cisco().c_str();
        opts->ipv4_socket_cisco =
            strndup(ipv4_socket_cisco, strlen(ipv4_socket_cisco));

        const char *ipv4_socket_juniper =
            cfg_wrapper.get_ipv4_socket_juniper().c_str();
        opts->ipv4_socket_juniper =
            strndup(ipv4_socket_juniper, strlen(ipv4_socket_juniper));

        const char *ipv4_socket_huawei =
            cfg_wrapper.get_ipv4_socket_huawei().c_str();
        opts->ipv4_socket_huawei =
            strndup(ipv4_socket_huawei, strlen(ipv4_socket_huawei));

        /*const char *core_pid_folder =
         * cfg_wrapper.get_core_pid_folder().c_str();
        opts->core_pid_folder =
            strndup(core_pid_folder, strlen(core_pid_folder));*/

        const char *cisco_workers = cfg_wrapper.get_cisco_workers().c_str();
        opts->cisco_workers = strndup(cisco_workers, strlen(cisco_workers));

        const char *juniper_workers = cfg_wrapper.get_juniper_workers().c_str();
        opts->juniper_workers = strndup(juniper_workers, strlen(juniper_workers));

        const char *huawei_workers = cfg_wrapper.get_huawei_workers().c_str();
        opts->huawei_workers = strndup(huawei_workers, strlen(huawei_workers));

        const char *replies_cisco = cfg_wrapper.get_replies_cisco().c_str();
        opts->replies_cisco = strndup(replies_cisco, strlen(replies_cisco));

        const char *replies_juniper = cfg_wrapper.get_replies_juniper().c_str();
        opts->replies_juniper = strndup(replies_juniper, strlen(replies_juniper));

        const char *replies_huawei = cfg_wrapper.get_replies_huawei().c_str();
        opts->replies_huawei = strndup(replies_huawei, strlen(replies_huawei));

        const char *syslog = cfg_wrapper.get_syslog().c_str();
        opts->syslog = strndup(syslog, strlen(syslog));

        const char *syslog_facility = cfg_wrapper.get_syslog_facility().c_str();
        opts->syslog_facility = strndup(syslog_facility, strlen(syslog_facility));

        const char *syslog_ident = cfg_wrapper.get_syslog_ident().c_str();
        opts->syslog_ident = strndup(syslog_ident, strlen(syslog_ident));

        const char *console_log = cfg_wrapper.get_console_log().c_str();
        opts->console_log = strndup(console_log, strlen(console_log));

        const char *spdlog_level = cfg_wrapper.get_spdlog_level().c_str();
        opts->spdlog_level = strndup(spdlog_level, strlen(spdlog_level));

        const char *enable_cisco_gpbkv2json =
            cfg_wrapper.get_enable_cisco_gpbkv2json().c_str();
        opts->enable_cisco_gpbkv2json =
            strndup(enable_cisco_gpbkv2json, strlen(enable_cisco_gpbkv2json));

        const char *enable_cisco_message_to_json_string =
            cfg_wrapper.get_enable_cisco_message_to_json_string().c_str();
        opts->enable_cisco_message_to_json_string =
            strndup(enable_cisco_message_to_json_string,
                strlen(enable_cisco_message_to_json_string));

        const char *enable_label_encode_as_map =
            cfg_wrapper.get_enable_label_encode_as_map().c_str();
        opts->enable_label_encode_as_map =
            strndup(enable_label_encode_as_map,
                strlen(enable_label_encode_as_map));

        const char *enable_label_encode_as_map_ptm =
            cfg_wrapper.get_enable_label_encode_as_map_ptm().c_str();
        opts->enable_label_encode_as_map_ptm =
            strndup(enable_label_encode_as_map_ptm,
                strlen(enable_label_encode_as_map_ptm));

        return opts;
    }

    void InitGrpcPayload(grpc_payload **pload_, const char *event_type,
        const char *serialization, const char *writer_id,
        const char *telemetry_node, const char *telemetry_port,
        const char *telemetry_data)
    {
        grpc_payload *pload = (grpc_payload *) malloc(sizeof(grpc_payload));

        // Check if memory was successfully allocated
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

        // Check if memory was successfully allocated for each member
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
        free(opts->ipv4_socket_huawei);
        //free(opts->core_pid_folder);
        free(opts->cisco_workers);
        free(opts->juniper_workers);
        free(opts->huawei_workers);
        free(opts->replies_cisco);
        free(opts->replies_juniper);
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
        LoadOptions(cfg_path, zmq_uri);

        if (main_cfg_parameters.at("ipv4_socket_cisco").empty() == true &&
            main_cfg_parameters.at("ipv4_socket_juniper").empty() == true &&
            main_cfg_parameters.at("ipv4_socket_huawei").empty() == true) {
                spdlog::get("multi-logger")->
                    error("[ipv4_socket_*] configuration issue: "
                    "unable to find at least one valid IPv4 socket where to bind "
                    "the daemon");
                std::exit(EXIT_FAILURE);
        }

        // Use a vector to store the worker threads
        std::vector<pthread_t> workers(MAX_WORKERS);

        // Cisco
        LoadThreads(workers.data(), "ipv4_socket_cisco", "replies_cisco",
            "cisco_workers");

        // Juniper
        LoadThreads(workers.data(), "ipv4_socket_juniper", "replies_juniper",
            "juniper_workers");

        // Huawei
        LoadThreads(workers.data(), "ipv4_socket_huawei", "replies_huawei",
            "huawei_workers");

        size_t w;
        for (w = 0; w < MAX_WORKERS; w++) {
            pthread_detach(workers[w]);
        }
    }

    void LoadOptions(const char *cfg_path, const char *zmq_uri)
    {
        // static log-sinks are configured within the constructor
        LogsHandler logs_handler;
        spdlog::get("multi-logger-boot")->debug("main: main()");

        CfgHandler cfg_handler;
        cfg_handler.set_cfg_path(cfg_path);

        LogsCfgHandler logs_cfg_handler;
        if (logs_cfg_handler.lookup_logs_parameters(
            cfg_handler.get_cfg_path(),
            cfg_handler.get_logs_parameters()) == false) {
            // can't read the logs cfg params the destructor logging won't
            // be possible (segmentation fault)
            std::exit(EXIT_FAILURE);
        } else {
            logs_cfg_parameters = cfg_handler.get_logs_parameters();
            // set the log-sinks after reading from the configuration file
            logs_handler.set_spdlog_sinks();
        }

        MainCfgHandler main_cfg_handler;
        if (main_cfg_handler.lookup_main_parameters(
            cfg_handler.get_cfg_path(),
            cfg_handler.get_main_parameters()) == false) {
            std::exit(EXIT_FAILURE);
        } else {
            main_cfg_parameters = cfg_handler.get_main_parameters();
        }

        DataManipulationCfgHandler data_manipulation_cfg_handler;
        if (data_manipulation_cfg_handler.lookup_data_manipulation_parameters(
            cfg_handler.get_cfg_path(),
            cfg_handler.get_data_manipulation_parameters()) ==false) {
            std::exit(EXIT_FAILURE);
        } else {
            data_manipulation_cfg_parameters =
                cfg_handler.get_data_manipulation_parameters();
        }

        KafkaCfgHandler kafka_cfg_handler;
        if (kafka_cfg_handler.lookup_kafka_parameters(
            cfg_handler.get_cfg_path(),
            cfg_handler.get_kafka_parameters()) == false) {
            std::exit(EXIT_FAILURE);
        } else {
            kafka_delivery_cfg_parameters = cfg_handler.get_kafka_parameters();
        }

        ZmqCfgHandler zmq_cfg_handler;
        if (zmq_cfg_handler.lookup_zmq_parameters(
            zmq_uri,
            cfg_handler.get_zmq_parameters()) == false) {
            std::exit(EXIT_FAILURE);
        } else {
            zmq_delivery_cfg_parameters = cfg_handler.get_zmq_parameters();
        }
    }

    void *VendorThread(void *ipv4_socket_str_)
    {
        const char *ipv4_socket_str = (char *) ipv4_socket_str_;

        if (strstr(ipv4_socket_str, "cisco") != NULL) {
            std::string ipv4_socket_cisco =
                main_cfg_parameters.at(ipv4_socket_str);

            std::string cisco_srv_socket {ipv4_socket_cisco};
            Srv cisco_mdt_dialout_collector;
            cisco_mdt_dialout_collector.CiscoBind(cisco_srv_socket);
        } else if (strstr(ipv4_socket_str, "juniper") != NULL) {
            std::string ipv4_socket_juniper =
                main_cfg_parameters.at(ipv4_socket_str);

            std::string juniper_srv_socket {ipv4_socket_juniper};
            Srv juniper_mdt_dialout_collector;
            juniper_mdt_dialout_collector.JuniperBind(juniper_srv_socket);
        } else if (strstr(ipv4_socket_str, "huawei") != NULL) {
            std::string ipv4_socket_huawei =
                main_cfg_parameters.at(ipv4_socket_str);

            std::string huawei_srv_socket {ipv4_socket_huawei};
            Srv huawei_mdt_dialout_collector;
            huawei_mdt_dialout_collector.HuaweiBind(huawei_srv_socket);
        }

        return (NULL);
    }

    void LoadThreads(pthread_t *workers_vec,
        const char *ipv4_socket_str,
        const char *replies_str,
        const char *workers_str)
    {
        if (main_cfg_parameters.at(ipv4_socket_str).empty() == false) {
            int replies =
                std::stoi(main_cfg_parameters.at(replies_str));
            if (replies < 0 || replies > 1000) {
                spdlog::get("multi-logger")->
                    error("[{}] configuaration issue: the "
                    "allowed amount of replies per session is defined between 10 "
                    "and 1000. (default = 0 => unlimited)", replies_str);
                std::exit(EXIT_FAILURE);
            }
            size_t workers = std::stoi(main_cfg_parameters.at(workers_str));
            if (workers < 1 || workers > 5) {
                spdlog::get("multi-logger")->
                    error("[{}] configuaration issue: the "
                    "allowed amount of workers is defined between 1 "
                    "and 5. (default = 1)", workers_str);
                std::exit(EXIT_FAILURE);
            }
            size_t w;
            for (w = 0; w < workers; w++) {
                pthread_create(&workers_vec[w], NULL, VendorThread,
                    (void *) ipv4_socket_str);
            }
            spdlog::get("multi-logger")->
                info("mdt-dialout-collector listening on {} ",
                main_cfg_parameters.at(ipv4_socket_str));
        }
    }
}

