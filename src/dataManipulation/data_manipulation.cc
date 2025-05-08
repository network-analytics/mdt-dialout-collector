// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "data_manipulation.h"


// Envelop & collector meta-data
bool DataManipulation::MetaData(std::string &json_str,
    const std::string &peer_ip,
    const std::string &peer_port,
    std::string &json_str_out)
{
    std::time_t timestamp = std::time(nullptr);
    const auto json_str_length = json_str.length();
    //Json::String error;
    JSONCPP_STRING error;
    Json::Value root;
    Json::CharReaderBuilder builder_r;
    Json::StreamWriterBuilder builder_w;
    builder_w["emitUTF8"] = true;
    builder_w["indentation"] = "";

    const std::unique_ptr<Json::CharReader> reader(builder_r.newCharReader());
    const std::unique_ptr<Json::StreamWriter> writer(
        builder_w.newStreamWriter());


    if (!reader->parse(json_str.c_str(), json_str.c_str() + json_str_length,
        &root, &error) && json_str_length != 0) {
        spdlog::get("multi-logger")->
            error("[MetaData] data-manipulation issue: "
            "conversion to JSON failure, {}", error);
        return false;
    } else {
        root.clear();
        set_sequence_number();
        root["event_type"] = "gRPC";
        root["serialization"] = "json_string";
        root["seq"] = static_cast<uint64_t>(get_sequence_number());
        root["timestamp"] = timestamp;
        root["writer_id"] = main_cfg_parameters.at("writer_id");
        root["telemetry_node"] = peer_ip;
        root["telemetry_port"] = static_cast<uint16_t>(std::stoi(peer_port));
        root["telemetry_data"] = json_str;
        spdlog::get("multi-logger")->info("[MetaData] data-manipulation: "
            "{} meta-data added successfully", peer_ip);
    }

    json_str_out = Json::writeString(builder_w, root);

    return true;
}

// forge JSON & enrich with MAP (node_id/platform_id)
bool DataManipulation::AppendLabelMap(
    std::unordered_map<std::string,std::vector<std::string>> &label_map,
    const std::string &peer_ip,
    const std::string &json_str,
    std::string &json_str_out)
{
    const auto json_str_length = json_str.length();
    //Json::String error;
    JSONCPP_STRING error;
    Json::Value root;
    Json::Value jlabel_map;
    Json::CharReaderBuilder builder_r;
    Json::StreamWriterBuilder builder_w;
    builder_w["emitUTF8"] = true;
    builder_w["indentation"] = "";

    const std::unique_ptr<Json::CharReader> reader(builder_r.newCharReader());
    const std::unique_ptr<Json::StreamWriter> writer(
        builder_w.newStreamWriter());

    const auto search = label_map.find(peer_ip);

    if (!reader->parse(json_str.c_str(), json_str.c_str() + json_str_length,
        &root, &error) && json_str_length != 0) {
        spdlog::get("multi-logger")->
            error("[AppendLabelMap] data-manipulation issue: "
            "conversion to JSON failure, {}", error);
        return false;
    } else {
        if (search != label_map.end()) {
            jlabel_map["nkey"] = search->second.at(0);
            jlabel_map["pkey"] = search->second.at(1);
            spdlog::get("multi-logger")->
                info("[AppendLabelMap] data-manipulation: "
                "{} data enrichment successful", peer_ip);
        } else {
            jlabel_map["node_id"] = "unknown";
            jlabel_map["platform_id"] = "unknown";
            spdlog::get("multi-logger")->
                warn("[AppendLabelMap] data-manipulation issue: "
                "{} not found, data enrichment failure", peer_ip);
        }

        root["label"] = jlabel_map;
        json_str_out = Json::writeString(builder_w, root);
    }

    return true;
}

