#ifndef _CFG_HANDLER_H_
#define _CFG_HANDLER_H_

#include <iostream>
#include <libconfig.h++>


class CfgHandler {
public:
    //TBC

protected:
    const std::string mdt_dialout_collector_conf =
                "/etc/opt/mdt-dialout-collector/mdt_dialout_collector.conf";
    std::map<std::string, std::string> parameters;
};

class MainCfgHandler : public CfgHandler {
public:
    // Params are initialized within the constructor
    MainCfgHandler();

    // Setters - directly from the configuration file
    int lookup_main_parameters(std::string cfg_path,
                                std::map<std::string, std::string>& params);

    // Getters
    std::string get_iface() {
                      return iface; };
    std::string get_ipv4_socket_v1() {
                      return ipv4_socket_v1; };
    std::string get_ipv4_socket_v2() {
                      return ipv4_socket_v2; };
private:
    std::string iface;
    std::string ipv4_socket_v1;
    std::string ipv4_socket_v2;
};

class KafkaCfgHandler : public CfgHandler {
public:
    // Params are initialized within the constructor
    KafkaCfgHandler();

    // Setters - directly from the configuration file
    int lookup_kafka_parameters(std::string cfg_path,
                                std::map<std::string, std::string>& params);

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

