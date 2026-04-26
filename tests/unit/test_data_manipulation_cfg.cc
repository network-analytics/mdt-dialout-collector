// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include "cfg_test_common.h"

namespace {

class DataManipulationCfgTest : public ::testing::Test {
protected:
    void SetUp() override { mdt_test::EnsureNullLoggers(); }

    static std::map<std::string, std::string> Parse(const char *body) {
        mdt_test::TmpCfg cfg(body);
        std::map<std::string, std::string> params;
        DataManipulationCfgHandler h;
        EXPECT_TRUE(h.lookup_data_manipulation_parameters(
            cfg.path(), params));
        return params;
    }

    static bool ParseExpectFail(const char *body) {
        mdt_test::TmpCfg cfg(body);
        std::map<std::string, std::string> params;
        DataManipulationCfgHandler h;
        return !h.lookup_data_manipulation_parameters(cfg.path(), params);
    }
};

TEST_F(DataManipulationCfgTest, DefaultsSatisfyCiscoXor) {
    // Defaults: gpbkv2json=true, message_to_json_string=false → XOR holds.
    auto p = Parse("");
    EXPECT_EQ(p.at("enable_cisco_gpbkv2json"),             "true");
    EXPECT_EQ(p.at("enable_cisco_message_to_json_string"), "false");
    EXPECT_EQ(p.at("enable_label_encode_as_map"),          "false");
    EXPECT_EQ(p.at("enable_label_encode_as_map_ptm"),      "false");
}

TEST_F(DataManipulationCfgTest, CiscoBothTrueRejected) {
    EXPECT_TRUE(ParseExpectFail(R"(
        enable_cisco_gpbkv2json             = "true";
        enable_cisco_message_to_json_string = "true";
    )"));
}

TEST_F(DataManipulationCfgTest, CiscoBothFalseRejected) {
    EXPECT_TRUE(ParseExpectFail(R"(
        enable_cisco_gpbkv2json             = "false";
        enable_cisco_message_to_json_string = "false";
    )"));
}

TEST_F(DataManipulationCfgTest, CiscoMessageToJsonOnlyAccepted) {
    auto p = Parse(R"(
        enable_cisco_gpbkv2json             = "false";
        enable_cisco_message_to_json_string = "true";
    )");
    EXPECT_EQ(p.at("enable_cisco_gpbkv2json"),             "false");
    EXPECT_EQ(p.at("enable_cisco_message_to_json_string"), "true");
}

TEST_F(DataManipulationCfgTest, LabelEncodersBothTrueRejected) {
    // Both csv and ptm enabled with paths that exist (so the XOR check
    // is what trips, not the file-existence checks).
    EXPECT_TRUE(ParseExpectFail(R"(
        enable_label_encode_as_map     = "true";
        label_map_csv_path             = "/dev/null";
        enable_label_encode_as_map_ptm = "true";
        label_map_ptm_path             = "/dev/null";
    )"));
}

TEST_F(DataManipulationCfgTest, LabelEncoderCsvHappyPath) {
    auto p = Parse(R"(
        enable_label_encode_as_map = "true";
        label_map_csv_path         = "/dev/null";
    )");
    EXPECT_EQ(p.at("enable_label_encode_as_map"), "true");
    EXPECT_EQ(p.at("label_map_csv_path"),         "/dev/null");
}

TEST_F(DataManipulationCfgTest, LabelEncoderCsvNonexistentPathRejected) {
    EXPECT_TRUE(ParseExpectFail(R"(
        enable_label_encode_as_map = "true";
        label_map_csv_path         = "/no/such/file.csv";
    )"));
}

TEST_F(DataManipulationCfgTest, LabelEncoderPtmHappyPath) {
    auto p = Parse(R"(
        enable_label_encode_as_map_ptm = "true";
        label_map_ptm_path             = "/dev/null";
    )");
    EXPECT_EQ(p.at("enable_label_encode_as_map_ptm"), "true");
    EXPECT_EQ(p.at("label_map_ptm_path"),             "/dev/null");
}

}  // namespace
