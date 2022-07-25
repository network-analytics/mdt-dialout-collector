#ifndef _CFG_HANDLER_H_
#define _CFG_HANDLER_H_

#include <iostream>
#include <libconfig.h++>
#include <map>


class CfgHandler {
public:
    //TBC

protected:
    const std::string mdt_dialout_collector_conf =
        "/etc/opt/mdt-dialout-collector/mdt_dialout_collector.conf";
    std::map<std::string, std::string> parameters;
};

// Main configuration parameters
class MainCfgHandler : public CfgHandler {
public:
    // Params are initialized within the constructor
    MainCfgHandler();

    // Setters - directly from the configuration file
    int lookup_main_parameters(std::string cfg_path,
        std::map<std::string, std::string>& params);

    // Getters
    std::string get_iface() const {
        return iface; };
    std::string get_ipv4_socket_cisco() const {
        return ipv4_socket_cisco; };
    std::string get_ipv4_socket_juniper() const {
        return ipv4_socket_juniper; };
    std::string get_ipv4_socket_huawei() const {
        return ipv4_socket_huawei; };
    std::string get_cisco_workers() const {
        return cisco_workers; };
    std::string get_juniper_workers() const {
        return juniper_workers; };
    std::string get_huawei_workers() const {
        return huawei_workers; };
private:
    std::string iface;
    std::string ipv4_socket_cisco;
    std::string ipv4_socket_juniper;
    std::string ipv4_socket_huawei;
    std::string cisco_workers;
    std::string juniper_workers;
    std::string huawei_workers;
};

// Data manipulation configuration parameters
class DataManipulationCfgHandler : public CfgHandler {
public:
    // Params are initialized within the constructor
    DataManipulationCfgHandler();

    // Setters - directly from the configuration file
    int lookup_main_parameters(std::string cfg_path,
        std::map<std::string, std::string>& params);

    // Getters
    std::string get_enable_cisco_message_to_json_string() const {
        return enable_cisco_message_to_json_string; };
    std::string get_enable_cisco_gpbkv2json() const {
        return enable_cisco_gpbkv2json; };
    std::string get_enable_label_encode_as_map() const {
        return enable_label_encode_as_map; };
private:
    std::string enable_cisco_message_to_json_string;
    std::string enable_cisco_gpbkv2json;
    std::string enable_label_encode_as_map;
};

// Kafka configuration parameters
class KafkaCfgHandler : public CfgHandler {
public:
    // Params are initialized within the constructor
    KafkaCfgHandler();

    // Setters - directly from the configuration file
    int lookup_kafka_parameters(std::string cfg_path,
        std::map<std::string, std::string>& params);

    // Getters
    std::string get_kafka_topic() const {
        return topic; };
    std::string get_kafka_bootstrap_servers() const {
        return bootstrap_servers; };
    std::string get_kafka_enable_idempotence() const {
        return enable_idempotence; };
    std::string get_kafka_client_id() const {
        return client_id; };
    std::string get_kafka_security_protocol() const {
        return security_protocol; };
    std::string get_kafka_ssl_key_location() const {
        return ssl_key_location; };
    std::string get_kafka_ssl_certificate_location() const {
        return ssl_certificate_location; };
    std::string get_kafka_ssl_ca_location() const {
        return ssl_ca_location; };
    std::string get_kafka_log_level() const {
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