// GPB-KV Normalization & generate JSON-Str
bool DataManipulation::CiscoGpbkv2Json(
    const cisco_telemetry::Telemetry &cisco_tlm,
    std::string &json_str_out)
{
     Json::Value root;
    // First read the metadata defined in cisco_telemtry.proto
    if (cisco_tlm.has_node_id_str()) {
        root["node_id"] = cisco_tlm.node_id_str();
    }
    if (cisco_tlm.has_subscription_id_str()) {
        root["subscription_id"] = cisco_tlm.subscription_id_str();
    }
    root["encoding_path"] = cisco_tlm.encoding_path();
    root["collection_id"] = (Json::UInt64) cisco_tlm.collection_id();
    root["collection_start_time"] =
        (Json::UInt64) cisco_tlm.collection_start_time();
    root["msg_timestamp"] =
        (Json::UInt64) cisco_tlm.msg_timestamp();
    root["collection_end_time"] =
        (Json::UInt64) cisco_tlm.collection_end_time();

    // Iterate through the key/values in data_gpbkv
    Json::Value gpbkv;
    for (auto const &field: cisco_tlm.data_gpbkv()) {
        Json::Value value = DataManipulation::CiscoGpbkvField2Json(field);
        if (field.name().empty()) {
            gpbkv.append(value);
        } else {
            Json::Value json_field;
            json_field[field.name()] = value;
            gpbkv.append(json_field);
        }
    }
    root["data_gpbkv"] = gpbkv;

    // Serialize the JSON value into a string
    Json::StreamWriterBuilder builder_w;
    builder_w["emitUTF8"] = true;
    builder_w["indentation"] = "";
    const std::unique_ptr<Json::StreamWriter> writer(
        builder_w.newStreamWriter());
    json_str_out = Json::writeString(builder_w, root);

    return true;
}

Json::Value DataManipulation::CiscoGpbkvField2Json(
    const cisco_telemetry::TelemetryField &field)
{
    Json::Value root;
    // gpbkv allows for nested kv fields, we recursively decode each one of
    // them.
    Json::Value sub_fields;
    for (const cisco_telemetry::TelemetryField& sub_field: field.fields()) {
        Json::Value sub_field_value =
            DataManipulation::CiscoGpbkvField2Json(sub_field);
        Json::Value sub_field_json;
        if (sub_field.name().size() == 0) {
            sub_fields.append(sub_field_value);
        } else {
            sub_field_json[sub_field.name()] = sub_field_value;
            sub_fields.append(sub_field_json);
        }
    }

    // the value of the field can be one of several predefined types, or null.
    Json::Value value;
    if (field.has_bytes_value()) {
        value = field.bytes_value();
    } else if (field.has_string_value()) {
        value = field.string_value();
    } else if (field.has_bool_value()) {
        value = field.bool_value();
    } else if (field.has_uint32_value()) {
        value = field.uint32_value();
    } else if (field.has_uint64_value()) {
        value = (Json::UInt64) field.uint64_value();
    } else if (field.has_sint32_value()) {
        value = field.sint32_value();
    } else if (field.has_sint64_value()) {
        value = (Json::Int64) field.sint64_value();
    } else if (field.has_double_value()) {
        value = field.double_value();
    }  else if (field.has_float_value()) {
        value = field.float_value();
    }

    // timestamp is required field for each field, routers send zero for basic
    // fields (e.g., int64)
    if (field.timestamp() != 0) {
        root["timestamp"] = (Json::UInt64) field.timestamp();
    }

    // Clean up the output to send only the properties that have values
    if (!sub_fields.empty()) {
        root["fields"] = sub_fields;
    }
    if (!root.empty()) {
        if (!value.empty()) {
            root["value"] = value;
        }
        return root;
    }
    return value;
}

// 1. Decode & Extract mata-data & Add to JSON-Obj (juniper_tlm_header_ext)
// 2. From JSON-Obj to JSON-Str
bool DataManipulation::JuniperExtension(
    juniper_gnmi::SubscribeResponse &juniper_stream,
    GnmiJuniperTelemetryHeaderExtension &juniper_tlm_header_ext,
    Json::Value &root)
{
    bool parsing_str {false};
    std::string stream_data_in;

    for (const auto &ext : juniper_stream.extension()) {
        if (ext.has_registered_ext() &&
            ext.registered_ext().id() ==
                juniper_gnmi_ext::ExtensionID::EID_JUNIPER_TELEMETRY_HEADER) {
            parsing_str = juniper_tlm_header_ext.ParseFromString(
                ext.registered_ext().msg());

            if (parsing_str == true) {
                if (!juniper_tlm_header_ext.system_id().empty()) {
                    root["system_id"] = juniper_tlm_header_ext.system_id();
                    //std::cout << juniper_tlm_header_ext->system_id() << "\n";
                }

                stream_data_in.clear();
                google::protobuf::util::JsonPrintOptions opt;
                opt.add_whitespace = false;
                google::protobuf::util::MessageToJsonString(
                    juniper_tlm_header_ext,
                    &stream_data_in,
                    opt);
                root["extension"] = stream_data_in;
                //std::cout << stream_data_in << "\n";
            } else {
                return false;
            }
        }
    }

    return true;
}

