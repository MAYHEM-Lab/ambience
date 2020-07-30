#pragma once

#include <cstdint>
#include <tos/print.hpp>
#include <tos/span.hpp>

namespace tos::gnu {
struct elf_note_section {
public:
    tos::span<const char> name() const {
        return tos::span<const char>(reinterpret_cast<const char*>(&data[0]), namesz - 1);
    }

    tos::span<const uint8_t> id() const {
        return tos::span<const uint8_t>(&data[namesz], descsz);
    }

private:
    uint32_t namesz;
    uint32_t descsz;
    uint32_t type;
    uint8_t data[];
};

template<class StreamT>
void print(StreamT& stream, const elf_note_section& version) {
    tos::print(stream, "Build ID:", version.name(), "");
    for (uint8_t byte : version.id()) {
        tos::print(stream, uintptr_t(byte));
    }
}
} // namespace tos::gnu

extern "C" {
extern const tos::gnu::elf_note_section build_id;
}
