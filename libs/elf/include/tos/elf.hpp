#pragma once

#include "tos/error.hpp"
#include <tos/elf/header.hpp>
#include <tos/elf/program_header.hpp>
#include <tos/expected.hpp>
#include <tos/span.hpp>

namespace tos::elf {
enum class errors
{
    too_short,
    bad_magic,
    bad_class,
    bad_endian,
    bad_abi
};

TOS_ERROR_ENUM(errors);

class elf64 {
public:
    explicit elf64(span<const uint8_t> buffer);

    static expected<elf64, errors> from_buffer(span<const uint8_t> buffer);

    const elf64_header& header() const {
        return *reinterpret_cast<const elf64_header*>(m_buf.data());
    }

    span<const elf64_program_header> program_headers() const {
        auto header_buf = m_buf.slice(header().pheader_offset,
                                      header().pheader_size * header().pheader_num);
        auto ptr = reinterpret_cast<const elf64_program_header*>(header_buf.data());
        return {ptr, header().pheader_num};
    }

    span<const uint8_t> segment(const elf64_program_header& header) const {
        return m_buf.slice(header.file_offset, header.file_size);
    }

private:
    span<const uint8_t> m_buf;
};
} // namespace tos::elf