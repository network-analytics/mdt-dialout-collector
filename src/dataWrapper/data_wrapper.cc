// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "data_wrapper.h"


bool DataWrapper::BuildDataWrapper(
    const std::string &event_type,
    const std::string &serialization,
    const std::string &writer_id,
    const std::string &telemetry_node,
    const std::string &telemetry_port,
    const std::string &telemetry_data)
{
    set_sequence_number();
    set_event_type(event_type);
    set_serialization(serialization);
    set_timestamp();
    set_writer_id(writer_id);
    set_telemetry_node(telemetry_node);
    set_telemetry_port(telemetry_port);
    set_telemetry_data(telemetry_data);

    return true;
}

void DataWrapper::DisplayDataWrapper()
{
    uint64_t sequence_number = get_sequence_number();
    const std::string event_type = get_event_type();
    const std::string serialization = get_serialization();
    std::time_t timestamp = get_timestamp();
    const std::string writer_id = get_writer_id();
    const std::string telemetry_node = get_telemetry_node();
    uint16_t telemetry_port = static_cast<uint16_t>(
        std::stoi(get_telemetry_port()));
    const std::string telemetry_data = get_telemetry_data();

    std::cout << sequence_number << "\n";
    std::cout << event_type << "\n";
    std::cout << serialization << "\n";
    std::cout << timestamp << "\n";
    std::cout << writer_id << "\n";
    std::cout << telemetry_node << "\n";
    std::cout << telemetry_port << "\n";
    std::cout << telemetry_data << "\n";
}

