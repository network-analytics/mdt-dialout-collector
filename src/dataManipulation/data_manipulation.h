// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _DATA_MANIPULATION_H_
#define _DATA_MANIPULATION_H_

// C++ Standard Library headers
#include <unordered_map>
#include <ctime>
// External Library headers
#include <json/json.h>
// mdt-dialout-collector Library headers
#include "proto/Cisco/cisco_telemetry.pb.h"
#include "proto/Juniper/juniper_gnmi.pb.h"
#include "proto/Juniper/juniper_telemetry_header_extension.pb.h"
#include "proto/Huawei/huawei_telemetry.pb.h"
#include "proto/OpenConfig/openconfig_interfaces.pb.h"
#include <google/protobuf/util/json_util.h>
#include "../utils/logs_handler.h"
#include "../utils/cfg_handler.h"


class DataManipulation {
public:
    DataManipulation() {
        spdlog::get("multi-logger")->
            debug("constructor: DataManipulation()"); };
    ~DataManipulation() {
        spdlog::get("multi-logger")->
            debug("destructor: ~DataManipulation()"); };
    bool MetaData(
        std::string &json_str,
        const std::string &peer_ip,
        const std::string &peer_port,
        std::string &json_str_out);
    void set_sequence_number() { sequence_number++; };
    uint64_t get_sequence_number() { return sequence_number; };
    bool AppendLabelMap(
        std::unordered_map<std::string,std::vector<std::string>> &label_map,
        const std::string &peer_ip,
        const std::string &json_str,
        std::string &json_str_out);
    bool CiscoGpbkv2Json(
        const cisco_telemetry::Telemetry &cisco_tlm,
        std::string &json_str_out);
    Json::Value CiscoGpbkvField2Json(
        const cisco_telemetry::TelemetryField &field);
    bool JuniperExtension(gnmi::SubscribeResponse &juniper_stream,
        GnmiJuniperTelemetryHeaderExtension &juniper_tlm_header_ext,
        Json::Value &root);
    bool JuniperUpdate(gnmi::SubscribeResponse &juniper_stream,
        std::string &json_str_out,
        Json::Value &root);
    bool HuaweiGpbOpenconfigInterface(
        const huawei_telemetry::Telemetry &huawei_tlm,
        openconfig_interfaces::Interfaces &oc_if,
        std::string &json_str_out);
private:
    uint64_t sequence_number = 0;
};

#endif

