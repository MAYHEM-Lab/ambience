#include "tos/thread.hpp"
#include <fmt/compile.h>
#include <sqlite3.h>
#include <tos/board.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <tos/span.hpp>

using bs = tos::bsp::board_spec;

void low_level_write(tos::span<const uint8_t>) {
}

int sqlite3_memvfs_init();

NO_ZERO
uint8_t data[1 * 1024 * 1024] = {};

void logfn(void* pArg, int iErrCode, const char* zMsg) {
    tos::debug::log(fmt::format(FMT_COMPILE("({}) {}"), iErrCode, zMsg));
}

void entry() {
    auto uart = bs::default_com::open();
    tos::debug::serial_sink uart_sink(&uart);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    uart_log.set_log_level(tos::debug::log_level::trace);
    tos::debug::set_default_log(&uart_log);

    tos::debug::log("Booted");

    sqlite3_config(SQLITE_CONFIG_LOG, &logfn, nullptr);

    auto init_res = sqlite3_initialize();
    tos::debug::log("init", init_res);

    auto mem_init_res = sqlite3_memvfs_init();
    tos::debug::log("mem_init", mem_init_res);

    sqlite3* db;
    auto path = fmt::format(FMT_COMPILE("file:foo?ptr={0:#x}&sz=0&max={1}"),
                            reinterpret_cast<uintptr_t>(&data),
                            sizeof data);
    LOG(path);
    auto open_res =
        sqlite3_open_v2(path.c_str(),
                        &db,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI,
                        nullptr);

    tos::debug::log("open:", open_res, db);

    char* err_msg = nullptr;
    auto rc = sqlite3_exec(db, "PRAGMA journal_mode = OFF;", nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to run pragma");
        LOG_ERROR(fmt::format(FMT_COMPILE("SQL error: {}"), err_msg));
//        sqlite3_free(err_msg);
    } else {
        LOG("Pragma executed successfully");
    }

    auto sql = "CREATE TABLE Friends(Id INTEGER PRIMARY KEY, Name TEXT);INSERT INTO Friends(Name) VALUES('hello')";
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to create table");
        LOG_ERROR(fmt::format(FMT_COMPILE("SQL error: {}"), err_msg));
        sqlite3_free(err_msg);
    } else {
        LOG("Table Friends created successfully");
    }

    int last_id = sqlite3_last_insert_rowid(db);
    LOG(fmt::format(FMT_COMPILE("The last Id of the inserted row is {}"), last_id));

    tos::this_thread::block_forever();
}

tos::stack_storage<TOS_DEFAULT_STACK_SIZE * 10> stak;
void tos_main() {
    tos::launch(stak, entry);
}