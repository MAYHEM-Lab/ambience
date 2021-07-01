#include "groups.hpp"

#include <boost/hana/for_each.hpp>
#include <tos/detail/poll.hpp>

namespace hana = boost::hana;
using namespace hana::literals;

groups_t init_all_groups(const platform_group_args& platform_args) {
    // clang-format off
    auto res = hana::make_tuple(
    {% for group in groups %}
        init_{{group}}(platform_args)
        {% if not loop.last %}
        ,
        {% endif %}
    {% endfor %}
    );
    // clang-format on

    boost::hana::for_each(
        res, [](auto& g) { tos::coro::make_detached(g.post_load()); });

    return res;
}
