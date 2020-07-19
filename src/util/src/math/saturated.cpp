#include <tos/math/saturated.hpp>
#include <limits>

namespace tos::math {
namespace {
template <class T>
T add_saturating_naive(const T& a, const T& b) {
    // https://stackoverflow.com/a/17587197/778117

    T limit = a < 0 ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max();

    T comp = limit - a;

    if ((a < 0) == (b > comp)) {
        return a + b;
    }

    return limit;
}
}
}