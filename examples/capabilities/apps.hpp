//
// Created by fatih on 3/9/19.
//

#include <array>
#include <caps/caps.hpp>
#include "common.hpp"
#include <caps/raw_serdes.hpp>
#include <caps/request.hpp>

caps::emsha::signer signer("sekret");

template <class StreamT>
void source_task(StreamT& str)
{
    std::array<char, 32> buf;

    auto r = str->read(buf);

    if (!r) return;
    auto p = force_get(r);

    auto path = authn::path_t{};

    strncpy(path.path, p.data(), p.size());

    auto cps = caps::mkcaps({
                                authn::cap_t(path, authn::rights::full)
                        }, signer);

    caps::serialize(str, *cps);
}

auto proc = caps::req_deserializer<authn::cap_t>(signer, authn::parse_req, authn::satisfies);

template <class StreamT>
static void sink_task(StreamT& str)
{
    std::array<char, 72> buf;

    auto r = str->read(buf);

    if (!r) return;

    auto rr = proc(force_get(r));

    if (!rr)
    {
        auto err = force_error(rr);
        switch (err)
        {
            case caps::deser_errors::not_array:
            case caps::deser_errors::no_req:
            case caps::deser_errors::no_seq:
            case caps::deser_errors::no_token:
                tos::println(str, "bad req format", int(err));
                return;
            case caps::deser_errors::bad_seq:
                tos::println(str, "replay!");
                return;
            case caps::deser_errors::bad_sign:
                tos::println(str, "bad signature");
                return;
            case caps::deser_errors::bad_cap:
                tos::println(str, "unauthorized!");
                return;
        }
        TOS_UNREACHABLE();
    }

    auto request = force_get(rr);

    tos::println(str, "ok:", request.p.path);
}
