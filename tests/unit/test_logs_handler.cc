// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include "utils/logs_handler.h"
#include "utils/cfg_handler.h"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>


namespace {

// Each case starts from a clean spdlog registry so we control which
// loggers exist when LogsHandler runs.
class LogsHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        spdlog::drop("multi-logger");
        spdlog::drop("multi-logger-boot");
    }
    void TearDown() override {
        spdlog::drop("multi-logger");
        spdlog::drop("multi-logger-boot");
    }
};

TEST_F(LogsHandlerTest, ConstructorRegistersBootLogger) {
    LogsHandler h;
    EXPECT_NE(spdlog::get("multi-logger-boot"), nullptr);
}

TEST_F(LogsHandlerTest, SetSpdlogSinksRegistersMultiLogger) {
    LogsHandler h;
    // Minimum config the post-cfg sink setup needs.
    logs_cfg_parameters["syslog"]          = "false";
    logs_cfg_parameters["syslog_facility"] = "NONE";
    logs_cfg_parameters["console_log"]     = "true";
    logs_cfg_parameters["spdlog_level"]    = "info";
    EXPECT_TRUE(h.set_spdlog_sinks());
    EXPECT_NE(spdlog::get("multi-logger"), nullptr);
}

TEST_F(LogsHandlerTest, DestructorIsNullSafeWithoutMultiLogger) {
    // Construct then destruct without ever calling set_spdlog_sinks().
    // Dtor must NOT segfault when multi-logger is unregistered. If the
    // null-safe fallback is broken, this test crashes the process.
    {
        LogsHandler h;
        EXPECT_EQ(spdlog::get("multi-logger"), nullptr);
    }
    SUCCEED();
}

}  // namespace
