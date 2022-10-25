// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include "bridge_lib.h"


Options *InitOptions()
{
    CfgWrapper cfg_wrapper;
    cfg_wrapper.BuildCfgWrapper(
        main_cfg_parameters.at("writer_id"),
        main_cfg_parameters.at("iface"),
        main_cfg_parameters.at("ipv4_socket_cisco"),
        main_cfg_parameters.at("ipv4_socket_juniper"),
        main_cfg_parameters.at("ipv4_socket_huawei"),
        /*main_cfg_parameters.at("core_pid_folder"),*/
        main_cfg_parameters.at("cisco_workers"),
        main_cfg_parameters.at("juniper_workers"),
        main_cfg_parameters.at("huawei_workers"),
        main_cfg_parameters.at("replies_cisco"),
        main_cfg_parameters.at("replies_juniper"),
        main_cfg_parameters.at("replies_huawei"),
        logs_cfg_parameters.at("syslog"),
        logs_cfg_parameters.at("syslog_facility"),
        logs_cfg_parameters.at("syslog_ident"),
        logs_cfg_parameters.at("console_log"),
        logs_cfg_parameters.at("spdlog_level"),
        data_manipulation_cfg_parameters.at("enable_cisco_gpbkv2json"),
        data_manipulation_cfg_parameters.at(
            "enable_cisco_message_to_json_string"),
        data_manipulation_cfg_parameters.at("enable_label_encode_as_map"),
        data_manipulation_cfg_parameters.at("enable_label_encode_as_map_ptm"));

    Options *opts = (Options *) malloc(sizeof(Options));

    const char *writer_id = cfg_wrapper.get_writer_id().c_str();
    opts->writer_id = strndup(writer_id, strlen(writer_id));

    const char *iface = cfg_wrapper.get_iface().c_str();
    opts->iface = strndup(iface, strlen(iface));

    const char *ipv4_socket_cisco =
        cfg_wrapper.get_ipv4_socket_cisco().c_str();
    opts->ipv4_socket_cisco =
        strndup(ipv4_socket_cisco, strlen(ipv4_socket_cisco));

    const char *ipv4_socket_juniper =
        cfg_wrapper.get_ipv4_socket_juniper().c_str();
    opts->ipv4_socket_juniper =
        strndup(ipv4_socket_juniper, strlen(ipv4_socket_juniper));

    const char *ipv4_socket_huawei =
        cfg_wrapper.get_ipv4_socket_huawei().c_str();
    opts->ipv4_socket_huawei =
        strndup(ipv4_socket_huawei, strlen(ipv4_socket_huawei));

    /*const char *core_pid_folder =
     * cfg_wrapper.get_core_pid_folder().c_str();
    opts->core_pid_folder =
        strndup(core_pid_folder, strlen(core_pid_folder));*/

    const char *cisco_workers = cfg_wrapper.get_cisco_workers().c_str();
    opts->cisco_workers = strndup(cisco_workers, strlen(cisco_workers));

    const char *juniper_workers = cfg_wrapper.get_juniper_workers().c_str();
    opts->juniper_workers = strndup(juniper_workers, strlen(juniper_workers));

    const char *huawei_workers = cfg_wrapper.get_huawei_workers().c_str();
    opts->huawei_workers = strndup(huawei_workers, strlen(huawei_workers));

    const char *replies_cisco = cfg_wrapper.get_replies_cisco().c_str();
    opts->replies_cisco = strndup(replies_cisco, strlen(replies_cisco));

    const char *replies_juniper = cfg_wrapper.get_replies_juniper().c_str();
    opts->replies_juniper = strndup(replies_juniper, strlen(replies_juniper));

    const char *replies_huawei = cfg_wrapper.get_replies_huawei().c_str();
    opts->replies_huawei = strndup(replies_huawei, strlen(replies_huawei));

    const char *syslog = cfg_wrapper.get_syslog().c_str();
    opts->syslog = strndup(syslog, strlen(syslog));

    const char *syslog_facility = cfg_wrapper.get_syslog_facility().c_str();
    opts->syslog_facility = strndup(syslog_facility, strlen(syslog_facility));

    const char *syslog_ident = cfg_wrapper.get_syslog_ident().c_str();
    opts->syslog_ident = strndup(syslog_ident, strlen(syslog_ident));

    const char *console_log = cfg_wrapper.get_console_log().c_str();
    opts->console_log = strndup(console_log, strlen(console_log));

    const char *spdlog_level = cfg_wrapper.get_spdlog_level().c_str();
    opts->spdlog_level = strndup(spdlog_level, strlen(spdlog_level));

    const char *enable_cisco_gpbkv2json =
        cfg_wrapper.get_enable_cisco_gpbkv2json().c_str();
    opts->enable_cisco_gpbkv2json =
        strndup(enable_cisco_gpbkv2json, strlen(enable_cisco_gpbkv2json));

    const char *enable_cisco_message_to_json_string =
        cfg_wrapper.get_enable_cisco_message_to_json_string().c_str();
    opts->enable_cisco_message_to_json_string =
        strndup(enable_cisco_message_to_json_string,
            strlen(enable_cisco_message_to_json_string));

    const char *enable_label_encode_as_map =
        cfg_wrapper.get_enable_label_encode_as_map().c_str();
    opts->enable_label_encode_as_map =
        strndup(enable_label_encode_as_map,
            strlen(enable_label_encode_as_map));

    const char *enable_label_encode_as_map_ptm =
        cfg_wrapper.get_enable_label_encode_as_map_ptm().c_str();
    opts->enable_label_encode_as_map_ptm =
        strndup(enable_label_encode_as_map_ptm,
            strlen(enable_label_encode_as_map_ptm));

    return opts;
}

void InitPayload(Payload **pload_, const char *event_type,
    const char *serialization, const char *writer_id,
    const char *telemetry_node, const char *telemetry_port,
    const char *telemetry_data)
{
    Payload *pload = (Payload *) malloc(sizeof(Payload));

    pload->event_type = strndup(event_type, strlen(event_type));
    pload->serialization = strndup(serialization, strlen(serialization));
    pload->writer_id = strndup(writer_id, strlen(writer_id));
    pload->telemetry_node = strndup(telemetry_node, strlen(telemetry_node));
    pload->telemetry_port = strndup(telemetry_port, strlen(telemetry_port));
    pload->telemetry_data = strndup(telemetry_data, strlen(telemetry_data));

    *pload_ = pload;
}

void FreeOptions(Options *opts)
{
    free(opts->writer_id);
    free(opts->iface);
    free(opts->ipv4_socket_cisco);
    free(opts->ipv4_socket_juniper);
    free(opts->ipv4_socket_huawei);
    //free(opts->core_pid_folder);
    free(opts->cisco_workers);
    free(opts->juniper_workers);
    free(opts->huawei_workers);
    free(opts->replies_cisco);
    free(opts->replies_juniper);
    free(opts->replies_huawei);
    free(opts->syslog);
    free(opts->syslog_facility);
    free(opts->syslog_ident);
    free(opts->console_log);
    free(opts->spdlog_level);
    free(opts->enable_cisco_gpbkv2json);
    free(opts->enable_cisco_message_to_json_string);
    free(opts->enable_label_encode_as_map);
    free(opts->enable_label_encode_as_map_ptm);
    free(opts);
}

void FreePayload(Payload *pload)
{
    free(pload->event_type);
    free(pload->serialization);
    free(pload->writer_id);
    free(pload->telemetry_node);
    free(pload->telemetry_port);
    free(pload->telemetry_data);
    free(pload);
}

