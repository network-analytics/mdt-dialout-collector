#ifndef _CFG_HANDLER_H_
#define _CFG_HANDLER_H_

#include <iostream>
#include <libconfig.h++>


class KafkaCfgHandler final {
public:
    KafkaCfgHandler();
    int lookup_kafka_parameters(std::string cfg_path);
    std::string get_kafka_topic() { return topic; };
    std::string get_kafka_bootstrap_servers() { return bootstrap_servers; };
    std::string get_kafka_enable_idempotence() { return enable_idempotence; };
    std::string get_kafka_client_id() { return client_id; };
    std::string get_kafka_security_protocol() { return security_protocol; };
    std::string get_kafka_ssl_key_location() { return ssl_key_location; };
    std::string get_kafka_ssl_certificate_location() {
                                            return ssl_certificate_location; };
    std::string get_kafka_ssl_ca_location() { return ssl_ca_location; };
    std::string get_kafka_log_level() { return log_level; };
private:
    const char *topic;
    const char *bootstrap_servers;
    const char *enable_idempotence;
    const char *client_id;
    const char *security_protocol;
    const char *ssl_key_location;
    const char *ssl_certificate_location;
    const char *ssl_ca_location;
    const char *log_level;
};

#endif

