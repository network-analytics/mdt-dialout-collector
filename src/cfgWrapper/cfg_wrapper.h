// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _CFG_WRAPPER_H_
#define _CFG_WRAPPER_H_

// C++ Standard Library headers
#include <ctime>
#include <iostream>
// External Library headers

// mdt-dialout-collector Library headers
#include "../utils/logs_handler.h"
#include "../utils/cfg_handler.h"


// C++ Class
class CfgWrapper {
public:
    CfgWrapper() {
        spdlog::get("multi-logger")->
            debug("constructor: CfgWrapper()"); };
    ~CfgWrapper() {
        spdlog::get("multi-logger")->
            debug("destructor: ~CfgWrapper()"); };

    bool BuildCfgWrapper(
        // main
        const std::string &writer_id,
        const std::string &iface,
        const std::string &ipv4_socket_cisco,
        const std::string &ipv4_socket_juniper,
        const std::string &ipv4_socket_huawei,
        //const std::string &core_pid_folder
        const std::string &cisco_workers,
        const std::string &juniper_workers,
        const std::string &huawei_workers,
        const std::string &replies_cisco,
        const std::string &replies_juniper,
        const std::string &replies_huawei,
        // logging
        const std::string &syslog,
        const std::string &syslog_facility,
        const std::string &syslog_ident,
        const std::string &console_log,
        const std::string &spdlog_level,
        // data-manipualtion
        const std::string &enable_cisco_gpbkv2json,
        const std::string &enable_cisco_message_to_json_string,
        const std::string &enable_label_encode_as_map,
        const std::string &enable_label_encode_as_map_ptm
    );

    // Setters
    void set_writer_id(const std::string &writer_id) {
        this->writer_id = writer_id;
    };
    void set_iface(const std::string &iface) {
        this->iface = iface;
    };
    void set_ipv4_socket_cisco(const std::string &ipv4_socket_cisco) {
        this->ipv4_socket_cisco = ipv4_socket_cisco;
    };
    void set_ipv4_socket_juniper(const std::string &ipv4_socket_juniper) {
        this->ipv4_socket_juniper = ipv4_socket_juniper;
    };
    void set_ipv4_socket_huawei(const std::string &ipv4_socket_huawei) {
        this->ipv4_socket_huawei = ipv4_socket_huawei;
    };
    //void set_core_pid_folder(const std::string &core_pid_folder) {
    //    this->core_pid_folder = core_pid_folder;
    //};
    void set_cisco_workers(const std::string &cisco_workers) {
        this->cisco_workers = cisco_workers;
    };
    void set_juniper_workers(const std::string &juniper_workers) {
        this->juniper_workers = juniper_workers;
    };
    void set_huawei_workers(const std::string &huawei_workers) {
        this->huawei_workers = huawei_workers;
    };
    void set_replies_cisco(const std::string &replies_cisco) {
        this->replies_cisco = replies_cisco;
    };
    void set_replies_juniper(const std::string &replies_juniper) {
        this->replies_juniper = replies_juniper;
    };
    void set_replies_huawei(const std::string &replies_huawei) {
        this->replies_huawei = replies_huawei;
    };
    void set_syslog(const std::string &syslog) {
        this->syslog = syslog;
    };
    void set_syslog_facility(const std::string &syslog_facility) {
        this->syslog_facility = syslog_facility;
    };
    void set_syslog_ident(const std::string &syslog_ident) {
        this->syslog_ident = syslog_ident;
    };
    void set_console_log(const std::string &console_log) {
        this->console_log = console_log;
    };
    void set_spdlog_level(const std::string &spdlog_level) {
        this->spdlog_level = spdlog_level;
    };
    void set_enable_cisco_gpbkv2json(
        const std::string &enable_cisco_gpbkv2json) {
        this->enable_cisco_gpbkv2json = enable_cisco_gpbkv2json;
    };
    void set_enable_cisco_message_to_json_string(
        const std::string &enable_cisco_message_to_json_string) {
        this->enable_cisco_message_to_json_string =
            enable_cisco_message_to_json_string;
    };
    void set_enable_label_encode_as_map(
        const std::string &enable_label_encode_as_map) {
        this->enable_label_encode_as_map = enable_label_encode_as_map;
    };
    void set_enable_label_encode_as_map_ptm(
        const std::string &enable_label_encode_as_map_ptm) {
        this->enable_label_encode_as_map_ptm = enable_label_encode_as_map_ptm;
    };

    // Getters
    std::string &get_writer_id() { return this->writer_id; };
    std::string &get_iface() { return this->iface; };
    std::string &get_ipv4_socket_cisco() { return this->ipv4_socket_cisco; };
    std::string &get_ipv4_socket_juniper() {
        return this->ipv4_socket_juniper; };
    std::string &get_ipv4_socket_huawei() { return this->ipv4_socket_huawei; };
    //std::string &get_core_pid_folder() { return this->core_pid_folder; };
    std::string &get_cisco_workers() { return this->cisco_workers; };
    std::string &get_juniper_workers() { return this->juniper_workers; };
    std::string &get_huawei_workers() { return this->huawei_workers; };
    std::string &get_replies_cisco() { return this->replies_cisco; };
    std::string &get_replies_juniper() { return this->replies_juniper; };
    std::string &get_replies_huawei() { return this->replies_huawei; };
    std::string &get_syslog() { return this->syslog; };
    std::string &get_syslog_facility() { return this->syslog_facility; };
    std::string &get_syslog_ident() { return this->syslog_ident; };
    std::string &get_console_log() { return this->console_log; };
    std::string &get_spdlog_level() { return this->spdlog_level; };
    std::string &get_enable_cisco_gpbkv2json() {
        return this->enable_cisco_gpbkv2json; };
    std::string &get_enable_cisco_message_to_json_string() {
        return this->enable_cisco_message_to_json_string; };
    std::string &get_enable_label_encode_as_map() {
        return this->enable_label_encode_as_map; };
    std::string &get_enable_label_encode_as_map_ptm() {
        return this->enable_label_encode_as_map_ptm; };
private:
    // main
    std::string writer_id;
    std::string iface;
    std::string ipv4_socket_cisco;
    std::string ipv4_socket_juniper;
    std::string ipv4_socket_huawei;
    //std::string core_pid_folder;
    std::string cisco_workers;
    std::string juniper_workers;
    std::string huawei_workers;
    std::string replies_cisco;
    std::string replies_juniper;
    std::string replies_huawei;
    // logging
    std::string syslog;
    std::string syslog_facility;
    std::string syslog_ident;
    std::string console_log;
    std::string spdlog_level;
    // data-manipualtion
    std::string enable_cisco_gpbkv2json;
    std::string enable_cisco_message_to_json_string;
    std::string enable_label_encode_as_map;
    std::string enable_label_encode_as_map_ptm;
};

#endif

