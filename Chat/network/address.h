#pragma once

#include <cstdint>
#include <string>

namespace net
{
    inline std::string IPv4(uint32_t i)
    {
        return std::to_string((i & 0xFF)) + "."
        + std::to_string(((i >> 8) & 0xFF)) + "."
        + std::to_string(((i >> 16) & 0xFF)) + "."
        + std::to_string(((i >> 24) & 0xFF));
    }

    /**
     * ENet Address
     */
    struct Address
    {
        uint32_t host; ///< IPv4 address
        uint16_t port; ///< Port number
        Address(uint32_t inHost = 0, uint16_t inPort = 0): host(inHost), port(inPort) {}
        std::string str() const { return IPv4(host) + ":" + std::to_string(port); }
        bool operator==(const Address& other) { return host == other.host && port == other.port; }
        bool operator!=(const Address& other) { return !(*this == other); }
        operator bool() { return port > 0; } // ensure values are set for this address
    };
}
