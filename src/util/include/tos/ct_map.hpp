//
// Created by fatih on 6/8/18.
//

#pragma once

#include "type_traits.hpp"

template <class T> struct ct { using type = T; };

template <class KeyT, class ValT>
struct el_t
{
    ValT val;
    template <class ValU>
    constexpr el_t(ValU&& v) : val(tos::forward<ValU>(v)) {}
};

template <class KeyT, class ValT>
struct el_t<KeyT, ValT&>
{
    ValT& val;

    template <class ValU>
    constexpr el_t(ValU& v) : val(v) {}
};

struct base_key_policy
{
    template <class KeyT, class ValT>
    static constexpr tos::true_type validate(ct<KeyT>, ct<ValT>) { return {}; }
};

template <class KeyPolicy = base_key_policy, class...> struct ct_map;
template <class KeyPolicy, class... KeyT, class... ValT>
struct ct_map<KeyPolicy, el_t<KeyT, ValT>...> :
        el_t<KeyT, ValT>...
{
    template <class... ValU>
    constexpr explicit ct_map(ValU&&... vals) :
            el_t<KeyT, ValT>{tos::forward<ValU>(vals)}... {}

    template <class NewValT>
    constexpr ct_map<KeyPolicy, el_t<KeyT, ValT>..., el_t<NewValT, NewValT>>
    add(NewValT&& new_val) &&
    {
        static_assert(KeyPolicy::validate(ct<NewValT>{}, ct<NewValT>{}), "");
        return ct_map<KeyPolicy, el_t<KeyT, ValT>..., el_t<NewValT, NewValT>>
                { tos::move(((ValT&)(el_t<KeyT, ValT>&)*this))..., tos::forward<NewValT>(new_val) };
    }

    template <class NewKeyT, class NewValT>
    constexpr ct_map<KeyPolicy, el_t<KeyT, ValT>..., el_t<NewKeyT, NewValT>>
    add(NewValT&& new_val) &&
    {
        static_assert(KeyPolicy::validate(ct<NewKeyT>{}, ct<NewValT>{}), "");
        return ct_map<KeyPolicy, el_t<KeyT, ValT>..., el_t<NewKeyT, NewValT>>
                { tos::move(((ValT&)(el_t<KeyT, ValT>&)*this))..., tos::forward<NewValT>(new_val) };
    }

    template <class NewKeyT, class NewValT>
    constexpr ct_map<KeyPolicy, el_t<KeyT, ValT>..., el_t<NewKeyT, NewValT>>
    add(ct<NewKeyT>, NewValT&& new_val) &&
    {
        static_assert(KeyPolicy::validate(ct<NewKeyT>{}, ct<NewValT>{}), "");
        return ct_map<KeyPolicy, el_t<KeyT, ValT>..., el_t<NewKeyT, NewValT>>
                { tos::move(((ValT&)(el_t<KeyT, ValT>&)*this))..., tos::forward<NewValT>(new_val) };
    }

    template <class... ReqKeyTs, class... ReqValTs>
    constexpr operator ct_map<KeyPolicy, el_t<ReqKeyTs, ReqValTs&>...>() const;
};

template <class KeyT, class ValT>
constexpr const ValT& get(const el_t<KeyT, ValT>& x)
{
    return x.val;
}

template <class KeyT, class ValT>
constexpr const ValT& get(const el_t<KeyT, ValT&>& x)
{
    return x.val;
}

template <class KeyT, class ValT>
constexpr ValT& get(el_t<KeyT, ValT>& x)
{
    return x.val;
}

template <class KeyT, class ValT>
constexpr ValT& get(el_t<KeyT, ValT&>& x)
{
    return x.val;
}

template <class KeyT, class ValT, class OrT>
constexpr const ValT& get_or(OrT&, const el_t<KeyT, ValT>& x)
{
    return x.val;
}

template <class KeyT, class ValT, class OrT>
constexpr const ValT& get_or(OrT&, const el_t<KeyT, ValT&>& x)
{
    return x.val;
}

template <class KeyT, class ValT, class OrT>
constexpr ValT& get_or(OrT&, el_t<KeyT, ValT>& x)
{
    return x.val;
}

template <class KeyT, class ValT, class OrT>
constexpr ValT& get_or(OrT&, el_t<KeyT, ValT&>& x)
{
    return x.val;
}

template <class KeyT, class OrT>
constexpr OrT& get_or(OrT& v, ...)
{
    return v;
}

template <class KeyPolicy, class... KeyT, class... ValT>
template <class... ReqKeyTs, class... ReqValTs>
constexpr ct_map<KeyPolicy, el_t<KeyT, ValT>...>::operator ct_map<KeyPolicy, el_t<ReqKeyTs, ReqValTs&>...>() const
{
    return ct_map<KeyPolicy, el_t<ReqKeyTs, ReqValTs&>...>{ get<ReqKeyTs>(*this)... };
}

constexpr ct_map<> make_map() { return ct_map<>{}; }