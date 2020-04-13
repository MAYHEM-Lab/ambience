#pragma once

#include "sink.hpp"

namespace tos::debug {
class null_sink : public detail::any_sink {
public:
    bool begin(log_level) override {
        return false;
    }
    void add(int64_t) override {
    }
    void add(std::string_view) override {
    }
    void add(bool) override {
    }
    void add(void*) override {
    }
    void add(log_level) override {
    }
    void add(span<const uint8_t>) override {
    }
    void end() override {
    }
};
}