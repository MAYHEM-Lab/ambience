#pragma once

#include <boost/hana/tuple.hpp>
#include <tos/ae/kernel/platform_support.hpp>

// clang-format off
{% for include in group_includes %}
#include <{{include}}>
{% endfor %}
// clang-format on

using groups_t = boost::hana::tuple<{{groups | join(", ")}}>;

groups_t init_all_groups(const platform_group_args& platform_args);