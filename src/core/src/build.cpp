#include <tos/build.hpp>

namespace tos::build {
span<const uint8_t> commit_hash() {
    return raw_cast(span<const char>(TOS_GIT_SHA1));
}
}