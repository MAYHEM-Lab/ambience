#pragma once

#include <tos/ae/service_host.hpp>

namespace tos::ae {
struct export_args {};

struct exporter {
    virtual void export_service(const sync_service_host&, const export_args&) = 0;
    virtual void export_service(const async_service_host&, const export_args&) = 0;

    virtual ~exporter() = default;
};
}