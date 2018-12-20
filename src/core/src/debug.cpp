//
// Created by fatih on 8/27/18.
//

#include <tos/debug.hpp>

#if defined(TOS_ARCH_lx106)
namespace tos
{
    esp82::sync_uart0 debug_out;
}
#endif