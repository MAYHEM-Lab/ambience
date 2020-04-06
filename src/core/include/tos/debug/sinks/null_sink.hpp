#pragma once

#include "sink.hpp"

namespace tos::debug {
class null_sink : public detail::any_sink {
public:
    bool begin() override {
        return false;
    }
    void add(int64_t) override {
    }
    void add(std::string_view) override {
    }
    void end() override {
    }
};
}