// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT


// mdt-dialout-collector Library headers
#include "dataManipulation/data_manipulation.h"


// forge JSON & enrich with MAP (node_id/platform_id)
bool DataManipulation::AppendLabelMap(
    std::unordered_map<std::string,std::vector<std::string>> &label_map,
    const std::string &peer_ip,
    const std::string &json_str,
    std::string &json_str_out)
{
    const auto json_str_length = static_cast<int>(json_str.length());
    JSONCPP_STRING err;
    Json::Value root;
    Json::CharReaderBuilder builderR;
    Json::StreamWriterBuilder builderW;
    builderW["indentation"] = "";
    const std::unique_ptr<Json::CharReader> reader(builderR.newCharReader());
    const std::unique_ptr<Json::StreamWriter> writer(
        builderW.newStreamWriter());
    Json::Value jlabel_map;
    // select exclusively the IP addr from peer
    unsigned start_delim = (peer_ip.find_first_of(":") + 1);
    unsigned stop_delim = peer_ip.find_last_of(":");
    std::string _peer_ip = peer_ip.substr(
        start_delim, (stop_delim - start_delim));
    const auto search = label_map.find(_peer_ip);
    if (search != label_map.end()) {
        jlabel_map["node_id"] = search->second.at(0);
        jlabel_map["platform_id"] = search->second.at(1);
    } else{
        jlabel_map["node_id"] = "unknown";
        jlabel_map["platform_id"] = "unknown";
        std::cout << peer_ip << " not found: enrichment failed.\n";
    }

    if (!reader->parse(json_str.c_str(), json_str.c_str() + json_str_length,
        &root, &err) && json_str_length != 0) {
        std::cout << "ERROR parsing the string, conversion to JSON Failed!"
            << err
            << std::endl;
        //std::cout << "Failing message: " << json_str << std::endl;
        return false;
    }

    root["label"] = jlabel_map;
    json_str_out = Json::writeString(builderW, root);

    return true;
}

