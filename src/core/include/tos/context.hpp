#pragma once

#include <cstdint>
#include <tos/components/component.hpp>
#include <tos/intrusive_list.hpp>
#include <tuple>
#include <type_traits>

namespace tos {
struct context : list_node<context> {
    template<class ComponentT>
    ComponentT* get_component();

    virtual ~context() = default;

private:
    virtual component* get_component_with_id(component_id_t) = 0;
};

template<class... Components>
class static_context : public context {
public:
    auto& all_components() {
        return m_components;
    }
private:
    std::tuple<Components...> m_components;
    component* get_component_with_id([[maybe_unused]] component_id_t id) override;
};

namespace global {
/**
 * All threads belong to this context by default.
 */
inline static_context<> default_context;
}
} // namespace tos

// impl

namespace tos {
template<class ComponentT>
ComponentT* context::get_component() {
    return static_cast<ComponentT*>(get_component_with_id(component_id<ComponentT>()));
}

namespace detail {
template<class T>
auto idof(T&) -> std::integral_constant<component_id_t, std::remove_reference_t<T>::id>;
}
template<class... Components>
component* static_context<Components...>::get_component_with_id(component_id_t id) {
#define CTX_CASE(i)                                                                      \
    case decltype(detail::idof(std::get<i>(m_components)))::value:                         \
        return &std::get<i>(m_components);

    if constexpr (sizeof...(Components) == 1) {
        switch (id) { CTX_CASE(0) }
    } else if constexpr (sizeof...(Components) == 2) {
        switch (id) {
            CTX_CASE(0)
            CTX_CASE(1)
        }
    } else if constexpr (sizeof...(Components) == 3) {
        switch (id) {
            CTX_CASE(0)
            CTX_CASE(1)
            CTX_CASE(2)
        }
    } else if constexpr (sizeof...(Components) == 4) {
        switch (id) {
            CTX_CASE(0)
            CTX_CASE(1)
            CTX_CASE(2)
            CTX_CASE(3)
        }
    } else if constexpr (sizeof...(Components) == 5) {
        switch (id) {
            CTX_CASE(0)
            CTX_CASE(1)
            CTX_CASE(2)
            CTX_CASE(3)
            CTX_CASE(4)
        }
    } else if constexpr (sizeof...(Components) == 6) {
        switch (id) {
            CTX_CASE(0)
            CTX_CASE(1)
            CTX_CASE(2)
            CTX_CASE(3)
            CTX_CASE(4)
            CTX_CASE(5)
        }
    }
    static_assert(sizeof...(Components) <= 6,
                  "A static context may not have more than 6 components!");
#undef CTX_CASE
    return nullptr;
}
} // namespace tos