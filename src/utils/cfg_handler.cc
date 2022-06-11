#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <memory>
#include <libconfig.h++>
#include "cfg_handler.h"


std::unique_ptr<libconfig::Config> kafka_params(new libconfig::Config());

KafkaCfgHandler::KafkaCfgHandler()
{

    if (lookup_kafka_parameters(
    "/home/toto/Projects/mdt-dialout-collector/configs/kafka_producer.cfg")) {
        topic = kafka_params->lookup("topic");
        bootstrap_servers = kafka_params->lookup("bootstrap_servers");
        enable_idempotence = kafka_params->lookup("enable_idempotence");
        client_id = kafka_params->lookup("client_id");
        security_protocol = kafka_params->lookup("security_protocol");
        ssl_key_location = kafka_params->lookup("ssl_key_location");
        ssl_certificate_location = kafka_params->lookup("certificate_location");
        ssl_ca_location = kafka_params->lookup("ssl_ca_location");
        log_level = kafka_params->lookup("log_level");
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
