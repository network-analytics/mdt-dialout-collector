// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _PEER_PARSER_H_
#define _PEER_PARSER_H_

#include <string>


// gRPC server_ctx_.peer() string parser. Handles both address families:
//   "ipv4:1.2.3.4:5678"        -> ip="1.2.3.4",        port="5678"
//   "ipv6:[2001:db8::1]:5678"  -> ip="2001:db8::1",    port="5678"
//                                 (brackets stripped — downstream
//                                 consumers (label_map lookups, kafka
//                                 keys) want the bare address.)
// Empty fields on parse failure.
struct PeerEndpoint {
    std::string ip;
    std::string port;
};

PeerEndpoint ParsePeer(const std::string &peer);

#endif
