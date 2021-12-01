#include <sqlite3.h>
#include <tos/span.hpp>

extern "C" {
// int sqlite3_initialize() {
//     return SQLITE_OK;
// }

int sqlite3_os_init() {
    return 0;
}

struct tm *localtime(const time_t *timep) {
    return nullptr;
}
}
