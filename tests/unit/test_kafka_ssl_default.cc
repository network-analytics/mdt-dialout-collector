// Regression test for KafkaCfgHandler::lookup_kafka_parameters.
// SSL default: when security_protocol="ssl" and the operator does NOT set
// enable_ssl_certificate_verification, the parsed value MUST be "true".

#include "utils/cfg_handler.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>

#include <gtest/gtest.h>

#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <unistd.h>

namespace {

constexpr const char *kSslNoVerifyKey = R"(
topic = "t";
bootstrap_servers = "broker:9093";
security_protocol = "ssl";
ssl_key_location = "/dev/null";
ssl_certificate_location = "/dev/null";
ssl_ca_location = "/dev/null";
)";

constexpr const char *kSslVerifyFalse = R"(
topic = "t";
bootstrap_servers = "broker:9093";
security_protocol = "ssl";
ssl_key_location = "/dev/null";
ssl_certificate_location = "/dev/null";
ssl_ca_location = "/dev/null";
enable_ssl_certificate_verification = "false";
)";

constexpr const char *kSslVerifyTrue = R"(
topic = "t";
bootstrap_servers = "broker:9093";
security_protocol = "ssl";
ssl_key_location = "/dev/null";
ssl_certificate_location = "/dev/null";
ssl_ca_location = "/dev/null";
enable_ssl_certificate_verification = "true";
)";

constexpr const char *kPlaintext = R"(
topic = "t";
bootstrap_servers = "broker:9092";
security_protocol = "plaintext";
)";

class KafkaSslDefaultTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Idempotent: gtest fixtures may share the process, but logger
        // registration is global.
        if (!spdlog::get("multi-logger-boot")) {
            auto null_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
            spdlog::register_logger(std::make_shared<spdlog::logger>(
                "multi-logger-boot", null_sink));
            spdlog::register_logger(std::make_shared<spdlog::logger>(
                "multi-logger", null_sink));
        }
        main_cfg_parameters["data_delivery_method"] = "kafka";
    }

    static std::string write_tmp_cfg(const std::string &body) {
        char path[] = "/tmp/mdt_ut_cfg_XXXXXX";
        int fd = mkstemp(path);
        EXPECT_GE(fd, 0);
        auto n = ::write(fd, body.data(), body.size());
        (void)n;
        ::close(fd);
        return std::string(path);
    }

    static std::string parse_and_get_verify(const char *cfg) {
        auto path = write_tmp_cfg(cfg);
        std::map<std::string, std::string> params;
        KafkaCfgHandler h;
        EXPECT_TRUE(h.lookup_kafka_parameters(path, params));
        std::remove(path.c_str());
        return params.at("enable_ssl_certificate_verification");
    }
};

TEST_F(KafkaSslDefaultTest, SslWithoutVerifyKeyDefaultsToTrue) {
    // Regression guard for commit 9f2b70f: the implicit default under SSL
    // must be "true" so we don't silently re-introduce MITM exposure.
    EXPECT_EQ(parse_and_get_verify(kSslNoVerifyKey), "true");
}

TEST_F(KafkaSslDefaultTest, SslExplicitFalsePreserved) {
    EXPECT_EQ(parse_and_get_verify(kSslVerifyFalse), "false");
}

TEST_F(KafkaSslDefaultTest, SslExplicitTruePreserved) {
    EXPECT_EQ(parse_and_get_verify(kSslVerifyTrue), "true");
}

TEST_F(KafkaSslDefaultTest, PlaintextKeepsFalse) {
    // No SSL in play, so the value is moot but should remain "false" for
    // downstream consumers that read the key unconditionally.
    EXPECT_EQ(parse_and_get_verify(kPlaintext), "false");
}

}  // namespace
