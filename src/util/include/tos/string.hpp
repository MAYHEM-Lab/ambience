#pragma once

#include <vector>
#include <string_view>

namespace tos {
template <class OutIt>
void split(std::string_view sv, std::string_view delim, OutIt it) {
    while (true) {
        if (sv.empty()) break;
        auto i = sv.find(delim);
        *it++ = sv.substr(0, i);
        if (i == sv.npos) {
            break;
        }
        sv = sv.substr(i + delim.size());
    }
}

std::vector<std::string_view> split(std::string_view sv, std::string_view delim) {
    std::vector<std::string_view> splitted;
    split(sv, delim, std::back_inserter(splitted));
    return splitted;
}
}