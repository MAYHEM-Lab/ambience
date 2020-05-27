#pragma once

#include <tos/intrusive_list.hpp>
#include <tos/intrusive_ptr.hpp>
#include <tos/vfs/common.hpp>

namespace tos::vfs {
class node : public list_node<node> {
public:
    virtual ~node();

    [[nodiscard]] node_mode mode() const {
        return m_mode;
    }

    [[nodiscard]] node_type type() const {
        return m_type;
    }

    [[nodiscard]] node_id_t id() const {
        return m_id;
    }

protected:
    explicit node(fs_ptr fs, node_id_t id, node_type type, node_mode mode);

private:
    friend void intrusive_ref(node* obj) {
        obj->m_refcnt++;
    }

    friend void intrusive_unref(node* obj);

    int16_t m_refcnt = 0;

    fs_ptr m_fs;
    node_type m_type;
    node_id_t m_id;
    node_mode m_mode;
};
} // namespace tos::vfs