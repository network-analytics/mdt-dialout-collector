// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include "cfg_test_common.h"

namespace {

class LogsCfgTest : public ::testing::Test {
protected:
    void SetUp() override { mdt_test::EnsureNullLoggers(); }

    static std::map<std::string, std::string> Parse(const char *body) {
        mdt_test::TmpCfg cfg(body);
        std::map<std::string, std::string> params;
        LogsCfgHandler h;
        EXPECT_TRUE(h.lookup_logs_parameters(cfg.path(), params));
        return params;
    }

    static bool ParseExpectFail(const char *body) {
        mdt_test::TmpCfg cfg(body);
        std::map<std::string, std::string> params;
        LogsCfgHandler h;
        return !h.lookup_logs_parameters(cfg.path(), params);
    }
};

TEST_F(LogsCfgTest, EmptyConfigUsesAllDefaults) {
    auto p = Parse("");
    EXPECT_EQ(p.at("syslog"),          "false");
    EXPECT_EQ(p.at("syslog_facility"), "NONE");
    EXPECT_EQ(p.at("syslog_ident"),    "NONE");
    EXPECT_EQ(p.at("console_log"),     "true");
    EXPECT_EQ(p.at("spdlog_level"),    "info");
}

TEST_F(LogsCfgTest, SyslogTrueGetsBundleDefaults) {
    auto p = Parse(R"(syslog = "true";)");
    EXPECT_EQ(p.at("syslog"),          "true");
    EXPECT_EQ(p.at("syslog_facility"), "LOG_USER");
    EXPECT_EQ(p.at("syslog_ident"),    "mdt-dialout-collector");
}

TEST_F(LogsCfgTest, CustomSyslogFacilityAccepted) {
    auto p = Parse(R"(
        syslog = "true";
        syslog_facility = "LOG_LOCAL3";
        syslog_ident = "my-collector";
    )");
    EXPECT_EQ(p.at("syslog_facility"), "LOG_LOCAL3");
    EXPECT_EQ(p.at("syslog_ident"),    "my-collector");
}

TEST_F(LogsCfgTest, InvalidSyslogFacilityRejected) {
    EXPECT_TRUE(ParseExpectFail(R"(
        syslog = "true";
        syslog_facility = "LOG_NOPE";
    )"));
}

TEST_F(LogsCfgTest, InvalidSpdlogLevelRejected) {
    EXPECT_TRUE(ParseExpectFail(R"(spdlog_level = "trace";)"));
}

TEST_F(LogsCfgTest, ValidSpdlogLevelsAccepted) {
    for (const char *lvl : {"debug", "info", "warn", "error", "off"}) {
        std::string cfg = std::string("spdlog_level = \"") + lvl + "\";";
        auto p = Parse(cfg.c_str());
        EXPECT_EQ(p.at("spdlog_level"), lvl);
    }
}

}  // namespace
