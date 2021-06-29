#pragma once

namespace tos::ae::kernel {
template<class GroupDescr, class PlatformArgs>
auto load_it(const PlatformArgs& args) {
    using loader = typename GroupDescr::loader;
    auto res = loader::load(GroupDescr{}, args);
    return res;
}
} // namespace tos::ae::kernel