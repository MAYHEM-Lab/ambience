//
// Created by fatih on 1/8/19.
//

#include <caps/caps.hpp>
#include <tos/ft.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <variant>
#include <tos/mem_stream.hpp>
#include <caps/raw_serdes.hpp>
#include <tos/semaphore.hpp>

using namespace nlohmann;

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

    enum class rights
    {
        read  = 0b01,
        write = 0b10,
        full  = 0b11
    };

    struct cap_t
    {
        enum class types { id } t;

        union {
            id_t m_id;
        };

        rights m_rights;

        cap_t() = default;
        cap_t(id_t id, rights r) : t{types::id}, m_id{id}, m_rights{r} { }
    };

    bool satisfies(const cap_t& haystack, const cap_t& needle)
    {
        if (haystack.t != needle.t) return false;

        switch (haystack.t) {
            case cap_t::types::id:
                if (haystack.m_id != needle.m_id)
                    return false;
                break;
        }

        if ((int(haystack.m_rights) & int(needle.m_rights)) != int(needle.m_rights))
            return false;

        return true;
    }
}

void server_task(tos::span<const char> buf)
{
    tos::imemory_stream str{buf};

    auto c = caps::deserialize<authn::cap_t, caps::emsha::signer>(str);

    authn::id_t my_id { "fatih" };

    auto look = authn::cap_t { my_id, authn::rights::read };

    std::cout << caps::validate(*c, look, authn::satisfies) << '\n';
}

void caps_task()
{
    caps::emsha::signer s { "hiya" };

    authn::id_t my_id { "fatih" };
    auto cap = authn::cap_t{my_id, authn::rights::full};

    auto c = caps::mkcaps({ cap }, s);

    std::array<char, 128> buf;
    tos::omemory_stream str{buf};

    caps::serialize(str, *c);

    tos::semaphore sem{0};
    tos::launch([&]{
        server_task(str.get());
        sem.up();
    });
    sem.down();
}

void tos_main()
{
    tos::launch(caps_task);
}