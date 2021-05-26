#pragma once

#include <string_view>
#include <type_traits>

namespace tos {
template <class T>
concept ObjectDumper = requires (T& t) {
    t.field(std::declval<std::string_view>());
};

template <class T>
concept DumpTarget = requires (T& t) {
    { t.object(std::declval<std::string_view>()) } -> ObjectDumper;
};

template <class T, class Target>
concept Dumpable = requires (T& t, Target& target) {
    dump(target, t);
};
}