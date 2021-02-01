#include <tos/elf.hpp>
#include <tos/elf/common.hpp>
#include <tos/elf/header.hpp>
#include <tos/elf/magic.hpp>

namespace tos::elf {
elf64::elf64(span<const uint8_t> buffer)
    : m_buf{buffer} {
}

expected<elf64, errors> elf64::from_buffer(span<const uint8_t> buffer) {
    if (buffer.size() < sizeof(elf64_header)) {
        return unexpected(errors::too_short);
    }
    if (tos::span(magic) != buffer.slice(0, 4)) {
        return unexpected(errors::bad_magic);
    }
    auto header = reinterpret_cast<const elf64_header*>(buffer.data());

    if (header->class_ != elf_class::class64) {
        return unexpected(errors::bad_class);
    }

    if (header->enc != endian::little) {
        return unexpected(errors::bad_endian);
    }

    if (header->os_abi == abi::hpux) {
        return unexpected(errors::bad_abi);
    }

    return elf64{buffer};
}
} // namespace tos::elf