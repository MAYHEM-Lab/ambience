//
// Created by fatih on 8/20/18.
//

#pragma once

extern "C"
{
#include <umsgpack.h>
}
#undef bool

#include <tos/span.hpp>

namespace tos
{
    namespace msgpack
    {
        class packer;

        class map_inserter
        {

        };

        class map_packer
        {
        public:

        private:
            friend class packer;
            map_packer(packer& p, size_t len)
                : m_len{len}, m_done{0}, m_packer{p} {
            }

            size_t m_len;
            size_t m_done;
            packer& m_packer;
        };

        class packer
        {
        public:
            explicit packer(span<char> buffer)
            {
                m_buf = (umsgpack_packer_buf*)buffer.data();
                umsgpack_packer_init(m_buf, buffer.size());
            };

        private:
            umsgpack_packer_buf* m_buf;
        };
    }
};