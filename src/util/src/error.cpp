#include <errno.h>
#include <tos/error.hpp>
#include <tos/expected.hpp>
#include <tos/result.hpp>

namespace tos {
struct errno_err {
    int m_errno;

    constexpr std::string_view name() const {
        return "int_err";
    }

    constexpr std::string_view message() const {
        switch (m_errno) {
        case EADDRINUSE:
            return "EADDRINUSE";
        }
        return "EUNKNOWN";
    }
};

namespace {
result<int> failingfn(bool work) {
    if (work) {
        return 42;
    } else {
        return errno_err{42};
    }
}
} // namespace
} // namespace tos