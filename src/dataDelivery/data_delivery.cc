// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "dataDelivery/data_delivery.h"


bool DataDelivery::AsyncKafkaProducer(const std::string &json_str)
{
    kafka::Topic topic = data_delivery_cfg_parameters.at("topic");

    try {
        kafka::Properties properties ({
            {"bootstrap.servers",
                data_delivery_cfg_parameters.at("bootstrap_servers")},
            {"enable.idempotence",
                data_delivery_cfg_parameters.at("enable_idempotence")},
            {"client.id",
                data_delivery_cfg_parameters.at("client_id")},
            {"security.protocol",
                data_delivery_cfg_parameters.at("security_protocol")},
            {"ssl.key.location",
                data_delivery_cfg_parameters.at("ssl_key_location")},
            {"ssl.certificate.location",
                data_delivery_cfg_parameters.at("ssl_certificate_location")},
            {"ssl.ca.location",
                data_delivery_cfg_parameters.at("ssl_ca_location")},
            {"log_level",
                data_delivery_cfg_parameters.at("log_level")},
        });

        kafka::clients::KafkaProducer producer(properties);

        if (json_str.empty()) {
            multi_logger->error("[AsyncKafkaProducer] data-delivery issue: "
                "empty JSON received");
            return false;
        }

        auto msg = kafka::clients::producer::ProducerRecord(topic,
            kafka::NullKey,
            kafka::Value(json_str.c_str(), json_str.size()));

        producer.send(
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

