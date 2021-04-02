#include <calc_generated.hpp>
#include <tos/ae/user_space.hpp>
#include <tos/allocator/free_list.hpp>
#include <tos/task.hpp>

[[gnu::section(".nozero")]] uint8_t heap[2048];
tos::memory::free_list alloc{heap};

[[gnu::noinline]] void* operator new(size_t sz) {
    auto ptr = alloc.allocate(sz);
    if (ptr == nullptr) {
        //TODO: handle this via a panic
    }
    return ptr;
}

[[gnu::noinline]] void operator delete(void* pt) {
    alloc.free(pt);
}


template<class>
struct convert_types;

template<class... Ts>
struct convert_types<lidl::meta::list<Ts...>> {
    using type = lidl::meta::list<decltype(&std::declval<Ts&>())...>;
    using tuple_type = std::tuple<decltype(&std::declval<Ts&>())...>;
};

using zerocopy_fn_t = void (*)(lidl::service_base&, const void*, void*);

template<class ServiceT, int ProcId>
constexpr auto zerocopy_translator() -> zerocopy_fn_t {
    return [](lidl::service_base& serv_base, const void* args, void* ret) {
        auto& serv = static_cast<ServiceT&>(serv_base);
        using ServDesc = lidl::service_descriptor<ServiceT>;
        constexpr auto& proc_desc = std::get<ProcId>(ServDesc::procedures);
        using ProcTraits = lidl::procedure_traits<decltype(proc_desc.function)>;
        using ArgsTupleType =
            typename convert_types<typename ProcTraits::param_types>::tuple_type;
        using RetType = typename ProcTraits::return_type;
        static constexpr bool is_ref = std::is_reference_v<RetType>;
        using ActualRetType =
            std::conditional_t<is_ref,
                               std::add_pointer_t<std::remove_reference_t<RetType>>,
                               RetType>;

        auto do_call = [&serv, ret](auto*... vals) {
            //            LOG(*vals...);
            constexpr auto& fn = proc_desc.function;
            if constexpr (is_ref) {
                auto& res = (serv.*fn)(*vals...);
                new (ret) ActualRetType(&res);
            } else {
                new (ret) ActualRetType((serv.*fn)(*vals...));
            }
        };

        auto& args_tuple = *static_cast<const ArgsTupleType*>(args);
        std::apply(do_call, args_tuple);
    };
}

template<class ServiceT, size_t... Is>
constexpr zerocopy_fn_t vt[] = {zerocopy_translator<ServiceT, Is>()...};

template<class ServiceT, size_t... Is>
constexpr tos::span<const zerocopy_fn_t>
do_make_zerocopy_vtable(std::index_sequence<Is...>) {
    return tos::span<const zerocopy_fn_t>(vt<ServiceT, Is...>);
}

template<class ServiceT>
constexpr tos::span<const zerocopy_fn_t> make_zerocopy_vtable() {
    using ServDesc = lidl::service_descriptor<ServiceT>;
    return do_make_zerocopy_vtable<ServiceT>(
        std::make_index_sequence<std::tuple_size_v<decltype(ServDesc::procedures)>>{});
}

struct service_info {
    template<class ServiceT>
    explicit service_info(ServiceT* serv)
        : impl{serv}
        , vtable{make_zerocopy_vtable<ServiceT>()} {
    }

    bool run_proc(int proc, const void* arg, void* res) const {
        vtable[proc](*impl, arg, res);
        return true;
    }

    lidl::service_base* impl;
    tos::span<const zerocopy_fn_t> vtable;
};

template<size_t Count>
struct group {
    template<class... Services>
    static group make(Services*... servs) {
        return group(servs...);
    }

    bool run_proc(int channel, int proc, const void* arg, void* res) const {
        auto& serv = m_services[channel];
        return serv.run_proc(proc, arg, res);
    }

private:
    template<class... Services>
    explicit group(Services*... servs)
        : m_services{service_info(servs)...} {
    }

    std::array<service_info, Count> m_services;
};

template<size_t N>
bool run_req(const group<N>& grp, const tos::ae::req_elem& el) {
    return grp.run_proc(el.channel, el.procid, el.arg_ptr, el.ret_ptr);
}
group<1>* g;

tos::Task<bool> handle_req(tos::ae::req_elem el) {
    co_return run_req(*g, el);
}

tos::Task<tos::ae::service::calculator*> init_basic_calc();
tos::Task<void> task() {
    auto basic_calc = co_await init_basic_calc();
    basic_calc->add(4, 5);

    auto g = group<1>::make(basic_calc);
    ::g = &g;

    while (true) {
        co_await tos::ae::log_str("Hello world from user space!");
        co_await tos::ae::log_str("Second hello world from user space!");
    }
}