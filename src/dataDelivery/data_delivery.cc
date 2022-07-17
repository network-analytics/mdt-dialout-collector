#include <iostream>
#include "kafka/KafkaProducer.h"
#include "cfg_handler.h"
#include "dataDelivery/data_delivery.h"


int DataDelivery::async_kafka_producer(const std::string& json_str)
{
    // --- Required for config parameters ---
    std::unique_ptr<KafkaCfgHandler> kafka_cfg_handler(new KafkaCfgHandler());

    kafka::Topic topic =
        kafka_cfg_handler->get_kafka_topic();
    std::string bootstrap_servers =
        kafka_cfg_handler->get_kafka_bootstrap_servers();
    std::string enable_idempotence =
        kafka_cfg_handler->get_kafka_enable_idempotence();
    std::string client_id =
        kafka_cfg_handler->get_kafka_client_id();
    std::string security_protocol =
        kafka_cfg_handler->get_kafka_security_protocol();
    std::string ssl_key_location =
        kafka_cfg_handler->get_kafka_ssl_key_location();
    std::string ssl_certificate_location =
        kafka_cfg_handler->get_kafka_ssl_certificate_location();
    std::string ssl_ca_location =
        kafka_cfg_handler->get_kafka_ssl_ca_location();
    std::string log_level =
        kafka_cfg_handler->get_kafka_log_level();
    // --- Required for config parameters ---

    try {
        // Additional kafka producer's config options here
        kafka::Properties properties ({
            {"bootstrap.servers",  bootstrap_servers},
            {"enable.idempotence", enable_idempotence},
            {"client.id", client_id},
            {"security.protocol", security_protocol},
            {"ssl.key.location", ssl_key_location},
            {"ssl.certificate.location", ssl_certificate_location},
            {"ssl.ca.location", ssl_ca_location},
            {"log_level", log_level},
        });

        kafka::clients::KafkaProducer producer(properties);

        if (json_str.empty()) {
            // Implementing a better handling
            std::cout << "KAFKA - Empty JSON received " << std::endl;
            return EXIT_FAILURE;
        }

        auto msg = kafka::clients::producer::ProducerRecord(topic,
            kafka::NullKey,
            kafka::Value(json_str.c_str(), json_str.size()));

        producer.send(
            msg,
            [](const kafka::clients::producer::RecordMetadata& mdata,
                const kafka::Error& err) {
            if (!err) {
                std::cout << "Msg delivered: "
                    << mdata.toString() << std::endl;
            } else {
                std::cerr << "Msg delivery failed: "
                    << err.message() << std::endl;
            }
        }, kafka::clients::KafkaProducer::SendOption::ToCopyRecordValue);
    } catch (const kafka::KafkaException& ex) {
        std::cerr << "Unexpected exception: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

