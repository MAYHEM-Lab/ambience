//
// Created by fatih on 6/8/18.
//

/**
 * ct_map implements a mapping from compile time types to
 * runtime values.
 *
 * In TOS, it's commonly used to implement builder pattern
 * parameters that are passed to various functions.
 *
 * Every map type has a key policy which validates the types
 * of the keys and values inserted into the map.
 *
 * Instances of this class template cannot be copied, but
 * they can implicitly convert into another map of the same
 * keys and of values where the values are references into
 * the original map.
 *
 *     struct baud_rate_tag {};
 *     struct parity_tag {};
 *     auto map = make_map()
 *                .add<baud_rate_tag>(115200)
 *                .add<parity_tag>(usart_parity::odd);
 *     auto usart = open(devs::usart, map);
 *
 * The above snippet creates a map object that will return
 * (int)115200 when looked up with `baud_rate_tag` and
 * usart_parity::odd when looked up with `parity_tag`.
 *
 * Depending on the handler of the open call, compilation
 * may fail since it requires additional parameters.
 *
 * The advantage of this pattern is that it allows for
 * multi-step initialization of the map object.
 *
 * One important note is that the `add` functions can only
 * be called on r-values as the run time values are moved
 * into the newly created map upon every add. This means
 * we can't modify an existing map object:
 *
 *     map.add<foo_type>(42); // won't compile
 *     auto map2 = map.add<foo_type>(42); // won't compile
 *
 * However, it's possible to _move_ from an existing map
 * and add to that:
 *
 *     auto map3 = std::move(map)
 *                 .add<foo_type>(42);
 *
 * As usual with all moved from objects, `map` is left in
 * an indeterminate state, where reading from it will
 * return the respective moved from object of the value
 * type.
 *
 * The error messages generated due to a non-existent key or
 * conversion error is, unfortunately, unwieldy due to the
 * heavy template usage. This needs some work.
 */

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
