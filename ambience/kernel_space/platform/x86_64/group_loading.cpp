#include <nonstd/variant.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/arch.hpp>
#include <tos/debug/log.hpp>
#include <tos/elf.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/suspended_launch.hpp>
#include <tos/x86_64/syscall.hpp>

namespace {
using page_alloc_res = mpark::variant<tos::cur_arch::mmu_errors>;
using errors = mpark::variant<page_alloc_res, nullptr_t>;

void switch_to_user(void* user_code) {
    using namespace tos::x86_64;
    asm volatile("add $8, %rsp");
    asm volatile("movq $0, %rbp");
    asm volatile("movq $0x202, %r11");
    sysret(user_code);
}

tos::expected<void, errors> map_elf(const tos::elf::elf64& elf,
                                    tos::physical_page_allocator& palloc,
                                    tos::cur_arch::translation_table& root_table) {
    for (auto pheader : elf.program_headers()) {
        if (pheader.type != tos::elf::segment_type::load) {
            continue;
        }

        tos::debug::log("Load",
                        (void*)(uint32_t)pheader.file_size,
                        "bytes from",
                        (void*)(uint32_t)pheader.file_offset,
                        "to",
                        (void*)(uint32_t)pheader.virt_address);

        auto seg = elf.segment(pheader);

        void* base = const_cast<uint8_t*>(seg.data());
        if (pheader.file_size < pheader.virt_size) {
            auto pages = pheader.virt_size / tos::cur_arch::page_size_bytes;
            auto p = palloc.allocate(pages);
            base = palloc.address_of(*p);
        }

        LOG((void*)pheader.virt_address,
            pheader.virt_size,
            (void*)pheader.file_offset,
            pheader.file_size,
            base);

        auto vseg =
            tos::segment{.range = {.base = pheader.virt_address,
                                   .size = static_cast<ptrdiff_t>(pheader.virt_size)},
                         .perms = tos::permissions::all};

        EXPECTED_TRYV(tos::cur_arch::map_region(root_table,
                                                vseg,
                                                tos::user_accessible::yes,
                                                tos::memory_types::normal,
                                                &palloc,
                                                base));
    }

    return {};
}

tos::expected<tos::span<uint8_t>, errors>
create_and_map_stack(size_t stack_size,
                     tos::physical_page_allocator& palloc,
                     tos::cur_arch::translation_table& root_table) {
    stack_size = tos::align_nearest_up_pow2(stack_size, tos::cur_arch::page_size_bytes);
    auto page_count = stack_size / tos::cur_arch::page_size_bytes;
    auto stack_pages = palloc.allocate(page_count);

    if (!stack_pages) {
        return tos::unexpected(tos::cur_arch::mmu_errors::page_alloc_fail);
    }

    EXPECTED_TRYV(tos::cur_arch::map_region(
        root_table,
        tos::segment{.range = {.base = reinterpret_cast<uintptr_t>(
                                   palloc.address_of(*stack_pages)),
                               .size = static_cast<ptrdiff_t>(stack_size)},
                     tos::permissions::read_write},
        tos::user_accessible::yes,
        tos::memory_types::normal,
        &palloc,
        palloc.address_of(*stack_pages)));

    return tos::span<uint8_t>(static_cast<uint8_t*>(palloc.address_of(*stack_pages)),
                              stack_size);
}

tos::expected<tos::ae::kernel::user_group, errors> start_group(
    tos::span<uint8_t> stack, void (*entry)(), tos::interrupt_trampoline& trampoline) {
    LOG("Entry point:", (void*)entry);
    auto& user_thread = tos::suspended_launch(stack, switch_to_user, (void*)entry);
    set_name(user_thread, "User thread");

    auto& self = *tos::self();

    tos::ae::kernel::user_group res;
    res.state = &user_thread;
    auto syshandler = [&](tos::cur_arch::syscall_frame& frame) {
        assert(frame.rdi == 1);

        auto ifc = reinterpret_cast<tos::ae::interface*>(frame.rsi);
        res.iface.user_iface = ifc;

        trampoline.switch_to(self);
    };
    tos::x86_64::set_syscall_handler(tos::cur_arch::syscall_handler_t(syshandler));

    tos::swap_context(self, user_thread, tos::int_guard{});
    return res;
}
} // namespace

tos::expected<tos::ae::kernel::user_group, errors>
load_from_elf(const tos::elf::elf64& elf,
              tos::interrupt_trampoline& trampoline,
              tos::physical_page_allocator& palloc,
              tos::cur_arch::translation_table& root_table) {
    EXPECTED_TRYV(map_elf(elf, palloc, root_table));

    return start_group(EXPECTED_TRY(create_and_map_stack(
                           4 * tos::cur_arch::page_size_bytes, palloc, root_table)),
                       reinterpret_cast<void (*)()>(elf.header().entry),
                       trampoline);
}
