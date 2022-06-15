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


MainCfgHandler::MainCfgHandler()
{
    if (!lookup_main_parameters(this->mdt_dialout_collector_conf,
                                this->parameters)) {
        this->iface =
            parameters.at("iface").c_str();
        this->ipv4_socket_cisco =
            parameters.at("ipv4_socket_cisco").c_str();
        this->ipv4_socket_huawei =
            parameters.at("ipv4_socket_huawei").c_str();
    }
}

int MainCfgHandler::lookup_main_parameters(std::string cfg_path,
                                std::map<std::string, std::string>& params)
{
    std::unique_ptr<libconfig::Config> main_params(new libconfig::Config());

    try {
        main_params->readFile(cfg_path.c_str());
    } catch (const libconfig::FileIOException &fioex) {
        std::cout << "libconfig::FileIOException" << std::endl;
        return(EXIT_FAILURE);
    } catch(const libconfig::ParseException &pex) {
        std::cout << "libconfig::ParseException" << std::endl;
        return(EXIT_FAILURE);
    }

    try {
        libconfig::Setting& iface =
            main_params->lookup("iface");
        libconfig::Setting& ipv4_socket_cisco =
            main_params->lookup("ipv4_socket_cisco");
        libconfig::Setting& ipv4_socket_huawei =
            main_params->lookup("ipv4_socket_huawei");

        params.insert({"iface", iface});
        params.insert({"ipv4_socket_cisco", ipv4_socket_cisco});
        params.insert({"ipv4_socket_huawei", ipv4_socket_cisco});
    } catch(libconfig::SettingNotFoundException &snfex) {
        std::cout << "libconfig::SettingNotFoundException" << std::endl;
        return(EXIT_FAILURE);
    } catch (libconfig::SettingTypeException &stex){
        std::cout << "libconfig::SettingTypeException" << std::endl;
        return(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

KafkaCfgHandler::KafkaCfgHandler()
{
    if (!lookup_kafka_parameters(this->mdt_dialout_collector_conf,
                                this->parameters)) {
        this->topic =
            parameters.at("topic").c_str();
        this->bootstrap_servers =
            parameters.at("bootstrap_servers").c_str();
        this->enable_idempotence =
            parameters.at("enable_idempotence").c_str();
        this->client_id =
            parameters.at("client_id").c_str();
        this->security_protocol =
            parameters.at("security_protocol").c_str();
        this->ssl_key_location =
            parameters.at("ssl_key_location").c_str();
        this->ssl_certificate_location =
            parameters.at("ssl_certificate_location").c_str();
        this->ssl_ca_location =
            parameters.at("ssl_ca_location").c_str();
        this->log_level =
            parameters.at("log_level").c_str();
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
        libconfig::Setting& topic =
            kafka_params->lookup("topic");
        libconfig::Setting& bootstrap_servers =
            kafka_params->lookup("bootstrap_servers");
        libconfig::Setting& enable_idempotence =
            kafka_params->lookup("enable_idempotence");
        libconfig::Setting& client_id =
            kafka_params->lookup("client_id");
        libconfig::Setting& security_protocol =
            kafka_params->lookup("security_protocol");
        libconfig::Setting& ssl_key_location =
            kafka_params->lookup("ssl_key_location");
        libconfig::Setting& ssl_certificate_location =
            kafka_params->lookup("ssl_certificate_location");
        libconfig::Setting& ssl_ca_location =
            kafka_params->lookup("ssl_ca_location");
        libconfig::Setting& log_level =
            kafka_params->lookup("log_level");

        params.insert({"topic", topic});
        params.insert({"bootstrap_servers", bootstrap_servers});
        params.insert({"enable_idempotence", enable_idempotence});
        params.insert({"client_id", client_id});
        params.insert({"security_protocol", security_protocol});
        params.insert({"ssl_key_location", ssl_key_location});
        params.insert({"ssl_certificate_location", ssl_certificate_location});
        params.insert({"ssl_ca_location", ssl_ca_location});
        params.insert({"log_level", log_level});
    } catch(libconfig::SettingNotFoundException &snfex) {
        std::cout << "libconfig::SettingNotFoundException" << std::endl;
        return(EXIT_FAILURE);
    } catch (libconfig::SettingTypeException &stex){
        std::cout << "libconfig::SettingTypeException" << std::endl;
        return(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

