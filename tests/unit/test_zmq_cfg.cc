// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include "cfg_test_common.h"

namespace {

class ZmqCfgTest : public ::testing::Test {
protected:
    void SetUp() override { mdt_test::EnsureNullLoggers(); }

    static std::string LookupUri(const std::string &uri) {
        std::map<std::string, std::string> params;
        ZmqCfgHandler h;
        EXPECT_TRUE(h.lookup_zmq_parameters(uri, params));
        return params.at("zmq_uri");
    }
};

TEST_F(ZmqCfgTest, KafkaModeUsesDummyUri) {
    main_cfg_parameters["data_delivery_method"] = "kafka";
    EXPECT_EQ(LookupUri(""),                       "ipc:///tmp/dummy.sock");
    EXPECT_EQ(LookupUri("ipc:///tmp/whatever"),    "ipc:///tmp/dummy.sock");
}

TEST_F(ZmqCfgTest, ZmqModeEmptyUriDefaults) {
    main_cfg_parameters["data_delivery_method"] = "zmq";
    EXPECT_EQ(LookupUri(""), "ipc:///tmp/grpc.sock");
}

TEST_F(ZmqCfgTest, ZmqModeCustomUriPreserved) {
    main_cfg_parameters["data_delivery_method"] = "zmq";
    EXPECT_EQ(LookupUri("ipc:///run/mdt-collector.sock"),
              "ipc:///run/mdt-collector.sock");
    EXPECT_EQ(LookupUri("tcp://127.0.0.1:5555"),
              "tcp://127.0.0.1:5555");
}

}  // namespace
