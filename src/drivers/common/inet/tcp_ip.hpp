//
// Created by fatih on 6/30/18.
//

#pragma once

#include <stdint.h>
#include <string.h>

namespace tos
{
    /**
     * This class represents an IP port number
     */
    struct port_num_t
    {
        uint16_t port;
    };

    inline bool operator==(const port_num_t& a, const port_num_t& b)
    {
        return a.port == b.port;
    }

    inline bool operator!=(const port_num_t& a, const port_num_t& b)
    {
        return a.port != b.port;
    }

    struct ipv4_addr
    {
        uint8_t addr[4];
    };

    inline bool operator==(const ipv4_addr& a, const ipv4_addr& b)
    {
        return memcmp(a.addr, b.addr, 4) == 0;
    }

    inline bool operator!=(const ipv4_addr& a, const ipv4_addr& b)
    {
        return memcmp(a.addr, b.addr, 4) != 0;
    }
}