bool DataManipulation::CiscoGpbkv2Json(
    const std::unique_ptr<cisco_telemetry::Telemetry> &cisco_tlm,
    std::string &json_str_out)
{
     Json::Value root;
    // First read the metadata defined in cisco_telemtry.proto
    if (cisco_tlm->has_node_id_str()) {
        root["node_id"] = cisco_tlm->node_id_str();
    }
    if (cisco_tlm->has_subscription_id_str()) {
        root["subscription_id"] = cisco_tlm->subscription_id_str();
    }
    root["encoding_path"] = cisco_tlm->encoding_path();
    root["collection_id"] = (Json::UInt64) cisco_tlm->collection_id();
    root["collection_start_time"] =
        (Json::UInt64) cisco_tlm->collection_start_time();
    root["msg_timestamp"] =
        (Json::UInt64) cisco_tlm->msg_timestamp();
    root["collection_end_time"] =
        (Json::UInt64) cisco_tlm->collection_end_time();

    // Iterate through the key/values in data_gpbkv
    Json::Value gpbkv;
    for (auto const &field: cisco_tlm->data_gpbkv()) {
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
    Json::StreamWriterBuilder builderW;
    builderW["indentation"] = "";
    const std::unique_ptr<Json::StreamWriter> writer(
        builderW.newStreamWriter());
    json_str_out = Json::writeString(builderW, root);

    return true;
}

Json::Value DataManipulation::CiscoGpbkvField2Json(
    const cisco_telemetry::TelemetryField &field)
{
    Json::Value root;
    // gpbkv allows for nested kv fields, we recursively decode each one of them.
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

bool DataManipulation::JuniperExtension(
    gnmi::SubscribeResponse &juniper_stream,
    const std::unique_ptr<GnmiJuniperTelemetryHeaderExtension>
    &juniper_tlm_header_ext,
    Json::Value &root)
{
    bool parsing_str;
    std::string stream_data_in;

    for (const auto &ext : juniper_stream.extension()) {
        if (ext.has_registered_ext() &&
            ext.registered_ext().id() ==
                gnmi_ext::ExtensionID::EID_JUNIPER_TELEMETRY_HEADER) {
            parsing_str = juniper_tlm_header_ext->ParseFromString(
                ext.registered_ext().msg());

            if (parsing_str == true) {
                if (!juniper_tlm_header_ext->system_id().empty()) {
                    root["system_id"] = juniper_tlm_header_ext->system_id();
                    //std::cout << juniper_tlm_header_ext->system_id() << "\n";
                }

                stream_data_in.clear();
                google::protobuf::util::JsonPrintOptions opt;
                opt.add_whitespace = true;
                google::protobuf::util::MessageToJsonString(
                    *juniper_tlm_header_ext,
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

bool DataManipulation::JuniperUpdate(gnmi::SubscribeResponse &juniper_stream,
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

    //std::string value;
    std::string sensor_path;
    //std::cout << "-------> " << jup.ByteSizeLong() << "\n\n";
    if (juniper_stream.has_update()) {
        const auto &jup = juniper_stream.update();
        // The Notification MUST include the timestamp field
        std::uint64_t notification_timestamp = jup.timestamp();
        //std::cout << "DebugString: " << jup.prefix().Utf8DebugString()
        //    << "\n";
        int path_idx = 0;
        sensor_path.clear();
        while (path_idx < jup.prefix().elem_size()) {
            // first partial path with filters
            if (path_idx == 0 &&
                jup.prefix().elem().at(path_idx).key_size() > 0) {
                //std::cout << "/" << jup.prefix().elem().at(path_idx).name();
                sensor_path.append("/");
                sensor_path.append(jup.prefix().elem().at(path_idx).name());
                int filter = 1;
                for (const auto &[key, value] :
                    jup.prefix().elem().at(path_idx).key()) {
                    // only one filter
                    if (jup.prefix().elem().at(path_idx).key_size() == 1) {
                        //std::cout << "[" << key << "=" << value << "]";
                        sensor_path.append("[");
                        sensor_path.append(key);
                        sensor_path.append("=");
                        sensor_path.append(value);
                        sensor_path.append("]");
                        path_idx++;
                        continue;
                    }
                    // multiple filters
                    if (jup.prefix().elem().at(path_idx).key_size() > 1) {
                        // first filter
                        if (filter == 1) {
                            //std::cout << "[" << key << "=" << value
                            //    << " and ";
                            sensor_path.append("[");
                            sensor_path.append(key);
                            sensor_path.append("=");
                            sensor_path.append(value);
                            sensor_path.append(" and ");
                            filter++;
                            continue;
                        }
                        // last filter
                        if (filter ==
                            jup.prefix().elem().at(path_idx).key_size()) {
                            //std::cout << key << "=" << value << "]";
                            sensor_path.append(key);
                            sensor_path.append("=");
                            sensor_path.append(value);
                            sensor_path.append("]");
                            filter++;
                            continue;
                        }
                        // in-between filters
                        if (filter > 0) {
                            //std::cout << key << "=" << value << " and ";
                            sensor_path.append(key);
                            sensor_path.append("=");
                            sensor_path.append(value);
                            sensor_path.append(" and ");
                            filter++;
                            continue;
                        }
                    }
                }
                //std::cout << "/";
                sensor_path.append("/");
                path_idx++;
                continue;
            }
            // first partial path without filters
            if (path_idx == 0) {
                //std::cout << "/" << jup.prefix().elem().at(path_idx).name()
                //    << "/";
                sensor_path.append("/");
                sensor_path.append(jup.prefix().elem().at(path_idx).name());
                sensor_path.append("/");
                path_idx++;
                continue;
            }
            // in-between paths with filters
            if (jup.prefix().elem().at(path_idx).key_size() > 0) {
                //std::cout << jup.prefix().elem().at(path_idx).name();
                sensor_path.append(jup.prefix().elem().at(path_idx).name());
                int filter = 1;
                for (const auto &[key, value] :
                    jup.prefix().elem().at(path_idx).key()) {
                        // only one filter
                    if (jup.prefix().elem().at(path_idx).key_size() == 1) {
                        //std::cout << "[" << key << "=" << value << "]";
                        sensor_path.append("[");
                        sensor_path.append(key);
                        sensor_path.append("=");
                        sensor_path.append(value);
                        sensor_path.append("]");
                        path_idx++;
                        continue;
                    }
                    // multiple filters
                    if (jup.prefix().elem().at(path_idx).key_size() > 1) {
                        // first filter
                        if (filter == 1) {
                            //std::cout << "[" << key << "=" << value
                            //    << " and ";
                            sensor_path.append("[");
                            sensor_path.append(key);
                            sensor_path.append("=");
                            sensor_path.append(value);
                            sensor_path.append(" and ");
                            filter++;
                            continue;
                        }
                        // last filter
                        if (filter ==
                            jup.prefix().elem().at(path_idx).key_size()) {
                            //std::cout << key << "=" << value << "]";
                            sensor_path.append(key);
                            sensor_path.append("=");
                            sensor_path.append(value);
                            sensor_path.append("]");
                            filter++;
                            continue;
                        }
                        // in-between filters
                        if (filter > 0) {
                            //std::cout << key << "=" << value << " and ";
                            sensor_path.append(key);
                            sensor_path.append("=");
                            sensor_path.append(value);
                            sensor_path.append(" and ");
                            filter++;
                            continue;
                        }
                    }
                }
                //std::cout << "/";
                sensor_path.append("/");
                path_idx++;
                continue;
            }

            // no filtering
            //std::cout << jup.prefix().elem().at(path_idx).name() << "/";
            sensor_path.append(jup.prefix().elem().at(path_idx).name());
            sensor_path.append("/");
            path_idx++;
        }

        root["sensor_path"] = sensor_path;
        root["notification_timestamp"] = notification_timestamp;
        //std::cout << "sensor_path: " << sensor_path << "\n";

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
            root[path] = value.toStyledString();
        }
    }

    // Serialize the JSON value into a string
    Json::StreamWriterBuilder builderW;
    builderW["emitUTF8"] = false;
    builderW["indentation"] = "";
    const std::unique_ptr<Json::StreamWriter> writer(
        builderW.newStreamWriter());
    json_str_out = Json::writeString(builderW, root);

    return true;
}

bool DataManipulation::HuaweiGpbOpenconfigInterface(
    const std::unique_ptr<huawei_telemetry::Telemetry> &huawei_tlm,
    const std::unique_ptr<openconfig_interfaces::Interfaces> &oc_if,
    std::string &json_str_out)
{
    int data_rows = huawei_tlm->data_gpb().row_size();

    bool parsing_content {false};
    std::string content_s;
    Json::Value root;

    root["collection_id"] = huawei_tlm->collection_id();
    root["collection_start_time"] = huawei_tlm->collection_start_time();
    root["collection_end_time"] = huawei_tlm->collection_end_time();
    root["current_period"] = huawei_tlm->current_period();
    root["encoding"] = huawei_tlm->encoding();
    root["except_desc"] = huawei_tlm->except_desc();
    root["msg_timestamp"] = huawei_tlm->msg_timestamp();
    root["node_id_str"] = huawei_tlm->node_id_str();
    root["product_name"] = huawei_tlm->product_name();
    root["proto_path"] = huawei_tlm->proto_path();
    root["sensor_path"] = huawei_tlm->sensor_path();
    root["software_version"] = huawei_tlm->software_version();
    root["subscription_id_str"] = huawei_tlm->subscription_id_str();

    for (int idx_0 = 0; idx_0 < data_rows; ++idx_0) {
        content_s.clear();
        std::string content = huawei_tlm->
            data_gpb().row().at(idx_0).content();
        parsing_content = oc_if->ParseFromString(content);
        if (parsing_content == true) {
            google::protobuf::util::JsonPrintOptions opt;
            opt.add_whitespace = true;
            google::protobuf::util::MessageToJsonString(
                *oc_if,
                &content_s,
                opt);
        } else {
            return false;
        }

        root["decoded"].append(content_s);

        // Serialize the JSON value into a string
        Json::StreamWriterBuilder builderW;
        builderW["emitUTF8"] = false;
        builderW["indentation"] = "";
        const std::unique_ptr<Json::StreamWriter> writer(
        builderW.newStreamWriter());
        json_str_out = Json::writeString(builderW, root);
    }

    return true;
}

