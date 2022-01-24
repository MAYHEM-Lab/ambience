#pragma once

#include <tos/function_ref.hpp>
#include <tos/self_pointing.hpp>
#include <tos/x86_64/exception.hpp>

namespace tos::x86_64 {
class pit {
public:
    void disable();
    void set_frequency(int freq);

    uint32_t get_period() const;
    uint32_t get_counter() const;

private:
    int m_freq;
};
} // namespace tos::x86_64