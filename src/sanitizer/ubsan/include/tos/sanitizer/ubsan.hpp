#pragma once

#include <tos/function_ref.hpp>

namespace tos::ubsan::handlers {
struct ubsan_handlers {
    virtual void error() = 0;
    virtual ~ubsan_handlers() = default;
};

struct null_handlers : ubsan_handlers {
    void error() override {
        while (true);
    }
};

inline null_handlers _null_handlers;
inline ubsan_handlers* handlers = &_null_handlers;
}
