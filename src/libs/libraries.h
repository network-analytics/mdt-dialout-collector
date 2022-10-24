// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _LIBS_H_
#define _LIBS_H_


typedef struct {
    char *event_type;
    char *serialization;
    char *writer_id;
    char *telemetry_node;
    char *telemetry_port;
    char *telemetry_data;
} __attribute__ ((packed)) Payload;

extern void InitPayload(Payload **pload_, const char *event_type,
    const char *serialization, const char *writer_id,
    const char *telemetry_node, const char *telemetry_port,
    const char *telemetry_data);
extern void FreePayload(Payload *pload);

#endif

