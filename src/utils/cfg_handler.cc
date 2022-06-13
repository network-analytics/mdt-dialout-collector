#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <memory>
#include <map>
#include <string>
#include <libconfig.h++>
#include "cfg_handler.h"


extern const std::string kafka_cfg =
    "/home/toto/Projects/mdt-dialout-collector/configs/kafka_producer.cfg";
std::map<std::string, std::string> params;

KafkaCfgHandler::KafkaCfgHandler()
{
    if (!lookup_kafka_parameters(kafka_cfg, params)) {
        this->topic =
            params.at("topic").c_str();
        this->bootstrap_servers =
            params.at("bootstrap_servers").c_str();
        this->enable_idempotence =
            params.at("enable_idempotence").c_str();
        this->client_id =
            params.at("client_id").c_str();
        this->security_protocol =
            params.at("security_protocol").c_str();
        this->ssl_key_location =
            params.at("ssl_key_location").c_str();
        this->ssl_certificate_location =
            params.at("ssl_certificate_location").c_str();
        this->ssl_ca_location =
            params.at("ssl_ca_location").c_str();
        this->log_level =
            params.at("log_level").c_str();
    }
}

int KafkaCfgHandler::lookup_kafka_parameters(std::string cfg_path,
                                std::map<std::string, std::string>& params)
{
    std::unique_ptr<libconfig::Config> kafka_params(new libconfig::Config());

    try {
        kafka_params->readFile(cfg_path.c_str());
    } catch (const libconfig::FileIOException &fioex) {
        std::cout << "libconfig::FileIOException" << std::endl;
        return(EXIT_FAILURE);
    } catch(const libconfig::ParseException &pex) {
        std::cout << "libconfig::ParseException" << std::endl;
        return(EXIT_FAILURE);
    }

    try {
        libconfig::Setting& topic_ =
            kafka_params->lookup("topic");
        libconfig::Setting& bootstrap_servers_ =
            kafka_params->lookup("bootstrap_servers");
        libconfig::Setting& enable_idempotence_ =
            kafka_params->lookup("enable_idempotence");
        libconfig::Setting& client_id_ =
            kafka_params->lookup("client_id");
        libconfig::Setting& security_protocol_ =
            kafka_params->lookup("security_protocol");
        libconfig::Setting& ssl_key_location_ =
            kafka_params->lookup("ssl_key_location");
        libconfig::Setting& ssl_certificate_location_ =
            kafka_params->lookup("ssl_certificate_location");
        libconfig::Setting& ssl_ca_location_ =
            kafka_params->lookup("ssl_ca_location");
        libconfig::Setting& log_level_ =
            kafka_params->lookup("log_level");

        params.insert({"topic", topic_});
        params.insert({"bootstrap_servers", bootstrap_servers_});
        params.insert({"enable_idempotence", enable_idempotence_});
        params.insert({"client_id", client_id_});
        params.insert({"security_protocol", security_protocol_});
        params.insert({"ssl_key_location", ssl_key_location_});
        params.insert({"ssl_certificate_location", ssl_certificate_location_});
        params.insert({"ssl_ca_location", ssl_ca_location_});
        params.insert({"log_level", log_level_});
    } catch(libconfig::SettingNotFoundException &snfex) {
        std::cout << "libconfig::SettingNotFoundException" << std::endl;
        return(EXIT_FAILURE);
    } catch (libconfig::SettingTypeException &stex){
        std::cout << "libconfig::SettingTypeException" << std::endl;
        return(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

