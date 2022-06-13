#ifndef _CFG_HANDLER_H_
#define _CFG_HANDLER_H_

#include <iostream>
#include <libconfig.h++>


class KafkaCfgHandler final {
public:
    // Params are initialized within the constructor
    KafkaCfgHandler();

    // Setters - directly from the configuration file
    int lookup_kafka_parameters(std::string cfg_path);

    // Getters
    std::string get_kafka_topic() {
                      return topic; };
    std::string get_kafka_bootstrap_servers() {
                      return bootstrap_servers; };
    std::string get_kafka_enable_idempotence() {
                      return enable_idempotence; };
    std::string get_kafka_client_id() {
                      return client_id; };
    std::string get_kafka_security_protocol() {
                      return security_protocol; };
    std::string get_kafka_ssl_key_location() {
                      return ssl_key_location; };
    std::string get_kafka_ssl_certificate_location() {
                      return ssl_certificate_location; };
    std::string get_kafka_ssl_ca_location() {
                      return ssl_ca_location; };
    std::string get_kafka_log_level() {
                            return log_level; };
private:
    std::string topic;
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

