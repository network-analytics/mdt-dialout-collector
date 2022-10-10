// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _DATA_WRAPPER_H_
#define _DATA_WRAPPER_H_

// C++ Standard Library headers
#include <ctime>
// External Library headers

// mdt-dialout-collector Library headers
#include "logs_handler.h"


class DataWrapper {
public:
    DataWrapper() {
        spdlog::get("multi-logger")->
            debug("constructor: DataWrapper()"); };
    ~DataWrapper() {
        spdlog::get("multi-logger")->
            debug("destructor: ~DataWrapper()"); };

    bool BuildDataWrapper(
        uint64_t sequence_number,
        const std::string event_type,
        const std::string serialization,
        std::time_t timestamp,
        const std::string writer_id,
        const std::string telemetry_node,
        uint16_t telemetry_port,
        const std::string set_telemetry_data
    );

    void DisplayDataWrapper();

    // Setters
    void set_sequence_number() {
        this->sequence_number++;
    };
    void set_event_type(const std::string &event_type) {
        this->event_type = event_type;
    };
    void set_serialization(const std::string &serialization) {
        this->serialization = serialization;
    };
    void set_timestamp() {
        this->timestamp = std::time_t(nullptr);
    };
    void set_writer_id(const std::string &writer_id) {
        this->writer_id = writer_id;
    };
    void set_telemetry_node(const std::string &telemetry_node) {
        this->telemetry_node = telemetry_node;
    };
    void set_telemetry_port(uint16_t &telemetry_port) {
        this->telemetry_port = telemetry_port;
    };
    void set_telemetry_data(const std::string &telemetry_data) {
        this->telemetry_data = telemetry_data;
    }

    // Getters
    uint64_t get_sequence_number() { return this->sequence_number; };
    std::string get_event_type() { return this->event_type; };
    std::string get_serialization() { return this->serialization; };
    std::time_t get_timestamp() { return this->timestamp; };
    std::string get_writer_id() { return this->writer_id; };
    std::string get_telemetry_node() { return this->telemetry_node; };
    uint16_t get_telemetry_port() { return this->telemetry_port; };
    std::string get_telemetry_data() { return this->telemetry_data; };
private:
    uint64_t sequence_number = 0;
    std::string event_type;
    std::string serialization;
    std::time_t timestamp;
    std::string writer_id;
    std::string telemetry_node;
    uint16_t telemetry_port;
    std::string telemetry_data;
};

#endif

