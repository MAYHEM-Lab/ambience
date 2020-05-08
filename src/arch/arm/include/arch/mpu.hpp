#pragma once

#include <common/driver_base.hpp>
#include <tos/bitfield.hpp>
#include <tos/expected.hpp>
#include <tos/function_ref.hpp>
#include <tos/memory.hpp>
#include <tos/self_pointing.hpp>
#include "assembly.hpp"

namespace tos::arm {
enum class mpu_errors
{
    bad_permissions
};

union RASR_reg_t {
    uint32_t raw;
    BitField<0, 1> enable;
    BitField<1, 5> size;
    // BitField<6, 2> reserved;
    BitField<8, 8> subregion_disable;
    BitField<8, 8> srd;
    BitField<16, 1> bufferable;
    BitField<16, 1> b;
    BitField<17, 1> cacheable;
    BitField<17, 1> c;
    BitField<18, 1> shareable;
    BitField<18, 1> s;
    BitField<19, 3> type_extension;
    BitField<19, 3> tex;
    // BitField<22, 2> reserved;
    BitField<24, 3> access_permissions;
    BitField<24, 3> ap;
    // BitField<27, 1> reserved;
    BitField<28, 1> execute_never;
    BitField<28, 1> xn;
    // BitField<29, 3> reserved;
};

class mpu
    : public tos::self_pointing<mpu>
    , public tos::tracked_driver<mpu, 1> {
public:
    mpu();

    size_t num_supported_regions() const;
    size_t min_region_size() const;

    std::optional<tos::memory_region> get_region(int region_id);
    tos::expected<void, mpu_errors> set_region(int region_id,
                                               const tos::memory_region& region,
                                               permissions perms);

    void set_callback(function_ref<void()> callback) {
        m_callback = callback;
    }

    void isr();

private:
    function_ref<void()> m_callback{[](void*) { arm::breakpoint(); }};
};
} // namespace tos::arm