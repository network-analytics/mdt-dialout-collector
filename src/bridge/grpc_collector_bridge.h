// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _GRPC_COLLECTOR_BRIDGE_H_
#define _GRPC_COLLECTOR_BRIDGE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <zmq.h>


#ifdef __cplusplus
extern "C" {
#endif
    #define MAX_WORKERS 5

    typedef struct {
        char *writer_id;
        char *iface;
        char *ipv4_socket_cisco;
        char *ipv4_socket_juniper;
        char *ipv4_socket_nokia;
        char *ipv4_socket_huawei;
        char *cisco_workers;
        char *juniper_workers;
        char *nokia_workers;
        char *huawei_workers;
        char *replies_cisco;
        char *replies_juniper;
        char *replies_nokia;
        char *replies_huawei;
        char *syslog;
        char *syslog_facility;
        char *syslog_ident;
        char *console_log;
        char *spdlog_level;
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
    } __attribute__ ((packed)) grpc_payload;

    extern Options *InitOptions();
    extern void InitGrpcPayload(grpc_payload **pload_, const char *event_type,
        const char *serialization, const char *writer_id,
        const char *telemetry_node, const char *telemetry_port,
        const char *telemetry_data);

    extern void FreeOptions(Options *opts);
    extern void free_grpc_payload(grpc_payload *pload);

    extern void start_grpc_dialout_collector(const char *cfg_path,
        const char *zmq_uri);
    /* graceful shutdown: drains live Srv instances, joins all workers.
     * safe to call at any point; idempotent; returns 1. */
    extern int stop_grpc_dialout_collector(void);
    /* returns 1 on success, 0 on cfg validation failure */
    extern int LoadOptions(const char *cfg_path,
        const char *zmq_uri);
    extern void *VendorThread(void *vendor);
    extern void LoadThreads(pthread_t *workers_vec, const char *socket_str,
        const char *replies_str, const char *workers_str);

#ifdef __cplusplus
}
#endif

#endif

