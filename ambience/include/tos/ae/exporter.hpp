#pragma once

#include <tos/ae/service_host.hpp>
#include <import_export_generated.hpp>

namespace tos::ae {
struct export_args {};

struct service_exporter : exporter::sync_server {
    virtual void export_service(const sync_service_host&, const export_args&) = 0;
    virtual void export_service(const async_service_host&, const export_args&) = 0;

    virtual ~service_exporter() = default;
};
}