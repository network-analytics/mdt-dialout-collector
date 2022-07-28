// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "cfg_handler.h"


// Centralizing config parameters
std::unique_ptr<MainCfgHandler>
    main_cfg_handler(new MainCfgHandler());
std::unique_ptr<DataManipulationCfgHandler>
    data_manipulation_cfg_handler(new DataManipulationCfgHandler());
std::unique_ptr<KafkaCfgHandler>
    data_delivery_cfg_handler(new KafkaCfgHandler());

std::map<std::string, std::string> main_cfg_parameters =
    main_cfg_handler->get_parameters();
std::map<std::string, std::string> data_manipulation_cfg_parameters =
    data_manipulation_cfg_handler->get_parameters();
std::map<std::string, std::string> data_delivery_cfg_parameters =
    data_delivery_cfg_handler->get_parameters();

MainCfgHandler::MainCfgHandler()
{
    if (!lookup_main_parameters(this->mdt_dialout_collector_conf,
        this->parameters)) {
        this->core_pid_folder = parameters.at("core_pid_folder");
        this->iface = parameters.at("iface");
        this->ipv4_socket_cisco = parameters.at("ipv4_socket_cisco");
        this->ipv4_socket_juniper = parameters.at("ipv4_socket_juniper");
        this->ipv4_socket_huawei = parameters.at("ipv4_socket_huawei");
        this->cisco_workers = parameters.at("cisco_workers");
        this->juniper_workers = parameters.at("juniper_workers");
        this->huawei_workers = parameters.at("huawei_workers");
    } else {
        throw std::exception();
    }
}

