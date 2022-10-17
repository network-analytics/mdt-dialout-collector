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
// mdt-dialout-collector Library headers
#include "logs_handler.h"


// Centralizing config parameters
extern std::map<std::string, std::string> logs_cfg_parameters;
extern std::map<std::string, std::string> main_cfg_parameters;
extern std::map<std::string, std::string> data_manipulation_cfg_parameters;
extern std::map<std::string, std::string> kafka_delivery_cfg_parameters;


class CfgHandler {
public:
    CfgHandler() { spdlog::get("multi-logger-boot")->
        debug("constructor: CfgHandler()"); };
    ~CfgHandler() { spdlog::get("multi-logger")->
        debug("destructor: ~CfgHandler()"); };
    static bool set_parameters(libconfig::Config &params,
        const std::string &cfg_path);
    void set_cfg_path(const std::string &cfg) {
        this->mdt_dialout_collector_conf = cfg; };
    const std::string &get_cfg_path() const {
        return this->mdt_dialout_collector_conf; };
    // Single getter to handle them all
    std::map<std::string, std::string>
        &get_logs_parameters() {
            return logs_parameters; };
    std::map<std::string, std::string>
        &get_main_parameters() {
            return main_parameters; };
    std::map<std::string, std::string>
        &get_data_manipulation_parameters(){
            return data_manipulation_parameters; };
    std::map<std::string, std::string>
        &get_kafka_parameters() {
            return kafka_parameters; };
protected:
    std::string mdt_dialout_collector_conf;
    std::map<std::string, std::string> logs_parameters;
    std::map<std::string, std::string> main_parameters;
    std::map<std::string, std::string> data_manipulation_parameters;
    std::map<std::string, std::string> kafka_parameters;
};

// Logs Configs Handler
class LogsCfgHandler {
public:
    LogsCfgHandler() { spdlog::get("multi-logger-boot")->
        debug("constructor: LogsCfgHandler()"); };
    ~LogsCfgHandler() { spdlog::get("multi-logger")->
        debug("destructor: ~LogsCfgHandler()"); };
    // Setters - directly from the configuration file
    bool lookup_logs_parameters(const std::string &cfg_path,
        std::map<std::string, std::string> &params);

    // Getters
    //const std::string &get_syslog() const {
    //    return syslog; };
    //const std::string &get_syslog_facility() const {
    //    return syslog_facility; };
    //const std::string &get_console_log() const {
    //    return console_log; };
    //const std::string &get_spdlog_level() const {
    //    return spdlog_level; };
private:
    const std::string syslog;
    const std::string syslog_facility;
    const std::string console_log;
    const std::string spdlog_level;
};

// Main configuration parameters
class MainCfgHandler {
public:
    // Params are initialized within the constructor
    MainCfgHandler() {spdlog::get("multi-logger")->
        debug("constructor: MainCfgHandler()"); };
    ~MainCfgHandler() { spdlog::get("multi-logger")->
        debug("destructor: ~MainCfgHandler()"); };

    // Setters - directly from the configuration file
    bool lookup_main_parameters(const std::string &cfg_path,
        std::map<std::string, std::string> &params);

    // Getters
    //const std::string &get_writer_id() const {
    //    return writer_id; };
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
    //const std::string &get_replies_cisco() const {
    //    return replies_cisco; };
    //const std::string &get_replies_juniper() const {
    //    return replies_juniper; };
    //const std::string &get_replies_huawei() const {
    //    return replies_huawei; };
    //const std::string &get_cisco_workers() const {
    //    return cisco_workers; };
    //const std::string &get_juniper_workers() const {
    //    return juniper_workers; };
    //const std::string &get_huawei_workers() const {
    //    return huawei_workers; };
private:
    const std::string writer_id;
    const std::string core_pid_folder;
    const std::string iface;
    const std::string ipv4_socket_cisco;
    const std::string ipv4_socket_juniper;
    const std::string ipv4_socket_huawei;
    const std::string replies_cisco;
    const std::string replies_juniper;
    const std::string replies_huawei;
    const std::string cisco_workers;
    const std::string juniper_workers;
    const std::string huawei_workers;
};

// Data manipulation configuration parameters
class DataManipulationCfgHandler {
public:
    // Params are initialized within the constructor
    DataManipulationCfgHandler() { spdlog::get("multi-logger")->
        debug("constructor: DataManipulationCfgHandler()"); };
    ~DataManipulationCfgHandler() { spdlog::get("multi-logger")->
        debug("destructor: ~DataManipulationCfgHandler()"); };

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
    //const std::string &get_enable_label_encode_as_map_ptm() const {
    //    return enable_label_encode_as_map_ptm; };
    //const std::string &get_label_map_ptm_path() const {
    //    return label_map_ptm_path; };
private:
    const std::string enable_cisco_message_to_json_string;
    const std::string enable_cisco_gpbkv2json;
    const std::string enable_label_encode_as_map;
    const std::string label_map_csv_path;
    const std::string enable_label_encode_as_map_ptm;
    const std::string label_map_ptm_path;
};

// Kafka configuration parameters
class KafkaCfgHandler {
public:
    // Params are initialized within the constructor
    KafkaCfgHandler() { spdlog::get("multi-logger")->
        debug("constructor: KafkaCfgHandler()"); };
    ~KafkaCfgHandler() { spdlog::get("multi-logger")->
        debug("destructor: ~KafkaCfgHandler()"); };

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
    const std::string topic;
    const std::string bootstrap_servers;
    const std::string enable_idempotence;
    const std::string client_id;
    const std::string security_protocol;
    const std::string ssl_key_location;
    const std::string ssl_certificate_location;
    const std::string ssl_ca_location;
    const std::string log_level;
};

#endif

