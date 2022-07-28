// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _DATA_MANIPULATION_H_
#define _DATA_MANIPULATION_H_

// C++ Standard Library headers
#include <unordered_map>
// External Library headers
#include <json/json.h>
// mdt-dialout-collector Library headers
#include "cisco_telemetry.pb.h"
#include "juniper_gnmi.pb.h"
#include "juniper_telemetry_header_extension.pb.h"
#include "huawei_telemetry.pb.h"
#include "openconfig_interfaces.pb.h"
#include <google/protobuf/util/json_util.h>


class DataManipulation {
public:
    bool AppendLabelMap(
        std::unordered_map<std::string,std::vector<std::string>> &label_map,
        const std::string &peer_ip,
        const std::string &json_str,
        std::string &json_str_out);
    bool CiscoGpbkv2Json(
        const std::unique_ptr<cisco_telemetry::Telemetry> &cisco_tlm,
        std::string &json_str_out);
    Json::Value CiscoGpbkvField2Json(
        const cisco_telemetry::TelemetryField &field);
    bool JuniperExtension(gnmi::SubscribeResponse &juniper_stream,
        const std::unique_ptr<GnmiJuniperTelemetryHeaderExtension>
        &juniper_tlm_header_ext,
        Json::Value &root);
    bool JuniperUpdate(gnmi::SubscribeResponse &juniper_stream,
        std::string &json_str_out,
        Json::Value &root);
    bool HuaweiGpbOpenconfigInterface(
        const std::unique_ptr<huawei_telemetry::Telemetry> &huawei_tlm,
        const std::unique_ptr<openconfig_interfaces::Interfaces> &oc_if,
        std::string &json_str_out);
};

#endif

