#pragma once

#include <tos/expected.hpp>
#include <tos/self_pointing.hpp>
#include <tos/soc/bcm283x.hpp>

namespace tos::device {
class bcm283x_sdhost : public self_pointing<bcm283x_sdhost> {
public:
    enum class errors
    {

    };

    static expected<std::unique_ptr<bcm283x_sdhost>, errors>
    open(volatile bcm283x::sdhost_control_block* sdhost);

private:
};
} // namespace tos::device