#pragma once

#include <cstdint>
#include <tos/compiler.hpp>

namespace tos::mmio {
enum class permissions
{
    read = 1,
    write = 2,
    rw = read | write
};

template<uintptr_t Address, class Type = uintptr_t>
struct reg {
public:
    [[nodiscard]] ALWAYS_INLINE Type read() const {
        return *get();
    }

    ALWAYS_INLINE
    const reg& write(const Type& val) const {
        *get() = val;
        return *this;
    }

private:
    [[nodiscard]] ALWAYS_INLINE volatile Type* get() const {
        return reinterpret_cast<volatile Type*>(Address);
    }
};
} // namespace tos::mmio
