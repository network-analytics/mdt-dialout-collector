// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _KAFKA_DELIVERY_H_
#define _KAFKA_DELIVERY_H_

// C++ Standard Library headers

// External Library headers
#include "kafka/KafkaProducer.h"
// mdt-dialout-collector Library headers
#include "../utils/cfg_handler.h"
#include "kafka/Properties.h"
#include "kafka/Types.h"
#include "../utils/logs_handler.h"


class KafkaDelivery {
public:
    KafkaDelivery();
    ~KafkaDelivery() { spdlog::get("multi-logger")->
        debug("destructor: ~KafkaDelivery()"); };
    bool AsyncKafkaProducer(
        kafka::clients::KafkaProducer &producer,
        const std::string &peer,
        const std::string &json_str);
    void set_kafka_properties(kafka::Properties &properties);
    kafka::Properties get_properties() {
        return properties; };
    kafka::Topic get_topic() {
        return topic; }
    std::string get_bootstrap_servers() {
        return bootstrap_servers; };
    std::string get_enable_idempotence() {
        return enable_idempotence; };
    std::string get_client_id() {
        return client_id; };
    std::string get_security_protocol() {
        return security_protocol; };
    std::string get_ssl_key_location() {
        return ssl_key_location; };
    std::string get_ssl_certificate_location() {
        return ssl_certificate_location; };
    std::string get_ssl_ca_location() {
        return ssl_ca_location; };
    std::string get_log_level() {
        return log_level; };
    const std::string get_enable_ssl_certificate_verification() {
        return enable_ssl_certificate_verification; };
private:
    kafka::Properties properties;
    kafka::Topic topic;
    std::string bootstrap_servers;
    std::string enable_idempotence;
    std::string client_id;
    std::string security_protocol;
    std::string ssl_key_location;
    std::string ssl_certificate_location;
    std::string ssl_ca_location;
    std::string log_level;
    const std::string enable_ssl_certificate_verification;
};

#endif

