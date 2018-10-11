#pragma once

#include <stdint.h>
#include <tos/span.hpp>
#include "xbee/constants.hpp"
#include "xbee/types.hpp"

namespace tos
{
    template <class UsartT>
    class xbee_s1
    {
    public:
    private:

        UsartT* m_usart;
    };
}