// Generate the JSON-Str from the upadate msg
bool DataManipulation::JuniperUpdate(juniper_gnmi::SubscribeResponse &juniper_stream,
    std::string &json_str_out,
    Json::Value &root)
{
    // From the first update() generate the sensor_path
    //SubscribeResponse
    //---> bool sync_response = 3;
    //---> Notification update = 1;
    //     ---> (        ) bool  atomic = 6;
    //     ---> (        ) int64 timestamp = 1
    //     ---> (        ) Path  prefix = 2;
    //          ---> (        ) string origin = 2;
    //          ---> (        ) string target = 4;
    //          ---> (repeated) PathElem elem = 3;
    //                          ---> string name = 1;
    //                          ---> map<string, string> key = 2;

    // sensor_path as JSON
    if (juniper_stream.has_update()) {
        const auto &jup = juniper_stream.update();
        std::uint64_t notification_timestamp = jup.timestamp();

        int path_idx = 0;
        Json::Value sensor_path(Json::arrayValue);

        while (path_idx < jup.prefix().elem_size()) {
            Json::Value path_element;
            path_element["name"] = jup.prefix().elem().at(path_idx).name();

            // handling paths with filters
            if (jup.prefix().elem().at(path_idx).key_size() > 0) {
                Json::Value filters;
                for (const auto &[key, value] :
                jup.prefix().elem().at(path_idx).key()) {
                    filters[key] = value;
                }
                path_element["filters"] = filters;
            }
            sensor_path.append(path_element);
            path_idx++;
        }

        root["sensor_path"] = sensor_path;
        root["notification_timestamp"] = notification_timestamp;

    // sensor_path as string
    //std::string value;
    //std::string sensor_path;

    //std::cout << "-------> " << jup.ByteSizeLong() << "\n\n";
    //if (juniper_stream.has_update()) {
    //    const auto &jup = juniper_stream.update();
    //    // The Notification MUST include the timestamp field
    //    std::uint64_t notification_timestamp = jup.timestamp();
    //    //std::cout << "DebugString: " << jup.prefix().Utf8DebugString()
    //    //    << "\n";
    //    int path_idx = 0;
    //    sensor_path.clear();
    //    while (path_idx < jup.prefix().elem_size()) {
    //        // first partial path with filters
    //        if (path_idx == 0 &&
    //            jup.prefix().elem().at(path_idx).key_size() > 0) {
    //            //std::cout << "/" << jup.prefix().elem().at(path_idx).name();
    //            sensor_path.append("/");
    //            sensor_path.append(jup.prefix().elem().at(path_idx).name());
    //            int filter = 1;
    //            for (const auto &[key, value] :
    //                jup.prefix().elem().at(path_idx).key()) {
    //                // only one filter
    //                if (jup.prefix().elem().at(path_idx).key_size() == 1) {
    //                    //std::cout << "[" << key << "=" << value << "]";
    //                    sensor_path.append("[");
    //                    sensor_path.append(key);
    //                    sensor_path.append("=");
    //                    sensor_path.append(value);
    //                    sensor_path.append("]");
    //                    path_idx++;
    //                    continue;
    //                }
    //                // multiple filters
    //                if (jup.prefix().elem().at(path_idx).key_size() > 1) {
    //                    // first filter
    //                    if (filter == 1) {
    //                        //std::cout << "[" << key << "=" << value
    //                        //    << " and ";
    //                        sensor_path.append("[");
    //                        sensor_path.append(key);
    //                        sensor_path.append("=");
    //                        sensor_path.append(value);
    //                        sensor_path.append(" and ");
    //                        filter++;
    //                        continue;
    //                    }
    //                    // last filter
    //                    if (filter ==
    //                        jup.prefix().elem().at(path_idx).key_size()) {
    //                        //std::cout << key << "=" << value << "]";
    //                        sensor_path.append(key);
    //                        sensor_path.append("=");
    //                        sensor_path.append(value);
    //                        sensor_path.append("]");
    //                        filter++;
    //                        continue;
    //                    }
    //                    // in-between filters
    //                    if (filter > 0) {
    //                        //std::cout << key << "=" << value << " and ";
    //                        sensor_path.append(key);
    //                        sensor_path.append("=");
    //                        sensor_path.append(value);
    //                        sensor_path.append(" and ");
    //                        filter++;
    //                        continue;
    //                    }
    //                }
    //            }
    //            //std::cout << "/";
    //            sensor_path.append("/");
    //            path_idx++;
    //            continue;
    //        }
    //        // first partial path without filters
    //        if (path_idx == 0) {
    //            //std::cout << "/" << jup.prefix().elem().at(path_idx).name()
    //            //    << "/";
    //            sensor_path.append("/");
    //            sensor_path.append(jup.prefix().elem().at(path_idx).name());
    //            sensor_path.append("/");
    //            path_idx++;
    //            continue;
    //        }
    //        // in-between paths with filters
    //        if (jup.prefix().elem().at(path_idx).key_size() > 0) {
    //            //std::cout << jup.prefix().elem().at(path_idx).name();
    //            sensor_path.append(jup.prefix().elem().at(path_idx).name());
    //            int filter = 1;
    //            for (const auto &[key, value] :
    //                jup.prefix().elem().at(path_idx).key()) {
    //                    // only one filter
    //                if (jup.prefix().elem().at(path_idx).key_size() == 1) {
    //                    //std::cout << "[" << key << "=" << value << "]";
    //                    sensor_path.append("[");
    //                    sensor_path.append(key);
    //                    sensor_path.append("=");
    //                    sensor_path.append(value);
    //                    sensor_path.append("]");
    //                    path_idx++;
    //                    continue;
    //                }
    //                // multiple filters
    //                if (jup.prefix().elem().at(path_idx).key_size() > 1) {
    //                    // first filter
    //                    if (filter == 1) {
    //                        //std::cout << "[" << key << "=" << value
    //                        //    << " and ";
    //                        sensor_path.append("[");
    //                        sensor_path.append(key);
    //                        sensor_path.append("=");
    //                        sensor_path.append(value);
    //                        sensor_path.append(" and ");
    //                        filter++;
    //                        continue;
    //                    }
    //                    // last filter
    //                    if (filter ==
    //                        jup.prefix().elem().at(path_idx).key_size()) {
    //                        //std::cout << key << "=" << value << "]";
    //                        sensor_path.append(key);
    //                        sensor_path.append("=");
    //                        sensor_path.append(value);
    //                        sensor_path.append("]");
    //                        filter++;
    //                        continue;
    //                    }
    //                    // in-between filters
    //                    if (filter > 0) {
    //                        //std::cout << key << "=" << value << " and ";
    //                        sensor_path.append(key);
    //                        sensor_path.append("=");
    //                        sensor_path.append(value);
    //                        sensor_path.append(" and ");
    //                        filter++;
    //                        continue;
    //                    }
    //                }
    //            }
    //            //std::cout << "/";
    //            sensor_path.append("/");
    //            path_idx++;
    //            continue;
    //        }

    //        // no filtering
    //        //std::cout << jup.prefix().elem().at(path_idx).name() << "/";
    //        sensor_path.append(jup.prefix().elem().at(path_idx).name());
    //        sensor_path.append("/");
    //        path_idx++;
    //    }

    //    root["sensor_path"] = sensor_path;
    //    root["notification_timestamp"] = notification_timestamp;
    //    std::cout << "sensor_path: " << sensor_path << "\n";

        // From the second update().update() extract all the values
        // associated with a specific sensor path
        //SubscribeResponse
        //---> bool sync_response = 3;
        //---> Notification update = 1;
        //     ---> (repeated) Update update = 4;
        //          ---> TypedValue val = 3;
        //          ---> Path path = 1;
        //          ---> (        ) string origin = 2;
        //          ---> (        ) string target = 4;
        //          ---> (repeated) PathElem elem = 3;
        //                          ---> string name = 1;
        //                          ---> map<string, string> key = 2;
        //std::cout << "\n";
        std::string path;
        Json::Value value;
        for (const auto &_jup : jup.update()) {
            //std::cout << "DebugString: " << _jup.path().Utf8DebugString()
            //    << "\n";
            int path_idx = 0;
            path.clear();
            while (path_idx < _jup.path().elem_size()) {
                //std::cout << _jup.path().elem().at(path_idx).name()
                //    << " ---> ";
                path.append("/");
                path.append(_jup.path().elem().at(path_idx).name());
                path_idx++;
            }

            // only json_val() received
            value = _jup.val().json_val();
            //std::cout << value << "\n";
            root[path] = value;
        }
    }

    // Serialize the JSON value into a string
    Json::StreamWriterBuilder builder_w;
    builder_w["emitUTF8"] = true;
    builder_w["indentation"] = "";
    const std::unique_ptr<Json::StreamWriter> writer(
        builder_w.newStreamWriter());
    json_str_out = Json::writeString(builder_w, root);

    return true;
}


