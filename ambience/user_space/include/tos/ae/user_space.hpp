#pragma once

#include <cstdint>
#include <string_view>
#include <tos/ae/rings.hpp>
#include <tos/flags.hpp>
#include <tos/span.hpp>
#include <tos/self_pointing.hpp>
#include <tos/allocator/polymorphic_allocator.hpp>

namespace tos::ae {
void done_callback(void* ptr, uintptr_t status);
void proc_res_queue(interface& iface);

struct low_level_output_t : self_pointing<low_level_output_t> {
    int write(tos::span<const uint8_t> data);
};
inline low_level_output_t low_level_output;

uint64_t timestamp();

tos::memory::polymorphic_allocator& default_allocator();
} // namespace tos::ae

void dispatch_request(const tos::ae::req_elem& el);
