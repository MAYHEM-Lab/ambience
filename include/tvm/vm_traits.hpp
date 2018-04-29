//
// Created by fatih on 4/29/18.
//

namespace tvm
{
    template <class VmT = void>
    struct vm_traits
    {
        static constexpr auto opcode_len = 7;
    };

    template <class VmT = void>
    static constexpr auto opcode_len_v = vm_traits<VmT>::opcode_len;
}