// Generate the JSON-Str from the upadate msg
bool DataManipulation::NokiaUpdate(nokia_gnmi::SubscribeResponse &nokia_stream,
    std::string &json_str_out,
    Json::Value &root)
{
    // From the first update() generate the sensor_path
    //SubscribeResponse
    //---> bool sync_response = 3;
    //---> Notification update = 1;
    //     ---> (        ) bool  atomic = 6;
    //     ---> (        ) int64 timestamp = 1
    //     ---> (        ) Path  prefix = 2;
    //          ---> (        ) string origin = 2;
    //          ---> (        ) string target = 4;
    //          ---> (repeated) PathElem elem = 3;
    //                          ---> string name = 1;
    //                          ---> map<string, string> key = 2;

    // sensor_path as JSON
    if (nokia_stream.has_update()) {
        const auto &nup = nokia_stream.update();
        std::uint64_t notification_timestamp = nup.timestamp();

        // Log the full contents of the nup object using DebugString() if it's a protobuf object.
        spdlog::get("multi-logger")->info("[NokiaDebug] Full nup object: {}", nup.DebugString());
        if (nokia_stream.extension_size() > 0) {
            for (const auto& ext : nokia_stream.extension()) {
                spdlog::get("multi-logger")->info("[NokiaDebug] Nokia extension object: {}", ext.DebugString());
            }
        } else {
            spdlog::get("multi-logger")->info("[NokiaDebug] No extensions found in the SubscribeResponse.");
        }
        int path_idx = 0;
        Json::Value sensor_path(Json::arrayValue);
        spdlog::get("multi-logger")->info("[NokiaDebug] Full nup.prefix() object: {}", nup.prefix().DebugString());

        while (path_idx < nup.prefix().elem_size()) {
            Json::Value path_element;
            path_element["name"] = nup.prefix().elem().at(path_idx).name();

            // handling paths with filters
            if (nup.prefix().elem().at(path_idx).key_size() > 0) {
                Json::Value filters;
                for (const auto &[key, value] :
                nup.prefix().elem().at(path_idx).key()) {
                    filters[key] = value;
                }
                path_element["filters"] = filters;
            }
            sensor_path.append(path_element);
            path_idx++;
        }
        std::string collected_path;

        for (const auto& element : sensor_path) {
            // Access the "name" field from the JSON object
            if (element.isMember("name")) {
                collected_path += "/" + element["name"].asString();
            }
            // Check if "filters" exist and append them in square brackets. Uncomment to enable this logic.
            /*
            if (element.isMember("filters")) {
                const Json::Value& filters = element["filters"];
                if (!filters.empty()) {
                    collected_path += "[";
                    // Assuming we can have multiple filters for one field in a sensor_path, we can remove the loop if that's not an existing scenario
                    bool first = true;
                    for (const auto& key : filters.getMemberNames()) {
                        if (!first) {
                            collected_path += ",";
                        }
                        collected_path += key + "=" + filters[key].asString();
                        first = false;
                    }
                    collected_path += "]";
                }
            }*/
        }
        root["collected_path"] = collected_path;
        root["sensor_path"] = sensor_path;
        root["notification_timestamp"] = notification_timestamp;


        std::string path;
        Json::Value value;
        for (const auto &_nup : nup.update()) {
            //std::cout << "DebugString: " << _jup.path().Utf8DebugString()
            //    << "\n";
            int path_idx = 0;
            path.clear();
            spdlog::get("multi-logger")->info("[NokiaDebug] Full nup.path() object: {}", _nup.path().DebugString());
            while (path_idx < _nup.path().elem_size()) {
                //std::cout << _jup.path().elem().at(path_idx).name()
                //    << " ---> ";
                path.append("/");
                path.append(_nup.path().elem().at(path_idx).name());
                path_idx++;
            }

            value = _nup.val().json_ietf_val();
            //std::cout << value << "\n";
            root[path] = value;
        }
    }

    std::string raw_data;
    google::protobuf::util::JsonPrintOptions opt;
    opt.add_whitespace = false;
    auto status = google::protobuf::util::MessageToJsonString(nokia_stream, &raw_data, opt);
    if (!status.ok()) {
    spdlog::get("multi-logger")->error("[NokiaDebug] Failed to convert protobuf to JSON: {}", status.ToString());
    }
    // Log raw_data for debugging
    spdlog::get("multi-logger")->debug("[NokiaDebug] raw_data: {}", raw_data);
    // root["raw_data"] = raw_data;

    // Serialize the JSON value into a string
    Json::StreamWriterBuilder builder_w;
    builder_w["emitUTF8"] = true;
    builder_w["indentation"] = "";
    const std::unique_ptr<Json::StreamWriter> writer(
        builder_w.newStreamWriter());
    json_str_out = Json::writeString(builder_w, root);

    return true;
}

