{% for include in service_includes %}
#include <{{include}}>
{% endfor %}
#include <tos/ae/group.hpp>
#include <tos/ae/transport/upcall.hpp>
#include <tos/task.hpp>

extern tos::ae::interface iface;
auto transport = tos::ae::upcall_transport<&iface>{};

tos::ae::group<{{group.servs|length}}>* g;

tos::Task<bool> handle_req(tos::ae::req_elem el) {
    return tos::ae::run_req(*g, el);
}

{{service_init_sigs}}
tos::Task<void> task() {
    {{external_deps}}
    {% for init in service_inits %}
    {{init}}
    {% endfor %}
    {{group_init}}
}
