#pragma once

// clang-format off
{% for include in exporter_includes %}
#include <{{include}}>
{% endfor %}
// clang-format on

{% for exporter in exporters %}
tos::Task<{{exporters[exporter]}}*> get_{{exporter}}();
{% endfor %}
