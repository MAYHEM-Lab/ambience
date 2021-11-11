#include "tos/memory.hpp"
#include <tos/ae/kernel/loaders/preemptive_elf_group.hpp>
#include <tos/ae/kernel/runners/preemptive_user_runner.hpp>
#include <tos/ae/kernel/start_group.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/backing_object.hpp>
#include <tos/elf.hpp>
void dump_table(tos::cur_arch::translation_table& table);

namespace tos::ae::kernel {
namespace {
tos::expected<void, cur_arch::mmu_errors>
map_elf(const tos::elf::elf64& elf,
        tos::physical_page_allocator& palloc,
        tos::cur_arch::translation_table& root_table) {
    for (auto pheader : elf.program_headers()) {
        if (pheader.type != tos::elf::segment_type::load) {
            continue;
        }

        tos::debug::log("Load",
                        (void*)(uintptr_t)pheader.file_size,
                        "bytes from",
                        (void*)(uintptr_t)pheader.file_offset,
                        "to",
                        (void*)(uintptr_t)pheader.virt_address);

        auto seg = elf.segment(pheader);

        void* base = const_cast<uint8_t*>(seg.data());
        if (pheader.file_size < pheader.virt_size) {
            auto pages = pheader.virt_size / tos::cur_arch::page_size_bytes;
            auto p = palloc.allocate(pages);
            base = palloc.address_of(*p).direct_mapped();
        }

        LOG((void*)pheader.virt_address,
            pheader.virt_size,
            (void*)pheader.file_offset,
            pheader.file_size,
            base);

        auto vseg = pheader.virtual_segment();

        EXPECTED_TRYV(map_region(root_table,
                                 vseg,
                                 tos::user_accessible::yes,
                                 tos::memory_types::normal,
                                 &palloc,
                                 physical_address{reinterpret_cast<uintptr_t>(base)}));

        // The pages are mapped into the kernel as RW
        //        vseg.perms = permissions::read_write;
        EXPECTED_TRYV(map_region(x86_64::get_current_translation_table(),
                                 vseg,
                                 tos::user_accessible::no,
                                 tos::memory_types::normal,
                                 &palloc,
                                 physical_address{reinterpret_cast<uintptr_t>(base)}));
    }

    return {};
}

tos::expected<tos::span<uint8_t>, cur_arch::mmu_errors>
create_and_map_stack(size_t stack_size,
                     tos::physical_page_allocator& palloc,
                     tos::cur_arch::translation_table& root_table) {
    stack_size = tos::align_nearest_up_pow2(stack_size, tos::cur_arch::page_size_bytes);
    auto page_count = stack_size / tos::cur_arch::page_size_bytes;
    auto stack_pages = palloc.allocate(page_count);

    if (!stack_pages) {
        return tos::unexpected(cur_arch::mmu_errors::page_alloc_fail);
    }

    auto stack_address = palloc.address_of(*stack_pages);
    tos::debug::log("Stack at", stack_address.direct_mapped());

    EXPECTED_TRYV(map_region(
        root_table,
        identity_map(tos::physical_segment{
            .range = {.base = stack_address, .size = static_cast<ptrdiff_t>(stack_size)},
            tos::permissions::read_write}),
        tos::user_accessible::yes,
        tos::memory_types::normal,
        &palloc,
        stack_address));

    EXPECTED_TRYV(map_region(
        x86_64::get_current_translation_table(),
        identity_map(tos::physical_segment{
            .range = {.base = stack_address, .size = static_cast<ptrdiff_t>(stack_size)},
            tos::permissions::read_write}),
        tos::user_accessible::no,
        tos::memory_types::normal,
        &palloc,
        stack_address));

    return tos::span<uint8_t>(static_cast<uint8_t*>(stack_address.direct_mapped()),
                              stack_size);
}
} // namespace

std::unique_ptr<kernel::user_group>
preemptive_elf_group::do_load(span<const uint8_t> elf_body,
                              interrupt_trampoline& trampoline,
                              physical_page_allocator& palloc,
                              cur_arch::translation_table& root_table,
                              std::string_view name) {
    auto elf_res = tos::elf::elf64::from_buffer(elf_body);
    if (!elf_res) {
        LOG_ERROR("Could not parse payload!");
        LOG_ERROR("Error code: ", int(force_error(elf_res)));
        return nullptr;
    }

    auto root_as = tos::global::cur_as;
    auto our_as = root_as->clone(palloc);
    if (!our_as) {
        LOG_ERROR("Could not clone address space!");
        return nullptr;
    }

    auto& elf = force_get(elf_res);
    if (auto map_res = map_elf(elf, palloc, *force_get(our_as)->m_table); !map_res) {
        auto& err = force_error(map_res);
        tos::debug::error("Could not map ELF!", int(err));
        return nullptr;
    }

    auto stack_res = create_and_map_stack(
        4 * tos::cur_arch::page_size_bytes, palloc, *force_get(our_as)->m_table);

    if (!stack_res) {
        tos::debug::error("Could not create and map stack!");
        return nullptr;
    }

    auto stack = force_get(stack_res);

    //    dump_table(*force_get(our_as)->m_table);

    tos::debug::log(tos::global::cur_as, force_get(our_as).get());
    auto res = start_group(stack,
                           reinterpret_cast<void (*)()>(elf.header().entry),
                           trampoline,
                           name,
                           *force_get(our_as));
    tos::debug::log(tos::global::cur_as, force_get(our_as).get());

    if (res) {
        res->runner = &preemptive_user_group_runner::instance();
        res->as = std::move(force_get(our_as));
    }

    return res;
}
} // namespace tos::ae::kernel