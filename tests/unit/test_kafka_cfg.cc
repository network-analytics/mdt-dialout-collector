// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// Coverage for KafkaCfgHandler beyond the SSL-default regression test
// in test_kafka_ssl_default.cc.

#include "cfg_test_common.h"

namespace {

// Minimal kafka cfg (used when delivery is "kafka").
constexpr const char *kKafkaMinimal = R"(
    topic             = "t";
    bootstrap_servers = "broker:9092";
    security_protocol = "plaintext";
)";

class KafkaCfgTest : public ::testing::Test {
protected:
    void SetUp() override {
        mdt_test::EnsureNullLoggers();
        // Default: kafka delivery — KafkaCfgHandler reads this global.
        main_cfg_parameters["data_delivery_method"] = "kafka";
    }

    static std::map<std::string, std::string> Parse(const std::string &body) {
        mdt_test::TmpCfg cfg(body);
        std::map<std::string, std::string> params;
        KafkaCfgHandler h;
        EXPECT_TRUE(h.lookup_kafka_parameters(cfg.path(), params));
        return params;
    }

    static bool ParseExpectFail(const std::string &body) {
        mdt_test::TmpCfg cfg(body);
        std::map<std::string, std::string> params;
        KafkaCfgHandler h;
        return !h.lookup_kafka_parameters(cfg.path(), params);
    }
};

TEST_F(KafkaCfgTest, KafkaModeMissingTopicRejected) {
    EXPECT_TRUE(ParseExpectFail(R"(
        bootstrap_servers = "broker:9092";
        security_protocol = "plaintext";
    )"));
}

TEST_F(KafkaCfgTest, KafkaModeMissingBootstrapServersRejected) {
    EXPECT_TRUE(ParseExpectFail(R"(
        topic             = "t";
        security_protocol = "plaintext";
    )"));
}

TEST_F(KafkaCfgTest, KafkaModeMissingSecurityProtocolRejected) {
    EXPECT_TRUE(ParseExpectFail(R"(
        topic             = "t";
        bootstrap_servers = "broker:9092";
    )"));
}

TEST_F(KafkaCfgTest, ZmqModeFillsKafkaDummies) {
    main_cfg_parameters["data_delivery_method"] = "zmq";
    auto p = Parse("");
    EXPECT_EQ(p.at("topic"),             "dummy_topic");
    EXPECT_EQ(p.at("bootstrap_servers"), "dummy_servers");
    EXPECT_EQ(p.at("security_protocol"), "dummy_security_protocol");
}

TEST_F(KafkaCfgTest, KafkaModeMinimalGetsDefaults) {
    auto p = Parse(kKafkaMinimal);
    EXPECT_EQ(p.at("enable_idempotence"), "true");
    EXPECT_EQ(p.at("client_id"),          "mdt-dialout-collector");
    EXPECT_EQ(p.at("log_level"),          "6");
    // plaintext: SSL bundle filled with NULL placeholders.
    EXPECT_EQ(p.at("ssl_key_location"),                     "NULL");
    EXPECT_EQ(p.at("ssl_certificate_location"),             "NULL");
    EXPECT_EQ(p.at("ssl_ca_location"),                      "NULL");
    EXPECT_EQ(p.at("ssl_key_password"),                     "NULL");
    EXPECT_EQ(p.at("enable_ssl_certificate_verification"),  "false");
}

TEST_F(KafkaCfgTest, InvalidSecurityProtocolRejected) {
    EXPECT_TRUE(ParseExpectFail(R"(
        topic             = "t";
        bootstrap_servers = "broker:9092";
        security_protocol = "tcp";
    )"));
}

TEST_F(KafkaCfgTest, InvalidEnableIdempotenceRejected) {
    EXPECT_TRUE(ParseExpectFail(std::string(kKafkaMinimal) +
        R"(enable_idempotence = "maybe";)"));
}

TEST_F(KafkaCfgTest, SslMissingMandatoryComponentRejected) {
    // ssl_key_location is mandatory under SSL; missing → false.
    EXPECT_TRUE(ParseExpectFail(R"(
        topic                    = "t";
        bootstrap_servers        = "broker:9093";
        security_protocol        = "ssl";
        ssl_certificate_location = "/dev/null";
        ssl_ca_location          = "/dev/null";
    )"));
}

TEST_F(KafkaCfgTest, SslAllPresentAccepted) {
    auto p = Parse(R"(
        topic                    = "t";
        bootstrap_servers        = "broker:9093";
        security_protocol        = "ssl";
        ssl_key_location         = "/dev/null";
        ssl_certificate_location = "/dev/null";
        ssl_ca_location          = "/dev/null";
        ssl_key_password         = "secret";
    )");
    EXPECT_EQ(p.at("ssl_key_password"), "secret");
    // Secure-by-default fires when verification key is omitted.
    EXPECT_EQ(p.at("enable_ssl_certificate_verification"), "true");
}

}  // namespace
