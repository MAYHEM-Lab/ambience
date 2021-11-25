{% for include in service_includes %}
#include <{{include}}>
{% endfor %}
#include <tos/ae/group.hpp>
#include <tos/ae/transport/upcall.hpp>
#include <tos/task.hpp>

extern tos::ae::interface iface;
namespace {
constexpr auto transport = tos::ae::upcall_transport<&iface>{};
tos::ae::group<{{group.servs|length}}>* g;
}

void dispatch_request(const tos::ae::req_elem& el) {
    g->run_req(el);
}

{{service_init_sigs}}
tos::Task<void> task() {
    {{external_deps}}
    {% for init in service_inits %}
    {{init}}
    {% endfor %}
    {{group_init}}
}
