// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include "utils/peer_parser.h"

#include <gtest/gtest.h>


TEST(PeerParserTest, Ipv4HappyPath) {
    auto p = ParsePeer("ipv4:192.168.100.254:10007");
    EXPECT_EQ(p.ip,   "192.168.100.254");
    EXPECT_EQ(p.port, "10007");
}

TEST(PeerParserTest, Ipv4Loopback) {
    auto p = ParsePeer("ipv4:127.0.0.1:5555");
    EXPECT_EQ(p.ip,   "127.0.0.1");
    EXPECT_EQ(p.port, "5555");
}

TEST(PeerParserTest, Ipv6HappyPath) {
    auto p = ParsePeer("ipv6:[2001:db8::1]:10007");
    EXPECT_EQ(p.ip,   "2001:db8::1");  // brackets stripped
    EXPECT_EQ(p.port, "10007");
}

TEST(PeerParserTest, Ipv6Loopback) {
    auto p = ParsePeer("ipv6:[::1]:5555");
    EXPECT_EQ(p.ip,   "::1");
    EXPECT_EQ(p.port, "5555");
}

TEST(PeerParserTest, Ipv6FullExpanded) {
    auto p = ParsePeer("ipv6:[2001:0db8:0000:0000:0000:ff00:0042:8329]:443");
    EXPECT_EQ(p.ip,   "2001:0db8:0000:0000:0000:ff00:0042:8329");
    EXPECT_EQ(p.port, "443");
}

TEST(PeerParserTest, EmptyInputReturnsEmpty) {
    auto p = ParsePeer("");
    EXPECT_TRUE(p.ip.empty());
    EXPECT_TRUE(p.port.empty());
}

TEST(PeerParserTest, NoColonReturnsEmpty) {
    auto p = ParsePeer("garbage");
    EXPECT_TRUE(p.ip.empty());
    EXPECT_TRUE(p.port.empty());
}

TEST(PeerParserTest, Ipv6MissingClosingBracketReturnsEmpty) {
    auto p = ParsePeer("ipv6:[2001:db8::1");
    EXPECT_TRUE(p.ip.empty());
    EXPECT_TRUE(p.port.empty());
}

TEST(PeerParserTest, Ipv6MissingPortAfterBracketReturnsEmpty) {
    auto p = ParsePeer("ipv6:[2001:db8::1]");
    EXPECT_TRUE(p.ip.empty());
    EXPECT_TRUE(p.port.empty());
}