int MainCfgHandler::lookup_main_parameters(const std::string &cfg_path,
    std::map<std::string, std::string> &params)
{
    params.clear();
    std::unique_ptr<libconfig::Config> main_params(new libconfig::Config());

    try {
        main_params->readFile(cfg_path.c_str());
    } catch (const libconfig::FileIOException &fioex) {
        std::cout << "libconfig::FileIOException\n";
        return EXIT_FAILURE;
    } catch(const libconfig::ParseException &pex) {
        std::cout << "libconfig::ParseException\n";
        return EXIT_FAILURE;
    }

    // Main parameters evaluation
    bool core_pid_folder =
        main_params->exists("core_pid_folder");
    if (core_pid_folder == true) {
        libconfig::Setting &core_pid_folder =
            main_params->lookup("core_pid_folder");
        std::string core_pid_folder_s = core_pid_folder;
        if (core_pid_folder_s.empty() == false &&
            std::filesystem::exists(core_pid_folder_s) == true) {
            params.insert({"core_pid_folder", core_pid_folder_s});
        } else {
            std::cout << "core_pid_folder: invalid folder\n";
            std::exit(EXIT_FAILURE);
        }
    } else {
        const std::string default_core_pid_folder = "/var/run/";
        if (std::filesystem::exists(default_core_pid_folder) == true) {
            params.insert({"core_pid_folder", default_core_pid_folder});
        } else {
            std::cout << "core_pid_folder: invalid folder\n";
            std::exit(EXIT_FAILURE);
        }
    }

    bool iface = main_params->exists("iface");
    if (iface == true) {
        libconfig::Setting &iface = main_params->lookup("iface");
        std::string iface_s = iface;
        if (iface_s.empty() == false) {
            params.insert({"iface", iface_s});
        } else {
            std::cout << "empty iface not allowed\n";
            return EXIT_FAILURE;
        }
    } else {
        std::cout << "mdt-dialout-collector: iface mandatory\n";
        throw libconfig::SettingNotFoundException("iface");
    }

    bool ipv4_socket_cisco = main_params->exists("ipv4_socket_cisco");
    if (ipv4_socket_cisco == true) {
        libconfig::Setting &ipv4_socket_cisco =
            main_params->lookup("ipv4_socket_cisco");
        std::string ipv4_socket_cisco_s = ipv4_socket_cisco;
        if (ipv4_socket_cisco_s.empty() == false) {
            params.insert({"ipv4_socket_cisco", ipv4_socket_cisco_s});
        } else {
            std::cout << "ipv4_socket_cisco: valid value not empty\n";
            return EXIT_FAILURE;
        }
    } else {
        params.insert({"ipv4_socket_cisco", ""});
    }

    bool ipv4_socket_juniper = main_params->exists("ipv4_socket_juniper");
    if (ipv4_socket_juniper == true) {
        libconfig::Setting &ipv4_socket_juniper =
            main_params->lookup("ipv4_socket_juniper");
        std::string ipv4_socket_juniper_s = ipv4_socket_juniper;
        if (ipv4_socket_juniper_s.empty() == false) {
            params.insert({"ipv4_socket_juniper", ipv4_socket_juniper_s});
        } else {
            std::cout << "ipv4_socket_juniper: valid value not empty\n";
            return EXIT_FAILURE;
        }
    } else {
        params.insert({"ipv4_socket_juniper", ""});
    }

    bool ipv4_socket_huawei = main_params->exists("ipv4_socket_huawei");
    if (ipv4_socket_huawei == true) {
        libconfig::Setting &ipv4_socket_huawei =
            main_params->lookup("ipv4_socket_huawei");
        std::string ipv4_socket_huawei_s = ipv4_socket_huawei;
        if (ipv4_socket_huawei_s.empty() == false) {
            params.insert({"ipv4_socket_huawei", ipv4_socket_huawei_s});
        } else {
            std::cout << "ipv4_socket_huawei: valid value not empty\n";
            return EXIT_FAILURE;
        }
    } else {
        params.insert({"ipv4_socket_huawei", ""});
    }

    bool cisco_workers = main_params->exists("cisco_workers");
    if (cisco_workers == true) {
        libconfig::Setting &cisco_workers =
            main_params->lookup("cisco_workers");
        std::string cisco_workers_s = cisco_workers;
        if (cisco_workers_s.empty() == false) {
            params.insert({"cisco_workers", cisco_workers_s});
        } else {
            std::cout << "cisco_workers: valid value not empty\n";
            return EXIT_FAILURE;
        }
    } else {
        params.insert({"cisco_workers", "1"});
    }

    bool juniper_workers = main_params->exists("juniper_workers");
    if (juniper_workers == true) {
        libconfig::Setting &juniper_workers =
            main_params->lookup("juniper_workers");
        std::string juniper_workers_s = juniper_workers;
        if (juniper_workers_s.empty() == false) {
            params.insert({"juniper_workers", juniper_workers_s});
        } else {
            std::cout << "juniper_workers: valid value not empty\n";
            return EXIT_FAILURE;
        }
    } else {
        params.insert({"juniper_workers", "1"});
    }

    bool huawei_workers = main_params->exists("huawei_workers");
    if (huawei_workers == true) {
        libconfig::Setting &huawei_workers =
            main_params->lookup("huawei_workers");
        std::string huawei_workers_s = huawei_workers;
        if (huawei_workers_s.empty() == false) {
            params.insert({"huawei_workers", huawei_workers_s});
        } else {
            std::cout << "huawei_workers: valid value not empty\n";
            return EXIT_FAILURE;
        }
    } else {
        params.insert({"huawei_workers", "1"});
    }

    return EXIT_SUCCESS;
}

DataManipulationCfgHandler::DataManipulationCfgHandler()
{
    if (!lookup_data_manipulation_parameters(this->mdt_dialout_collector_conf,
        this->parameters)) {
        this->enable_cisco_message_to_json_string =
            parameters.at("enable_cisco_message_to_json_string");
        this->enable_cisco_gpbkv2json =
            parameters.at("enable_cisco_gpbkv2json");
        this->enable_label_encode_as_map =
            parameters.at("enable_label_encode_as_map");
        this->label_map_csv_path =
            parameters.at("label_map_csv_path");
    } else {
        throw std::exception();
    }
}

