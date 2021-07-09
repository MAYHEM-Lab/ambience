#pragma once

#include <tos/ae/service_host.hpp>

namespace tos::ae {
struct import_args {};

template<class ServiceT>
struct importer {
    virtual ServiceT* import_service(const import_args&) = 0;
    virtual ~importer() = default;
};
} // namespace tos::ae