#pragma once

#include "common/driver_base.hpp"
#include "tos/mutex.hpp"
#include <cstdint>
#include <tos/span.hpp>
#include <tos/semaphore.hpp>
#include <tos/expected.hpp>

namespace tos {
namespace stm32 {

enum class flash_errors
{
    unlock_failed,
    erase_failed,
    operation_failed,
    bad_size,
    bad_align
};

class flash : public tracked_driver<flash, 1> {
public:
    using sector_id_t = uint32_t;

    flash();

    expected<void, flash_errors> 
    erase(sector_id_t sector_id);
    
    expected<void, flash_errors>
    write(sector_id_t sector_id, span<const char> data, size_t offset);

    expected<void, flash_errors>
    read(sector_id_t sector_id, span<char> data, size_t offset);



    void isr(uint32_t page);
    void isr_fail(uint32_t page);

private:

    bool m_last_op_fail;
    semaphore m_done{0};
};

} // namespace stm32
} // namespace tos