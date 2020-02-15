//
// Created by fatih on 10/5/18.
//

#pragma once

#include <cstdint>

namespace tos {
/**
 * This struct represents a unique identifier for every thread
 * in the system.
 * 
 * The identifiers may be reused throughout the lifetime of the
 * system, but no two active threads will share an identifier.
 */
struct thread_id_t {
    uintptr_t id;
};

inline bool operator==(const thread_id_t& left, const thread_id_t& right) {
    return left.id == right.id;
}

inline bool operator<(const thread_id_t& left, const thread_id_t& right) {
    return left.id < right.id;
}
} // namespace tos
