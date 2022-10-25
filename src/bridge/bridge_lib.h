// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _BRIDGE_LIB_H_
#define _BRIDGE_LIB_H_

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "cfg_wrapper.h"

#define MAX_WORKERS 15


typedef struct {
    /* main */
    char *writer_id;
    char *iface;
    char *ipv4_socket_cisco;
    char *ipv4_socket_juniper;
    char *ipv4_socket_huawei;
    /* char *core_pid_folder; */
    /* workers */
    char *cisco_workers;
    char *juniper_workers;
    char *huawei_workers;
    /* replies */
    char *replies_cisco;
    char *replies_juniper;
    char *replies_huawei;
    /* logging */
    char *syslog;
    char *syslog_facility;
    char *syslog_ident;
    char *console_log;
    char *spdlog_level;
    /* data-flow manipulation */
    char *enable_cisco_gpbkv2json;
    char *enable_cisco_message_to_json_string;
    char *enable_label_encode_as_map;
    char *enable_label_encode_as_map_ptm;
} __attribute__ ((packed)) Options;

typedef struct {
    char *event_type;
    char *serialization;
    char *writer_id;
    char *telemetry_node;
    char *telemetry_port;
    char *telemetry_data;
} __attribute__ ((packed)) Payload;

extern Options *InitOptions();
extern void InitPayload(Payload **pload_, const char *event_type,
    const char *serialization, const char *writer_id,
    const char *telemetry_node, const char *telemetry_port,
    const char *telemetry_data);

extern void FreeOptions(Options *opts);
extern void FreePayload(Payload *pload);

extern void StartGrpcDialoutCollector(pthread_t *workers);
extern void LoadOptions();
extern void *VendorThread(void *vendor);
extern void LoadThreads(pthread_t *workers_vec, const char *ipv4_socket_str,
    const char *replies_str, const char *workers_str);

#endif

