// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "cfg_wrapper.h"


bool CfgWrapper::BuildCfgWrapper(
    // main
    const std::string &writer_id,
    const std::string &iface,
    const std::string &ipv4_socket_cisco,
    const std::string &ipv4_socket_juniper,
    const std::string &ipv4_socket_nokia,
    const std::string &ipv4_socket_huawei,
    //const std::string &core_pid_folder
    const std::string &cisco_workers,
    const std::string &juniper_workers,
    const std::string &nokia_workers,
    const std::string &huawei_workers,
    const std::string &replies_cisco,
    const std::string &replies_juniper,
    const std::string &replies_nokia,
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
    const std::string &enable_label_encode_as_map_ptm)
{
    set_writer_id(writer_id);
    set_iface(iface);
    set_ipv4_socket_cisco(ipv4_socket_cisco);
    set_ipv4_socket_juniper(ipv4_socket_juniper);
    set_ipv4_socket_nokia(ipv4_socket_nokia);
    set_ipv4_socket_huawei(ipv4_socket_huawei);
    //set_core_pid_folder(core_pid_folder);
    set_cisco_workers(cisco_workers);
    set_juniper_workers(juniper_workers);
    set_nokia_workers(nokia_workers);
    set_huawei_workers(huawei_workers);
    set_replies_cisco(replies_cisco);
    set_replies_juniper(replies_juniper);
    set_replies_nokia(replies_nokia);
    set_replies_huawei(replies_huawei);
    set_syslog(syslog);
    set_syslog_facility(syslog_facility);
    set_syslog_ident(syslog_ident);
    set_console_log(console_log);
    set_spdlog_level(spdlog_level);
    set_enable_cisco_gpbkv2json(enable_cisco_gpbkv2json);
    set_enable_cisco_message_to_json_string(
        enable_cisco_message_to_json_string);
    set_enable_label_encode_as_map(enable_label_encode_as_map);
    set_enable_label_encode_as_map_ptm(enable_label_encode_as_map_ptm);

    return true;
}

