#pragma once

#include "tos/expected.hpp"
#include "tos/utility.hpp"
namespace tos::nrf52::cc310 {
enum class cc_errors
{
    already_open,
    unknown_error,
    bad_key,
    bad_iv,
    bad_iv_length
};

class cryptocell : public tos::non_copy_movable {
public:
    ~cryptocell();
    cryptocell() = default;

private:
    friend expected<cryptocell, cc_errors> open_cryptocell();
};

expected<cryptocell, cc_errors> open_cryptocell();
} // namespace tos::nrf52::cc310