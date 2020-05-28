#include <string_view>
#include <tos/build.hpp>

namespace tos::build {
std::string_view commit_hash() {
    return TOS_GIT_SHA1;
}

std::string_view platform() {
    return TOS_CONFIG_PLATFORM;
}

std::string_view arch() {
    return TOS_CONFIG_ARCH;
}

std::string_view drivers() {
    return TOS_CONFIG_DRIVERS;
}
}