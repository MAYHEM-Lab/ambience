#pragma once

namespace tos {
struct source_location {
    const char* file_name;
    uint16_t line;
    uint16_t column;
};
}