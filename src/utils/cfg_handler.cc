#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <memory>
#include <libconfig.h++>
#include "cfg_handler.h"


std::unique_ptr<libconfig::Config> kafka_params(new libconfig::Config());
const std::string kafka_cfg = 
    "/home/tzhcusa1/Projects/mdt-dialout-collector/configs/kafka_producer.cfg";

KafkaCfgHandler::KafkaCfgHandler()
{
    if (!lookup_kafka_parameters(kafka_cfg)) {
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

        this->topic =
            (const char *) topic_;
        this->bootstrap_servers =
            (const char *) bootstrap_servers_;
        this->enable_idempotence =
            (const char *) enable_idempotence_;
        this->client_id =
            (const char *) client_id_;
        this->security_protocol =
            (const char *) security_protocol_;
        this->ssl_key_location =
            (const char *) ssl_key_location_;
        this->ssl_certificate_location =
            (const char *) ssl_certificate_location_;
        this->ssl_ca_location =
            (const char *) ssl_ca_location_;
        this->log_level =
            (const char *) log_level_;
    }   
}

int KafkaCfgHandler::lookup_kafka_parameters(std::string cfg_path)
{
    try {
        kafka_params->readFile(cfg_path.c_str());
    } catch (const libconfig::FileIOException &fioex) {
        std::cerr << "I/O error while reading file." << std::endl;
        return(EXIT_FAILURE);
    } catch(const libconfig::ParseException &pex) {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
                                    << " - " << pex.getError() << std::endl;
        return(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
