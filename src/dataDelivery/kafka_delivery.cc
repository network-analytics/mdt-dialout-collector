// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "kafka_delivery.h"


KafkaDelivery::KafkaDelivery()
{
    spdlog::get("multi-logger")->debug("constructor: KafkaDelivery()");
    this->topic =
        kafka_delivery_cfg_parameters.at("topic");
    this->bootstrap_servers =
        kafka_delivery_cfg_parameters.at("bootstrap_servers");
    this->enable_idempotence =
        kafka_delivery_cfg_parameters.at("enable_idempotence");
    this->client_id =
        kafka_delivery_cfg_parameters.at("client_id");
    this->security_protocol =
        kafka_delivery_cfg_parameters.at("security_protocol");
    this->ssl_key_location =
        kafka_delivery_cfg_parameters.at("ssl_key_location");
    this->ssl_key_password =
        kafka_delivery_cfg_parameters.at("ssl_key_password");
    this->ssl_certificate_location =
        kafka_delivery_cfg_parameters.at("ssl_certificate_location");
    this->ssl_ca_location =
        kafka_delivery_cfg_parameters.at("ssl_ca_location");
    this->enable_ssl_certificate_verification =
        kafka_delivery_cfg_parameters.at("enable_ssl_certificate_verification");
    this->log_level =
        kafka_delivery_cfg_parameters.at("log_level");

    set_kafka_properties(this->properties);
}

void KafkaDelivery::set_kafka_properties(kafka::Properties &properties)
{
    properties.put("bootstrap.servers", get_bootstrap_servers());
    properties.put("enable.idempotence", get_enable_idempotence());
    properties.put("client.id", get_client_id());
    properties.put("security.protocol", get_security_protocol());
    properties.put("ssl.key.location", get_ssl_key_location());
    properties.put("ssl.key.password", get_ssl_key_password());
    properties.put("ssl.certificate.location", get_ssl_certificate_location());
    properties.put("ssl.ca.location", get_ssl_ca_location());
    properties.put("enable.ssl.certificate.verification", get_enable_ssl_certificate_verification());
    properties.put("log_level", get_log_level());
}

bool KafkaDelivery::AsyncKafkaProducer(
    kafka::clients::KafkaProducer &producer,
    const std::string &peer,
    const std::string &json_str)
{
    if (json_str.empty()) {
        spdlog::get("multi-logger")->
            error("[AsyncKafkaProducer] data-delivery issue: "
            "empty JSON received");
        return false;
    }

    try {
        kafka::Topic topic = get_topic();
        kafka::Properties properties = get_properties();

	spdlog::get("multi-logger")->debug("[AsyncKafkaProducer] Attempting to send with key: '{}'", peer);
        auto record = kafka::clients::producer::ProducerRecord(
            topic,
            kafka::Key(peer.c_str(), peer.size()),
            kafka::Value(json_str.c_str(), json_str.size())
            );

        producer.send(
            record,
            [](const kafka::clients::producer::RecordMetadata &mdata,
                const kafka::Error &err) {
            if (!err) {
                spdlog::get("multi-logger")->
                    info("[AsyncKafkaProducer] data-delivery: "
                    "message successfully delivered");
            } else {
                spdlog::get("multi-logger")->
                    error("[AsyncKafkaProducer] data-delivery "
                    "issue: message delivery failure, {}", err.message());
            }
        }, kafka::clients::KafkaProducer::SendOption::ToCopyRecordValue);
    } catch (const kafka::KafkaException &kex) {
        spdlog::get("multi-logger")->
            error("[AsyncKafkaProducer] data-delivery issue: "
            "{}", kex.what());
        return false;
    }

    return true;
}

