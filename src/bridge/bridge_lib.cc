// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include <stdlib.h>
#include <string.h>
#include "libraries.h"
#include "cfg_wrapper.h"


Options *InitOptions()
{
    CfgWrapper cfg_wrapper;
    cfg_wrapper.BuildCfgWrapper(
        main_cfg_parameters.at("writer_id"));

    Options *opts = (Options *) malloc(sizeof(Options));

    const char *writer_id = cfg_wrapper.get_writer_id().c_str();
    opts->writer_id = strndup(writer_id, strlen(writer_id));

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

