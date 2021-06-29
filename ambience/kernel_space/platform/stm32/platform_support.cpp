#include <block_memory_generated.hpp>
#include <file_system_generated.hpp>
#include <tos/ae/kernel/platform_support.hpp>
#include <tos/ae/kernel/runners/preemptive_user_runner.hpp>
#include <tos/ae/registry.hpp>

tos::ae::services::block_memory::sync_server* init_stm32_flash_serv();
tos::ae::services::block_memory::sync_server* init_block_partiton(
    tos::ae::services::block_memory::sync_server* dev, int base_block, int block_count);
tos::ae::services::filesystem::sync_server*
make_littlefs_server(tos::ae::services::block_memory::sync_server* flash);

tos::ae::registry_base& get_registry();

void platform_support::stage1_init() {
}

void platform_support::stage2_init() {
    tos::ae::preemptive_user_group_runner::create(
        tos::make_erased_preemptive_runner(*this));
    auto flash = init_stm32_flash_serv();
    tos::debug::log(flash->get_block_size(), flash->get_block_count());
    auto partition = init_block_partiton(
        flash, 7 * (flash->get_block_count() / 8), flash->get_block_count() / 8);
    auto fs = make_littlefs_server(partition);
    get_registry().register_service("fs", fs);
}

platform_group_args platform_support::make_args() {
    return platform_group_args{.trampoline = &*m_trampoline};
}