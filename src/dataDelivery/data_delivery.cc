// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "dataDelivery/data_delivery.h"


DataDelivery::DataDelivery()
{
    multi_logger->debug("constructor: DataDelivery()");
    this->topic =
        data_delivery_cfg_parameters.at("topic");
    this->bootstrap_servers =
        data_delivery_cfg_parameters.at("bootstrap_servers");
    this->enable_idempotence =
        data_delivery_cfg_parameters.at("enable_idempotence");
    this->client_id =
        data_delivery_cfg_parameters.at("client_id");
    this->security_protocol =
        data_delivery_cfg_parameters.at("security_protocol");
    this->ssl_key_location =
        data_delivery_cfg_parameters.at("ssl_key_location");
    this->ssl_certificate_location =
        data_delivery_cfg_parameters.at("ssl_certificate_location");
    this->ssl_ca_location =
        data_delivery_cfg_parameters.at("ssl_ca_location");
    this->log_level =
        data_delivery_cfg_parameters.at("log_level");

    set_kafka_properties(this->properties);
}

void DataDelivery::set_kafka_properties(kafka::Properties &properties)
{
    properties.put("bootstrap.servers", get_bootstrap_servers());
    properties.put("enable.idempotence", get_enable_idempotence());
    properties.put("client.id", get_client_id());
    properties.put("security.protocol", get_security_protocol());
    properties.put("ssl.key.location", get_ssl_key_location());
    properties.put("ssl.certificate.location", get_ssl_certificate_location());
    properties.put("ssl.ca.location", get_ssl_ca_location());
    properties.put("log_level", get_log_level());
}

bool DataDelivery::AsyncKafkaProducer(
    std::unique_ptr<kafka::clients::KafkaProducer> &producer,
    const std::string &peer,
    const std::string &json_str)
{
    if (json_str.empty()) {
        multi_logger->error("[AsyncKafkaProducer] data-delivery issue: "
            "empty JSON received");
        return false;
    }

    try {
        kafka::Topic topic = get_topic();
        kafka::Properties properties = get_properties();

        auto msg = kafka::clients::producer::ProducerRecord(
            topic,
            kafka::Key{peer.c_str()},
            kafka::Value(json_str.c_str(), json_str.size()));

        producer->send(
            msg,
            [](const kafka::clients::producer::RecordMetadata &mdata,
                const kafka::Error &err) {
            if (!err) {
                multi_logger->info("[AsyncKafkaProducer] data-delivery: "
                    "message successfully delivered");
            } else {
                multi_logger->error("[AsyncKafkaProducer] data-delivery "
                    "issue: message delivery failure, {}", err.message());
            }
        }, kafka::clients::KafkaProducer::SendOption::ToCopyRecordValue);
    } catch (const kafka::KafkaException &kex) {
        multi_logger->error("[AsyncKafkaProducer] data-delivery issue: "
            "{}", kex.what());
        return false;
    }

    return true;
}

