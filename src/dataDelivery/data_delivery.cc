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
            std::cout << "KAFKA - Empty JSON received\n";
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
                std::cout << "Msg delivered: " << mdata.toString() << "\n";
            } else {
                std::cerr << "Msg delivery failed: " << err.message() << "\n";
            }
        }, kafka::clients::KafkaProducer::SendOption::ToCopyRecordValue);
    } catch (const kafka::KafkaException &ex) {
        std::cerr << "Unexpected exception: " << ex.what() << "\n";
        return false;
    }

    return true;
}

