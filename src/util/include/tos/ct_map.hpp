//
// Created by fatih on 6/8/18.
//

#pragma once

#include "type_traits.hpp"

namespace tos {
    template<class T>
    struct ct
    {
        using type = T;
    };

    template<class KeyT, class ValT>
    struct el_t
    {
        ValT val;

        template<class ValU>
        constexpr el_t(ValU&& v)
                : val(std::forward<ValU>(v))
        { }
    };

    template<class KeyT, class ValT>
    struct el_t<KeyT, ValT&>
    {
        ValT& val;

        constexpr el_t(ValT& v)
                :val(v)
        { }
    };

    template<class KeyPolicy, class...>
    struct ct_map;

    template<class KeyPolicy, class... KeyT, class... ValT>
    struct ct_map<KeyPolicy, el_t<KeyT, ValT>...> :
            el_t<KeyT, ValT> ...
    {
        template<class... ValU>
        constexpr explicit ct_map(ValU&& ... vals)
                :
                el_t<KeyT, ValT>{std::forward<ValU>(vals)}...
        { }

        template<class NewValT>
        constexpr auto
        add(NewValT&& new_val)&&
        {
            using clear_t = std::remove_const_t<std::remove_reference_t<NewValT>>;
            static_assert(KeyPolicy::validate(ct<clear_t>{}, ct<clear_t>{}), "");
            return ct_map<KeyPolicy, el_t<KeyT, ValT>..., el_t<clear_t, clear_t>>
                    {std::move(((el_t<KeyT, ValT>&) *this).val)..., std::forward<NewValT>(new_val)};
        }

        template<class NewKeyT, class NewValT>
        constexpr auto
        add(NewValT&& new_val)&&
        {
            using clear_t = std::remove_const_t<std::remove_reference_t<NewValT>>;
            static_assert(KeyPolicy::validate(ct<NewKeyT>{}, ct<clear_t>{}), "");
            return ct_map<KeyPolicy, el_t<KeyT, ValT>..., el_t<NewKeyT, clear_t>>
                    {std::move(((el_t<KeyT, ValT>&) *this).val)..., std::forward<NewValT>(new_val)};
        }

        template<class NewKeyT, class NewValT>
        constexpr auto
        add(ct<NewKeyT>, NewValT&& new_val)&&
        {
            using clear_t = std::remove_const_t<std::remove_reference_t<NewValT>>;
            static_assert(KeyPolicy::validate(ct<NewKeyT>{}, ct<clear_t>{}), "");
            return ct_map<KeyPolicy, el_t<KeyT, ValT>..., el_t<NewKeyT, clear_t>>
                    {std::move(((el_t<KeyT, ValT>&) *this).val)..., std::forward<NewValT>(new_val)};
        }

        template<class... ReqKeyTs, class... ReqValTs>
        constexpr operator ct_map<KeyPolicy, el_t<ReqKeyTs, ReqValTs&>...>() const;
    };

    template<class KeyT, class ValT>
    constexpr const ValT& get(const el_t<KeyT, ValT>& x)
    {
        return x.val;
    }

    template<class KeyT, class ValT>
    constexpr const ValT& get(const el_t<KeyT, ValT&>& x)
    {
        return x.val;
    }

    template<class KeyT, class ValT>
    constexpr ValT& get(el_t<KeyT, ValT>& x)
    {
        return x.val;
    }

    template<class KeyT, class ValT>
    constexpr ValT& get(el_t<KeyT, ValT&>& x)
    {
        return x.val;
    }

    template<class KeyT, class ValT, class OrT>
    constexpr const ValT& get_or(OrT&, const el_t<KeyT, ValT>& x)
    {
        return x.val;
    }

    template<class KeyT, class ValT, class OrT>
    constexpr const ValT& get_or(OrT&, const el_t<KeyT, ValT&>& x)
    {
        return x.val;
    }

    template<class KeyT, class ValT, class OrT>
    constexpr ValT& get_or(OrT&, el_t<KeyT, ValT>& x)
    {
        return x.val;
    }

    template<class KeyT, class ValT, class OrT>
    constexpr ValT& get_or(OrT&, el_t<KeyT, ValT&>& x)
    {
        return x.val;
    }

    template<class KeyT, class OrT>
    constexpr OrT& get_or(OrT& v, ...)
    {
        return v;
    }

    template<class KeyPolicy, class... KeyT, class... ValT>
    template<class... ReqKeyTs, class... ReqValTs>
    constexpr ct_map<KeyPolicy, el_t<KeyT, ValT>...>::operator ct_map<KeyPolicy, el_t<ReqKeyTs, ReqValTs&>...>() const
    {
        return ct_map<KeyPolicy, el_t<ReqKeyTs, ReqValTs&>...>{get<ReqKeyTs>(*this)...};
    }

    struct base_key_policy
    {
        template<class KeyT, class ValT>
        static constexpr std::true_type validate(ct<KeyT>, ct<ValT>)
        { return {}; }
    };

    constexpr ct_map<base_key_policy> make_map()
    { return ct_map<base_key_policy>{}; }
}
