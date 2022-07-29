// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _CFG_HANDLER_H_
#define _CFG_HANDLER_H_

// C++ Standard Library headers
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <map>
// External Library headers
#include <libconfig.h++>


// Centralizing config parameters
extern std::map<std::string, std::string> main_cfg_parameters;
extern std::map<std::string, std::string> data_manipulation_cfg_parameters;
extern std::map<std::string, std::string> data_delivery_cfg_parameters;

class CfgHandler {
public:
    CfgHandler() { std::cout << "CfgHandler()\n"; };
    bool set_parameters(std::unique_ptr<libconfig::Config> &params,
        const std::string &cfg_path);
    // Single getter to handle them all
    const std::map<std::string, std::string> &get_parameters() const {
        return parameters; };
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
    bool lookup_main_parameters(const std::string &cfg_path,
        std::map<std::string, std::string> &params);

    // Getters
    //const std::string &get_core_pid_folder() const {
    //    return core_pid_folder; };
    //const std::string &get_iface() const {
    //    return iface; };
    //const std::string &get_ipv4_socket_cisco() const {
    //    return ipv4_socket_cisco; };
    //const std::string &get_ipv4_socket_juniper() const {
    //    return ipv4_socket_juniper; };
    //const std::string &get_ipv4_socket_huawei() const {
    //    return ipv4_socket_huawei; };
    //const std::string &get_cisco_workers() const {
    //    return cisco_workers; };
    //const std::string &get_juniper_workers() const {
    //    return juniper_workers; };
    //const std::string &get_huawei_workers() const {
    //    return huawei_workers; };
private:
    std::string core_pid_folder;
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
    bool lookup_data_manipulation_parameters(const std::string &cfg_path,
        std::map<std::string, std::string> &params);

    // Getters
    //const std::string &get_enable_cisco_message_to_json_string() const {
    //    return enable_cisco_message_to_json_string; };
    //const std::string &get_enable_cisco_gpbkv2json() const {
    //    return enable_cisco_gpbkv2json; };
    //const std::string &get_enable_label_encode_as_map() const {
    //    return enable_label_encode_as_map; };
    //const std::string &get_label_map_csv_path() const {
    //    return label_map_csv_path; };
private:
    std::string enable_cisco_message_to_json_string;
    std::string enable_cisco_gpbkv2json;
    std::string enable_label_encode_as_map;
    std::string label_map_csv_path;
};

// Kafka configuration parameters
class KafkaCfgHandler : public CfgHandler {
public:
    // Params are initialized within the constructor
    KafkaCfgHandler();

    // Setters - directly from the configuration file
    bool lookup_kafka_parameters(const std::string &cfg_path,
        std::map<std::string, std::string> &params);

    // Getters
    //const std::string &get_kafka_topic() const {
    //    return topic; };
    //const std::string &get_kafka_bootstrap_servers() const {
    //    return bootstrap_servers; };
    //const std::string &get_kafka_enable_idempotence() const {
    //    return enable_idempotence; };
    //const std::string &get_kafka_client_id() const {
    //    return client_id; };
    //const std::string &get_kafka_security_protocol() const {
    //    return security_protocol; };
    //const std::string &get_kafka_ssl_key_location() const {
    //    return ssl_key_location; };
    //const std::string &get_kafka_ssl_certificate_location() const {
    //    return ssl_certificate_location; };
    //const std::string &get_kafka_ssl_ca_location() const {
    //    return ssl_ca_location; };
    //const std::string &get_kafka_log_level() const {
    //    return log_level; };
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

