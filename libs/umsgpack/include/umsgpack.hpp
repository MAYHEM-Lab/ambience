//
// Created by fatih on 8/20/18.
//

#pragma once

extern "C"
{
#include <umsgpack.h>
}
#undef bool

#include <string.h>
#include <tos/span.hpp>

namespace tos
{
    namespace msgpack
    {
        class packer;

        class map_packer
        {
        public:
            template <class ValT>
            void insert(const char* name, ValT val);

        private:
            friend class packer;
            map_packer(packer& p, size_t len);

            size_t m_len;
            size_t m_done;
            packer& m_packer;
        };

        class arr_packer
        {
        public:
            template <class T>
            void insert(T val);

            map_packer insert_map(size_t len);

        private:
            friend class packer;
            arr_packer(packer& p, size_t len);

            size_t m_len;
            size_t m_done;
            packer& m_packer;
        };

        class packer
        {
        public:
            explicit packer(span<char> buffer);

            map_packer insert_map(size_t len)
            {
                umsgpack_pack_map(m_buf, len);
                return {*this, len};
            }

            arr_packer insert_arr(size_t len)
            {
                umsgpack_pack_array(m_buf, len);
                return {*this, len};
            }

            span<const char> get()
            {
                return {(const char*)m_buf->data, umsgpack_get_length(m_buf)};
            }

            void put(float val);
            void put(int val);
            void put(const char* str);

        private:
            span<char> m_buffer;
            umsgpack_packer_buf* m_buf;
        };
    }
};

/// IMPL

namespace tos
{
    namespace msgpack
    {
        inline packer::packer(span<char> buffer) : m_buffer{buffer} {
            m_buf = (umsgpack_packer_buf*)buffer.data();
            umsgpack_packer_init(m_buf, buffer.size());
        }

        inline void packer::put(float val) {
            umsgpack_pack_float(m_buf, val);
        }

        inline void packer::put(int val) {
            umsgpack_pack_int(m_buf, val);
        }

        inline void packer::put(const char *str) {
            umsgpack_pack_str(m_buf, (char*)str, strlen(str));
        }

        inline map_packer::map_packer(packer &p, size_t len)
                : m_len{len}, m_done{0}, m_packer{p} {
        }

        template<class ValT>
        void map_packer::insert(const char *name, ValT val) {
            if (m_done == m_len)
            {
                //TODO: error
            }
            m_packer.put(name);
            m_packer.put(val);
            ++m_done;
        }

        template<class T>
        void arr_packer::insert(T val) {
            if (m_done == m_len)
            {
                //TODO: error
            }
            m_packer.put(val);
            ++m_done;
        }

        arr_packer::arr_packer(packer &p, size_t len) : m_len{len}, m_done{0}, m_packer{p} {

        }

        map_packer arr_packer::insert_map(size_t len) {
            return m_packer.insert_map(len);
        }
    }
};