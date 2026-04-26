// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include "peer_parser.h"


PeerEndpoint ParsePeer(const std::string &peer)
{
    PeerEndpoint out;

    // Skip the "ipvX:" family prefix.
    const auto family_sep = peer.find(':');
    if (family_sep == std::string::npos) {
        return out;
    }
    const std::string addr_port = peer.substr(family_sep + 1);
    if (addr_port.empty()) {
        return out;
    }

    if (addr_port.front() == '[') {
        // IPv6 form: "[addr]:port".
        const auto rb = addr_port.find(']');
        if (rb == std::string::npos || rb + 2 > addr_port.size() ||
            addr_port[rb + 1] != ':') {
            return out;
        }
        out.ip   = addr_port.substr(1, rb - 1);
        out.port = addr_port.substr(rb + 2);
        return out;
    }

    // IPv4 form: "addr:port" — port is everything after the last colon.
    const auto port_sep = addr_port.rfind(':');
    if (port_sep == std::string::npos) {
        return out;
    }
    out.ip   = addr_port.substr(0, port_sep);
    out.port = addr_port.substr(port_sep + 1);
    return out;
}
