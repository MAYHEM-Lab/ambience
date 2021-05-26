#pragma once

#include <cstdint>
#include <tos/expected.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/vfs/common.hpp>
#include <tos/vfs/node.hpp>

namespace tos::vfs {
class filesystem : list_node<filesystem> {
public:
    virtual ~filesystem() = default;

    virtual void collect_node(node* obj) = 0;

    virtual node_id_t root_node_id() = 0;

    expected<node_ptr, errors> open(node_id_t, node_mode);

    expected<node_ptr, errors> open_root() {
        return do_open(root_node_id(),
                       readonly() ? node_mode::readable : node_mode::read_write);
    }

    void sync() {
        if (readonly())
            return;
        do_sync();
    }

    [[nodiscard]] bool readonly() const {
        return m_readonly;
    }

private:
    virtual void do_sync() = 0;

    virtual expected<node_ptr, errors> do_open(node_id_t, node_mode) = 0;

    friend void intrusive_ref(filesystem* fs) {
        fs->m_refcnt++;
    }

    friend void intrusive_unref(filesystem* fs) {
        fs->m_refcnt--;
        if (fs->m_refcnt == 0) {
            // Collect fs
        }
    }

    bool m_readonly;

    int16_t m_refcnt;

protected:
    explicit filesystem(bool readonly)
        : m_readonly{readonly} {
    }
};
} // namespace tos::vfs