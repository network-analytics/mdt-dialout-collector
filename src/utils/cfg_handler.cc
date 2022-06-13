#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <memory>
#include <libconfig.h++>
#include "cfg_handler.h"


std::unique_ptr<libconfig::Config> kafka_params(new libconfig::Config());
//libconfig::Config kafka_params;

KafkaCfgHandler::KafkaCfgHandler()
{
    lookup_kafka_parameters(
    "/home/tzhcusa1/Projects/mdt-dialout-collector/configs/kafka_producer.cfg");

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

    topic = (const char *) topic_;
    bootstrap_servers = (const char *) bootstrap_servers_;
    enable_idempotence = (const char *) enable_idempotence_;
    client_id = (const char *) client_id_;
    security_protocol = (const char *) security_protocol_;
    ssl_key_location = (const char *) ssl_key_location_;
    ssl_certificate_location = (const char *) ssl_certificate_location_;
    ssl_ca_location = (const char *) ssl_ca_location_;
    log_level = (const char *) log_level_;
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
