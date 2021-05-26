#include <tos/vfs/node.hpp>
#include <tos/vfs/filesystem.hpp>

namespace tos::vfs {
void intrusive_unref(tos::vfs::node* obj) {
    obj->m_refcnt--;
    if (obj->m_refcnt == 0) {
        obj->m_fs->collect_node(obj);
    }
}

node::node(fs_ptr fs, node_id_t id, node_type type, node_mode mode)
    : m_fs{std::move(fs)}
    , m_type{type}
    , m_id{id}
    , m_mode{mode} {
}

node::~node() = default;
}
