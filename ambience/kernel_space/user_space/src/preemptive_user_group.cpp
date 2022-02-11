#include "tos/address_space.hpp"
#include "tos/debug/log.hpp"
#include "tos/error.hpp"
#include "tos/expected.hpp"
#include "tos/flags.hpp"
#include "tos/mapping.hpp"
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
struct elf_file_backing : tos::backing_object {
    elf_file_backing(elf::elf64 file)
        : elf_file{file} {
    }
    elf::elf64 elf_file;

    auto create_mapping(const virtual_segment& vm_segment,
                        uintptr_t obj_base,
                        tos::mapping& mapping) -> result<void> override {
        mapping.obj = this;
        mapping.vm_segment = vm_segment;
        mapping.obj_base = obj_base;
        mapping.mem_type = memory_types::normal;

        return {};
    }

    auto free_mapping(const tos::mapping& mapping) -> result<void> override {
        return {};
    }

    auto handle_memory_fault(const memory_fault& fault) -> result<void> override {
        for (auto pheader : elf_file.program_headers()) {
            if (contains(pheader.virtual_range(), fault.virt_addr)) {
                LOG("Found the segment", fault.virt_addr, pheader.virtual_range());

                auto base = elf_file.segment(pheader).data();

                auto range = fault.map->va->containing_fragment(
                    {fault.virt_addr, static_cast<int>(fault.access_size)});

                LOG(fault.map->va,
                    fault.map->va->m_table,
                    (void*)pheader.virt_address,
                    pheader.virt_size,
                    (void*)pheader.file_offset,
                    pheader.file_size,
                    base);

                EXPECTED_TRYV(fault.map->va->mark_resident(
                    *fault.map,
                    range,
                    physical_address{reinterpret_cast<uintptr_t>(base) +
                                     (range.base - fault.map->vm_segment.range.base)}));

                LOG("OK");
                return {};
            }
        }

        return unexpected(address_space_errors::no_mapping);
    }
};

enum class elf_errors
{
    write_execute_segment,
};

TOS_ERROR_ENUM(elf_errors);

result<void> map_elf(const tos::elf::elf64& elf,
                     tos::physical_page_allocator& palloc,
                     tos::cur_arch::address_space& root_table) {
    for (auto pheader : elf.program_headers()) {
        if (pheader.type != tos::elf::segment_type::load) {
            continue;
        }
        palloc.mark_unavailable(
            physical_range{physical_address{pheader.virtual_range().base.address()},
                           pheader.virtual_range().size});
    }

    auto backing = new elf_file_backing{elf};

    for (auto pheader : elf.program_headers()) {
        if (pheader.type != tos::elf::segment_type::load) {
            continue;
        }

        tos::debug::log("Map",
                        (void*)(uintptr_t)pheader.file_size,
                        "bytes from",
                        (void*)(uintptr_t)pheader.file_offset,
                        "to",
                        (void*)(uintptr_t)pheader.virt_address);

        auto seg = elf.segment(pheader);

        void* base = const_cast<uint8_t*>(seg.data());

        LOG((void*)pheader.virt_address,
            pheader.virt_size,
            (void*)pheader.file_offset,
            pheader.file_size,
            base);

        auto vseg = pheader.virtual_segment();
        vseg.range.size = pheader.file_size;

        if (tos::util::is_flag_set(vseg.perms, permissions::write) &&
            tos::util::is_flag_set(vseg.perms, permissions::execute)) {
            LOG_WARN("Write + Execute permission in ELF file.");
            return unexpected(elf_errors::write_execute_segment);
        }

        auto seg_map = new mapping{};
        seg_map->owned = true;
        seg_map->allow_user = user_accessible::yes;
        EXPECTED_TRYV(
            backing->create_mapping(vseg, reinterpret_cast<uintptr_t>(base), *seg_map));
        EXPECTED_TRYV(root_table.do_mapping(*seg_map, &palloc));

        auto kernel_seg_map = new mapping{};
        kernel_seg_map->owned = true;
        kernel_seg_map->allow_user = user_accessible::no;
        EXPECTED_TRYV(backing->clone_mapping(*seg_map, *kernel_seg_map));
        EXPECTED_TRYV(global::cur_as->do_mapping(*kernel_seg_map, &palloc));

        // If the file size is smaller than the virtual size, we put zero-pages after the
        // initialized part.
        if (pheader.file_size == pheader.virt_size) {
            continue;
        }

        const auto pages =
            (pheader.virt_size - pheader.file_size) / tos::cur_arch::page_size_bytes;
        const auto p = tos::span<physical_page>(palloc.allocate(pages), pages);
        const auto range = palloc.range_of(p);
        LOG(range.base, range.end());
        vseg.range.base += vseg.range.size;
        vseg.range.size = pheader.virt_size - pheader.file_size;
        EXPECTED_TRYV(map_region(*root_table.m_table,
                                 vseg,
                                 tos::user_accessible::yes,
                                 tos::memory_types::normal,
                                 &palloc,
                                 range.base));

        EXPECTED_TRYV(map_region(cur_arch::get_current_translation_table(),
                                 vseg,
                                 tos::user_accessible::no,
                                 tos::memory_types::normal,
                                 &palloc,
                                 range.base));
    }

    return {};
}

