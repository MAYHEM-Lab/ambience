//
// Created by fatih on 3/9/19.
//

#pragma once

#include <cstdint>
#include <cstring>
#include <algorithm>

namespace authn
{
    struct id_t
    {
        uint8_t id[8];
        bool operator==(const id_t& rhs) const
        {
            return std::equal(id, id + 8, rhs.id);
        }

        bool operator !=(const id_t& rhs) const
        {
            return !(*this == rhs);
        }
    };

    struct path_t
    {
        char path[16];
        bool operator==(const path_t& rhs) const
        {
            return std::strncmp(path, rhs.path, std::size(path)) == 0;
        }

        bool operator !=(const path_t& rhs) const
        {
            return !(*this == rhs);
        }
    };

    enum class rights
    {
        read  = 0b01,
        write = 0b10,
        full  = 0b11
    };

    struct cap_t
    {
        enum class types { id, path } t;

        union {
            id_t m_id;
            path_t m_path;
        };

        rights m_rights;

        cap_t() = default;
        cap_t(id_t id, rights r) : t{types::id}, m_id{id}, m_rights{r} { }
        cap_t(path_t path, rights r) : t{types::path}, m_path{path}, m_rights{r} { }
    };

    bool satisfies(const cap_t& haystack, const cap_t& needle)
    {
        if (haystack.t != needle.t) return false;

        switch (haystack.t) {
            case cap_t::types::id:
                if (haystack.m_id != needle.m_id)
                    return false;
                break;
            case cap_t::types::path:
                if (haystack.m_path != needle.m_path)
                    return false;
                break;
        }

        if ((int(haystack.m_rights) & int(needle.m_rights)) != int(needle.m_rights))
            return false;

        return true;
    }

    struct request
    {
        authn::path_t p;
    };

    inline authn::cap_t req_to_tok(const request& r)
    {
        return authn::cap_t(r.p, authn::rights::read);
    }

    inline auto parse_req(tos::span<const uint8_t> pl) {
        auto path = authn::path_t{};
        auto sz = std::min(pl.size(), std::size(path.path));
        memcpy(path.path, pl.data(), sz);
        path.path[sz] = 0;
        return request{path};
    }
} // namespace authn
