#pragma once

#include <string_view>
#include <tos/expected.hpp>
#include <tos/flags.hpp>
#include <tos/vfs/common.hpp>
#include <tos/vfs/node.hpp>

namespace tos::vfs {
class directory : public node {
public:
    [[nodiscard]] virtual uint32_t entry_count() const = 0;

    [[nodiscard]] virtual expected<node_id_t, errors>
    lookup(std::string_view name) const = 0;

    expected<void, errors> add_entry(std::string_view name, const node& node);

    expected<void, errors> remove_entry(std::string_view name, const node& node);

private:
    virtual expected<void, errors> do_add_entry(std::string_view name, const node&) = 0;

    virtual expected<void, errors> do_remove_entry(std::string_view name,
                                                   const node&) = 0;

protected:
    using node::node;
};
} // namespace tos::vfs