tos::expected<tos::span<uint8_t>, cur_arch::mmu_errors>
create_and_map_stack(size_t stack_size,
                     tos::physical_page_allocator& palloc,
                     tos::cur_arch::address_space& root_table) {
    stack_size = tos::align_nearest_up_pow2(stack_size, tos::cur_arch::page_size_bytes);
    auto page_count = stack_size / tos::cur_arch::page_size_bytes;
    auto stack_pages = palloc.allocate(page_count);

    if (!stack_pages) {
        return tos::unexpected(cur_arch::mmu_errors::page_alloc_fail);
    }

    auto stack_region = palloc.range_of({stack_pages, page_count});
    auto stack_address = stack_region.base;
    tos::debug::log("Stack at",
                    stack_address.direct_mapped(),
                    "-",
                    stack_region.end().direct_mapped());

    EXPECTED_TRYV(
        map_region(*root_table.m_table,
                   identity_map(tos::physical_segment{
                       .range = {stack_address, static_cast<ptrdiff_t>(stack_size)},
                       tos::permissions::read_write}),
                   tos::user_accessible::yes,
                   tos::memory_types::normal,
                   &palloc,
                   stack_address));

    EXPECTED_TRYV(
        map_region(cur_arch::get_current_translation_table(),
                   identity_map(tos::physical_segment{
                       .range = {stack_address, static_cast<ptrdiff_t>(stack_size)},
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
        LOG_ERROR("Could not parse payload!", force_error(elf_res));
        return nullptr;
    }

    auto root_as = tos::global::cur_as;
    auto our_as = root_as->clone(palloc);
    if (!our_as) {
        LOG_ERROR("Could not clone address space!");
        return nullptr;
    }

    auto& elf = force_get(elf_res);
    if (auto map_res = map_elf(elf, palloc, *force_get(our_as)); !map_res) {
        auto& err = force_error(map_res);
        tos::debug::error("Could not map ELF!", err);
        return nullptr;
    }

    auto stack_res = create_and_map_stack(
        4 * tos::cur_arch::page_size_bytes, palloc, *force_get(our_as));

    if (!stack_res) {
        tos::debug::error("Could not create and map stack!");
        return nullptr;
    }

    auto stack = force_get(stack_res);

    auto res = start_group(stack,
                           reinterpret_cast<void (*)()>(elf.header().entry),
                           trampoline,
                           name,
                           *force_get(our_as));

    if (res) {
        res->runner = &preemptive_user_group_runner::instance();
        res->as = std::move(force_get(our_as));
    }

    return res;
}
} // namespace tos::ae::kernel