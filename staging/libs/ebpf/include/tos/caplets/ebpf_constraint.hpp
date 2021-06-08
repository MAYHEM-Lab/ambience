#pragma once

#include <ebpf_constraint_generated.hpp>
#include <tos/caplets.hpp>

namespace tos::ebpf {
bool verify(const eBPFConstraint& ctr);

template<class TokenT>
bool verify(const eBPFConstraint& ctr,
            const caplets::common_validation_context<TokenT>&) {
    return verify(ctr);
}
}