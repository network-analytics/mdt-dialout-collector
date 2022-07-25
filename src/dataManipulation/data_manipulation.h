#ifndef _DATA_MANIPULATION_H_
#define _DATA_MANIPULATION_H_

#include <iostream>
#include <json/json.h>
#include "cisco_telemetry.pb.h"
#include "juniper_gnmi.pb.h"
#include "juniper_telemetry_header_extension.pb.h"
#include "huawei_telemetry.pb.h"
#include "openconfig_interfaces.pb.h"


class DataManipulation {
public:
    // Handling data manipulation functions
    bool append_label_map(
        std::unordered_map<std::string,std::vector<std::string>>& enrich_map,
        const std::string& peer_ip,
        const std::string& json_str,
        std::string& json_str_out);
    bool cisco_gpbkv2json(
        const std::unique_ptr<cisco_telemetry::Telemetry>& cisco_tlm,
        std::string& json_str_out);
    Json::Value cisco_gpbkv_field2json(
        const cisco_telemetry::TelemetryField& field);
    bool juniper_extension(gnmi::SubscribeResponse& juniper_stream,
        const std::unique_ptr<GnmiJuniperTelemetryHeaderExtension>&
        juniper_tlm_header_ext,
        Json::Value& root);
    bool juniper_update(gnmi::SubscribeResponse& juniper_stream,
        std::string& json_str_out,
        Json::Value& root);
    bool huawei_gpb_openconfig_interface(
        const std::unique_ptr<huawei_telemetry::Telemetry>& huawei_tlm,
        const std::unique_ptr<openconfig_interfaces::Interfaces>& oc_if,
        std::string& json_str_out);
};

#endif