// 1. Decode & Extract mata-data & Add to JSON-Obj (huawei_tlm)
// 2. Decode & Extract payload (record = content_s) & Add to JSON-Obj (oc-if)
// 3. From JSON-Obj to JSON-Str
bool DataManipulation::HuaweiGpbOpenconfigInterface(
    const huawei_telemetry::Telemetry &huawei_tlm,
    openconfig_interfaces::Interfaces &oc_if,
    std::string &json_str_out)
{
    int data_rows = huawei_tlm.data_gpb().row_size();

    bool parsing_content {false};
    std::string content_s;
    Json::Value root;

    root["collection_id"] = huawei_tlm.collection_id();
    root["collection_start_time"] = huawei_tlm.collection_start_time();
    root["collection_end_time"] = huawei_tlm.collection_end_time();
    root["current_period"] = huawei_tlm.current_period();
    root["encoding"] = huawei_tlm.encoding();
    root["except_desc"] = huawei_tlm.except_desc();
    root["msg_timestamp"] = huawei_tlm.msg_timestamp();
    root["node_id_str"] = huawei_tlm.node_id_str();
    root["product_name"] = huawei_tlm.product_name();
    root["proto_path"] = huawei_tlm.proto_path();
    root["sensor_path"] = huawei_tlm.sensor_path();
    root["software_version"] = huawei_tlm.software_version();
    root["subscription_id_str"] = huawei_tlm.subscription_id_str();

    for (int idx_0 = 0; idx_0 < data_rows; ++idx_0) {
        content_s.clear();
        std::string content = huawei_tlm.
            data_gpb().row().at(idx_0).content();
        parsing_content = oc_if.ParseFromString(content);
        if (parsing_content == true) {
            google::protobuf::util::JsonPrintOptions opt;
            opt.add_whitespace = false;
            google::protobuf::util::MessageToJsonString(
                oc_if,
                &content_s,
                opt);
        } else {
            return false;
        }

        root["decoded"].append(content_s);

        // Serialize the JSON value into a string
        Json::StreamWriterBuilder builder_w;
        builder_w["emitUTF8"] = true;
        builder_w["indentation"] = "";
        const std::unique_ptr<Json::StreamWriter> writer(
        builder_w.newStreamWriter());
        json_str_out = Json::writeString(builder_w, root);
    }

    return true;
}

