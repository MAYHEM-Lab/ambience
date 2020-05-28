#pragma once

#include <tos/expected.hpp>
#include <tos/span.hpp>
#include <tos/vfs/common.hpp>
#include <tos/vfs/node.hpp>

namespace tos::vfs {
class file : public node {
public:
    [[nodiscard]] virtual file_size_t size() const = 0;

    expected<span<uint8_t>, errors> read(file_offset_t, span<uint8_t> data);
    expected<size_t, errors> write(file_offset_t, span<const uint8_t> data);

private:
    virtual expected<span<uint8_t>, errors> do_read(file_offset_t,
                                                    span<uint8_t> data) = 0;

    virtual expected<size_t, errors> do_write(file_offset_t,
                                              span<const uint8_t> data) = 0;

protected:
    using node::node;
};
} // namespace tos::vfs