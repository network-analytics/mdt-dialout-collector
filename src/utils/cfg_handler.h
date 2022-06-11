#ifndef _CFG_HANDLER_H_
#define _CFG_HANDLER_H_

#include <iostream>
//#include <iomanip>
//#include <cstdlib>
#include <libconfig.h++>


class KafkaCfgHandler final {
public:
    KafkaCfgHandler();
    //int lookup_kafka_parameters(libconfig::Config kafka_parameters);
    //void get_kafka_parameters();
private:
    std::string bootstrap_servers;
    std::string enable_idempotence;
    std::string client_id;
    std::string security_protocol;
    std::string ssl_key_location;
    std::string ssl_certificate_location;
    std::string ssl_ca_location;
    std::string log_level;
};

#endif