int DataManipulationCfgHandler::lookup_data_manipulation_parameters(
    const std::string &cfg_path,
    std::map<std::string, std::string> &params)
{
    params.clear();
    std::unique_ptr<libconfig::Config>
        data_manipulation_params(new libconfig::Config());

    try {
        data_manipulation_params->readFile(cfg_path.c_str());
    } catch (const libconfig::FileIOException &fioex) {
        std::cout << "libconfig::FileIOException\n";
        return EXIT_FAILURE;
    } catch(const libconfig::ParseException &pex) {
        std::cout << "libconfig::ParseException\n";
        return EXIT_FAILURE;
    }

    // Data manipulation parameters evaluation
    bool enable_cisco_message_to_json_string =
        data_manipulation_params->exists("enable_cisco_message_to_json_string");
    if (enable_cisco_message_to_json_string == true) {
        libconfig::Setting &enable_cisco_message_to_json_string =
            data_manipulation_params->lookup(
            "enable_cisco_message_to_json_string");
        std::string enable_cisco_message_to_json_string_s =
            enable_cisco_message_to_json_string;
        if (enable_cisco_message_to_json_string_s.empty() == false) {
            params.insert({"enable_cisco_message_to_json_string",
            enable_cisco_message_to_json_string_s});
        } else {
            std::cout <<
                "enable_cisco_message_to_json_string: valid value not empty\n";
            return EXIT_FAILURE;
        }
    } else {
        params.insert({"enable_message_to_json_string", "false"});
    }

    bool enable_cisco_gpbkv2json =
        data_manipulation_params->exists("enable_cisco_gpbkv2json");
    if (enable_cisco_gpbkv2json == true) {
        libconfig::Setting &enable_cisco_gpbkv2json =
            data_manipulation_params->lookup("enable_cisco_gpbkv2json");
        std::string enable_cisco_gpbkv2json_s =
            enable_cisco_gpbkv2json;
        if (enable_cisco_gpbkv2json_s.empty() == false) {
            params.insert({"enable_cisco_gpbkv2json",
            enable_cisco_gpbkv2json_s});
        } else {
            std::cout << "enable_cisco_gpbkv2json: valid value not empty\n";
            return EXIT_FAILURE;
        }
    } else {
        params.insert({"enable_cisco_gpbkv2json", "true"});
    }

    bool enable_label_encode_as_map =
        data_manipulation_params->exists("enable_label_encode_as_map");
    if (enable_label_encode_as_map == true) {
        libconfig::Setting &enable_label_encode_as_map =
            data_manipulation_params->lookup("enable_label_encode_as_map");
        std::string enable_label_encode_as_map_s =
            enable_label_encode_as_map;
        if (enable_label_encode_as_map_s.empty() == false) {
            params.insert({"enable_label_encode_as_map",
            enable_label_encode_as_map_s});
        } else {
            std::cout << "enable_label_encode_as_map: valid value not empty\n";
            return EXIT_FAILURE;
        }
    } else {
        params.insert({"enable_label_encode_as_map", "false"});
    }

    bool label_map_csv_path =
        data_manipulation_params->exists("label_map_csv_path");
    if (label_map_csv_path == true) {
        libconfig::Setting &label_map_csv_path =
            data_manipulation_params->lookup("label_map_csv_path");
        std::string label_map_csv_path_s =
            label_map_csv_path;
        if (label_map_csv_path_s.empty() == false &&
            std::filesystem::exists(label_map_csv_path_s) == true) {
            params.insert({"label_map_csv_path",
            label_map_csv_path_s});
        } else {
            std::cout << "label_map_csv_path: invalid path\n";
            std::exit(EXIT_FAILURE);
        }
    } else {
        const std::string default_label_map_csv_path =
            "/opt/mdt_dialout_collector/csv/label_map.csv";
        if (std::filesystem::exists(default_label_map_csv_path) == true) {
            params.insert({"label_map_csv_path", default_label_map_csv_path});
        } else {
            std::cout << "label_map_csv_path: invalid path\n";
            std::exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}

KafkaCfgHandler::KafkaCfgHandler()
{
    if (!lookup_kafka_parameters(this->mdt_dialout_collector_conf,
        this->parameters)) {
        this->topic = parameters.at("topic");
        this->bootstrap_servers = parameters.at("bootstrap_servers");
        this->enable_idempotence = parameters.at("enable_idempotence");
        this->client_id = parameters.at("client_id");
        this->security_protocol = parameters.at("security_protocol");
        this->ssl_key_location = parameters.at("ssl_key_location");
        this->ssl_certificate_location =
            parameters.at("ssl_certificate_location");
        this->ssl_ca_location = parameters.at("ssl_ca_location");
        this->log_level = parameters.at("log_level");
    } else {
        throw std::exception();
    }
}

int KafkaCfgHandler::lookup_kafka_parameters(const std::string &cfg_path,
    std::map<std::string, std::string> &params)
{
    params.clear();
    std::unique_ptr<libconfig::Config> kafka_params(new libconfig::Config());

    try {
        kafka_params->readFile(cfg_path.c_str());
    } catch (const libconfig::FileIOException &fioex) {
        std::cout << "libconfig::FileIOException\n";
        return EXIT_FAILURE;
    } catch(const libconfig::ParseException &pex) {
        std::cout << "libconfig::ParseException\n";
        return EXIT_FAILURE;
    }

    // Kafka arameters evaluation
    bool topic = kafka_params->exists("topic");
    if (topic == true) {
        libconfig::Setting &topic = kafka_params->lookup("topic");
        std::string topic_s = topic.c_str();
        if (topic_s.empty() == false) {
            params.insert({"topic", topic_s});
        } else {
            std::cout << "empty topic not allowed\n";
            return EXIT_FAILURE;
        }
    } else {
        std::cout << "kafka-producer: topic mandatory\n";
        throw libconfig::SettingNotFoundException("topic");
    }

    bool bootstrap_servers = kafka_params->exists("bootstrap_servers");
    if (bootstrap_servers == true) {
        libconfig::Setting &bootstrap_servers =
            kafka_params->lookup("bootstrap_servers");
        std::string bootstrap_servers_s = bootstrap_servers.c_str();
        if (bootstrap_servers_s.empty() == false) {
            params.insert({"bootstrap_servers", bootstrap_servers_s});
        } else {
            std::cout << "empty bootstrap_servers not allowed\n";
            return EXIT_FAILURE;
        }
    } else {
        std::cout << "kafka-producer: bootstrap_servers mandatory\n";
        throw libconfig::SettingNotFoundException("bootstrap_servers");
    }

    bool enable_idempotence = kafka_params->exists("enable_idempotence");
    if (enable_idempotence == true) {
        libconfig::Setting &enable_idempotence =
            kafka_params->lookup("enable_idempotence");
        std::string enable_idempotence_s = enable_idempotence.c_str();
        if (enable_idempotence_s.empty() == false &&
            (enable_idempotence_s.compare("true") == 0 or
            enable_idempotence_s.compare("false") == 0)) {
            params.insert({"enable_idempotence", enable_idempotence_s});
        } else {
            std::cout << "enable_idempotence: valid value <true | false>\n";
            return EXIT_FAILURE;
        }
    } else {
        params.insert({"enable_idempotence", "true"});
    }

    bool client_id = kafka_params->exists("client_id");
    if (client_id == true) {
        libconfig::Setting &client_id = kafka_params->lookup("client_id");
        std::string client_id_s = client_id.c_str();
        if (client_id_s.empty() == false) {
            params.insert({"client_id", client_id_s});
        } else {
            std::cout << "client_id: valid value not empty\n";
            return EXIT_FAILURE;
        }
    } else {
        params.insert({"client_id", "mdt-dialout-collector"});
    }

    bool log_level = kafka_params->exists("log_level");
    if (log_level == true) {
        libconfig::Setting &log_level = kafka_params->lookup("log_level");
        std::string log_level_s = log_level.c_str();
        if (log_level_s.empty() == false) {
            params.insert({"log_level", log_level_s});
        } else {
            std::cout << "log_level: valid value 0..7\n";
            return EXIT_FAILURE;
        }
    } else {
        params.insert({"log_level", "6"});
    }

    bool security_protocol = kafka_params->exists("security_protocol");
    if (security_protocol == true) {
        libconfig::Setting &security_protocol =
            kafka_params->lookup("security_protocol");
        std::string security_protocol_s = security_protocol.c_str();
        if (security_protocol_s.empty() == false &&
            (security_protocol_s.compare("ssl") == 0 or
            security_protocol_s.compare("plaintext") == 0)) {
            params.insert({"security_protocol", security_protocol_s});
        } else {
            std::cout << "security_protocol: valid values <ssl | plaintext>\n";
            return EXIT_FAILURE;
        }
    } else {
        std::cout << "kafka-producer: security_protocol mandatory\n";
        throw libconfig::SettingNotFoundException("security_protocol");
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
                std::cout << "security_protocol: valid values not empty\n";
                return EXIT_FAILURE;
            }
        } else {
            std::cout <<
                "kafka-producer: security_protocol options mandatory\n";
            throw libconfig::SettingNotFoundException(
                                                "security_protocol options");
        }
    } else {
        params.insert({"ssl_key_location", "NULL"});
        params.insert({"ssl_certificate_location", "NULL"});
        params.insert({"ssl_ca_location", "NULL"});
    }

    return EXIT_SUCCESS;
}

