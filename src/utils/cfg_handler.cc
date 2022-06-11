#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <libconfig.h++>
#include "cfg_handler.h"


KafkaCfgHandler::KafkaCfgHandler()
{
    libconfig::Config kafka_params;
    kafka_params.setIncludeDir("configs/");
    kafka_params.readFile("main.cfg");
    std::string test = kafka_params.lookup("bootstrap_servers");
    std::cout << test << std::endl;
}

/*
int KafkaCfgHandler::lookup_kafka_parameters(
										libconfig::Config kafka_parameters)
{
    kafka_parameters.setIncludeDir("../../configs");

    try {
        kafka_parameters.readFile("main.cfg");
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


void KafkaCfgHandler::get_kafka_parameters()
{

}
*/

