// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include "cfg_test_common.h"

namespace {

// Minimal valid main cfg: just iface (mandatory) + a writable core_pid_folder.
constexpr const char *kMinimalMain = R"(
    iface = "lo";
    core_pid_folder = "/tmp";
)";

class MainCfgTest : public ::testing::Test {
protected:
    void SetUp() override { mdt_test::EnsureNullLoggers(); }

    static std::map<std::string, std::string> Parse(const std::string &body) {
        mdt_test::TmpCfg cfg(body);
        std::map<std::string, std::string> params;
        MainCfgHandler h;
        EXPECT_TRUE(h.lookup_main_parameters(cfg.path(), params));
        return params;
    }

    static bool ParseExpectFail(const std::string &body) {
        mdt_test::TmpCfg cfg(body);
        std::map<std::string, std::string> params;
        MainCfgHandler h;
        return !h.lookup_main_parameters(cfg.path(), params);
    }
};

TEST_F(MainCfgTest, MissingIfaceFails) {
    EXPECT_TRUE(ParseExpectFail(R"(core_pid_folder = "/tmp";)"));
}

TEST_F(MainCfgTest, MinimalConfigGetsAllDefaults) {
    auto p = Parse(kMinimalMain);
    EXPECT_EQ(p.at("writer_id"),             "mdt-dialout-collector");
    EXPECT_EQ(p.at("so_bindtodevice_check"), "true");
    EXPECT_EQ(p.at("enable_tls"),            "false");
    EXPECT_EQ(p.at("tls_cert_path"),         "");
    EXPECT_EQ(p.at("tls_key_path"),          "");
    EXPECT_EQ(p.at("data_delivery_method"),  "kafka");
    EXPECT_EQ(p.at("socket_cisco"),          "");
    EXPECT_EQ(p.at("socket_juniper"),        "");
    EXPECT_EQ(p.at("socket_nokia"),          "");
    EXPECT_EQ(p.at("socket_huawei"),         "");
}

TEST_F(MainCfgTest, TlsHappyPath) {
    auto p = Parse(std::string(kMinimalMain) + R"(
        enable_tls    = "true";
        tls_cert_path = "/dev/null";
        tls_key_path  = "/dev/null";
    )");
    EXPECT_EQ(p.at("enable_tls"),    "true");
    EXPECT_EQ(p.at("tls_cert_path"), "/dev/null");
    EXPECT_EQ(p.at("tls_key_path"),  "/dev/null");
}

TEST_F(MainCfgTest, TlsEnabledMissingKeyRejected) {
    EXPECT_TRUE(ParseExpectFail(std::string(kMinimalMain) + R"(
        enable_tls    = "true";
        tls_cert_path = "/dev/null";
    )"));
}

TEST_F(MainCfgTest, TlsEnabledNonexistentCertRejected) {
    EXPECT_TRUE(ParseExpectFail(std::string(kMinimalMain) + R"(
        enable_tls    = "true";
        tls_cert_path = "/no/such/cert";
        tls_key_path  = "/dev/null";
    )"));
}

TEST_F(MainCfgTest, InvalidEnableTlsRejected) {
    EXPECT_TRUE(ParseExpectFail(std::string(kMinimalMain) +
        R"(enable_tls = "yes";)"));
}

TEST_F(MainCfgTest, InvalidDataDeliveryMethodRejected) {
    EXPECT_TRUE(ParseExpectFail(std::string(kMinimalMain) +
        R"(data_delivery_method = "kfaka";)"));
}

TEST_F(MainCfgTest, InvalidSoBindtodeviceCheckRejected) {
    EXPECT_TRUE(ParseExpectFail(std::string(kMinimalMain) +
        R"(so_bindtodevice_check = "maybe";)"));
}

TEST_F(MainCfgTest, RepliesAndWorkersOnlyForConfiguredVendors) {
    auto p = Parse(std::string(kMinimalMain) + R"(
        socket_cisco = "0.0.0.0:10001";
    )");
    EXPECT_EQ(p.at("replies_cisco"), "0");
    EXPECT_EQ(p.at("cisco_workers"), "1");
    // juniper/nokia/huawei not configured → no replies_*/workers entries.
    EXPECT_EQ(p.count("replies_juniper"),  0u);
    EXPECT_EQ(p.count("juniper_workers"),  0u);
    EXPECT_EQ(p.count("replies_nokia"),    0u);
    EXPECT_EQ(p.count("nokia_workers"),    0u);
    EXPECT_EQ(p.count("replies_huawei"),   0u);
    EXPECT_EQ(p.count("huawei_workers"),   0u);
}

TEST_F(MainCfgTest, NonexistentCorePidFolderRejected) {
    EXPECT_TRUE(ParseExpectFail(R"(
        iface = "lo";
        core_pid_folder = "/no/such/dir";
    )"));
}

TEST_F(MainCfgTest, Ipv6SocketAccepted) {
    auto p = Parse(std::string(kMinimalMain) + R"(
        socket_cisco = "[::]:10001";
        socket_juniper = "[2001:db8::1]:10002";
    )");
    EXPECT_EQ(p.at("socket_cisco"),   "[::]:10001");
    EXPECT_EQ(p.at("socket_juniper"), "[2001:db8::1]:10002");
}

TEST_F(MainCfgTest, LegacyIpv4SocketAliasHonored) {
    // ipv4_socket_<vendor> is read into the new socket_<vendor> key
    // when the new key is absent — for backward compat with operator
    // configs predating the rename.
    auto p = Parse(std::string(kMinimalMain) + R"(
        ipv4_socket_cisco = "0.0.0.0:10001";
    )");
    EXPECT_EQ(p.at("socket_cisco"), "0.0.0.0:10001");
    EXPECT_EQ(p.count("ipv4_socket_cisco"), 0u);  // not stored under old name
}

TEST_F(MainCfgTest, NewKeyTakesPrecedenceOverLegacy) {
    auto p = Parse(std::string(kMinimalMain) + R"(
        socket_cisco      = "0.0.0.0:10001";
        ipv4_socket_cisco = "0.0.0.0:10002";
    )");
    EXPECT_EQ(p.at("socket_cisco"), "0.0.0.0:10001");
}

}  // namespace
