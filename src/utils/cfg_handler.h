// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _CFG_HANDLER_H_
#define _CFG_HANDLER_H_

#include <iostream>
#include <filesystem>
#include <iomanip>
#include <map>
#include <libconfig.h++>
#include "logs_handler.h"


extern std::map<std::string, std::string> logs_cfg_parameters;
extern std::map<std::string, std::string> main_cfg_parameters;
extern std::map<std::string, std::string> data_manipulation_cfg_parameters;
extern std::map<std::string, std::string> kafka_delivery_cfg_parameters;
extern std::map<std::string, std::string> zmq_delivery_cfg_parameters;


class CfgHandler {
public:
    CfgHandler() { spdlog::get("multi-logger-boot")->
        debug("constructor: CfgHandler()"); };
    ~CfgHandler() {
        if (auto l = spdlog::get("multi-logger")) {
            l->debug("destructor: ~CfgHandler()");
        }
    };
    static bool set_parameters(libconfig::Config &params,
        const std::string &cfg_path);
    void set_cfg_path(const std::string &cfg) {
        this->mdt_dialout_collector_conf = cfg; };
    const std::string &get_cfg_path() const {
        return this->mdt_dialout_collector_conf; };
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
    std::map<std::string, std::string>
        &get_zmq_parameters() {
            return zmq_parameters; };
protected:
    std::string mdt_dialout_collector_conf;
    std::map<std::string, std::string> logs_parameters;
    std::map<std::string, std::string> main_parameters;
    std::map<std::string, std::string> data_manipulation_parameters;
    std::map<std::string, std::string> kafka_parameters;
    std::map<std::string, std::string> zmq_parameters;
};

class LogsCfgHandler {
public:
    LogsCfgHandler() { spdlog::get("multi-logger-boot")->
        debug("constructor: LogsCfgHandler()"); };
    ~LogsCfgHandler() {
        if (auto l = spdlog::get("multi-logger")) {
            l->debug("destructor: ~LogsCfgHandler()");
        }
    };
    bool lookup_logs_parameters(const std::string &cfg_path,
        std::map<std::string, std::string> &params);
private:
    const std::string syslog;
    const std::string syslog_facility;
    const std::string console_log;
    const std::string spdlog_level;
};

class MainCfgHandler {
public:
    MainCfgHandler() {spdlog::get("multi-logger")->
        debug("constructor: MainCfgHandler()"); };
    ~MainCfgHandler() {
        if (auto l = spdlog::get("multi-logger")) {
            l->debug("destructor: ~MainCfgHandler()");
        }
    };

    bool lookup_main_parameters(const std::string &cfg_path,
        std::map<std::string, std::string> &params);
private:
    const std::string writer_id;
    const std::string core_pid_folder;
    const std::string iface;
    const std::string socket_cisco;
    const std::string socket_juniper;
    const std::string socket_nokia;
    const std::string socket_huawei;
    const std::string replies_cisco;
    const std::string replies_juniper;
    const std::string replies_nokia;
    const std::string replies_huawei;
    const std::string cisco_workers;
    const std::string juniper_workers;
    const std::string nokia_workers;
    const std::string huawei_workers;
    const std::string data_delivery_method;
    const std::string so_bindtodevice_check;
};

class DataManipulationCfgHandler {
public:
    DataManipulationCfgHandler() { spdlog::get("multi-logger")->
        debug("constructor: DataManipulationCfgHandler()"); };
    ~DataManipulationCfgHandler() {
        if (auto l = spdlog::get("multi-logger")) {
            l->debug("destructor: ~DataManipulationCfgHandler()");
        }
    };

    bool lookup_data_manipulation_parameters(const std::string &cfg_path,
        std::map<std::string, std::string> &params);
private:
    const std::string enable_cisco_message_to_json_string;
    const std::string enable_cisco_gpbkv2json;
    const std::string enable_label_encode_as_map;
    const std::string label_map_csv_path;
    const std::string enable_label_encode_as_map_ptm;
    const std::string label_map_ptm_path;
};

class KafkaCfgHandler {
public:
    KafkaCfgHandler() { spdlog::get("multi-logger")->
        debug("constructor: KafkaCfgHandler()"); };
    ~KafkaCfgHandler() {
        if (auto l = spdlog::get("multi-logger")) {
            l->debug("destructor: ~KafkaCfgHandler()");
        }
    };

    bool lookup_kafka_parameters(const std::string &cfg_path,
        std::map<std::string, std::string> &params);
private:
    const std::string topic;
    const std::string bootstrap_servers;
    const std::string enable_idempotence;
    const std::string client_id;
    const std::string security_protocol;
    const std::string ssl_key_location;
    const std::string ssl_key_password;
    const std::string ssl_certificate_location;
    const std::string ssl_ca_location;
    const std::string enable_ssl_certificate_verification;
    const std::string log_level;
};

class ZmqCfgHandler {
public:
    ZmqCfgHandler() { spdlog::get("multi-logger")->
        debug("constructor: ZmqCfgHandler()"); };
    ~ZmqCfgHandler() {
        if (auto l = spdlog::get("multi-logger")) {
            l->debug("destructor: ~ZmqCfgHandler()");
        }
    };

    bool lookup_zmq_parameters(const std::string &zmq_uri,
        std::map<std::string, std::string> &params);
private:
    const std::string zmq_uri;
};

#endif
