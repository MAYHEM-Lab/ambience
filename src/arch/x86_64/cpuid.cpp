#include <cpuid.h>
#include <tos/x86_64/cpuid.hpp>

namespace tos::x86_64::cpuid {
std::array<char, 12> manufacturer() {
    uint32_t cpuid_data[4];
    __get_cpuid(0, &cpuid_data[0], &cpuid_data[1], &cpuid_data[2], &cpuid_data[3]);

    std::array<char, 12> manufacturer_name;
    memcpy(&manufacturer_name[0], &cpuid_data[1], 4);
    memcpy(&manufacturer_name[4], &cpuid_data[3], 4);
    memcpy(&manufacturer_name[8], &cpuid_data[2], 4);

    return manufacturer_name;
}
} // namespace tos::x86_64::cpuid