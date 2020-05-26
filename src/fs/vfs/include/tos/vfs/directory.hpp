#pragma once

#include <string_view>
#include <tos/expected.hpp>
#include <tos/flags.hpp>
#include <tos/span.hpp>
#include <tos/vfs/common.hpp>
#include <tos/vfs/node.hpp>

namespace tos::vfs {
struct entry_info {
    size_t name_length;
    node_id_t node_id;
};

struct [[nodiscard]] directory_iterator_token{};

class directory : public node {
public:
    [[nodiscard]] virtual uint32_t entry_count() const = 0;

    [[nodiscard]] virtual expected<node_id_t, errors>
    lookup(std::string_view name) const = 0;

    expected<void, errors> add_entry(std::string_view name, const node& node);

    expected<void, errors> remove_entry(std::string_view name, const node& node);

    virtual directory_iterator_token* iteration_begin() const = 0;
    virtual entry_info iteration_entry_info(directory_iterator_token*,
                                            span<uint8_t> name_buf) const = 0;
    virtual bool iteration_next(directory_iterator_token*) const = 0;

private:
    virtual expected<void, errors> do_add_entry(std::string_view name, const node&) = 0;

    virtual expected<void, errors> do_remove_entry(std::string_view name,
                                                   const node&) = 0;

protected:
    using node::node;
};

struct directory_entry_range {
    struct directory_iterator {
        struct directory_iterator_end {};

        directory_iterator& operator++() {
            if (m_dir->iteration_next(m_cur)) {
                m_cur = nullptr;
            }
            return *this;
        }

        std::pair<std::string_view, node_id_t> operator*() const {
            auto info = m_dir->iteration_entry_info(m_cur, m_name_buf);
            auto name =
                m_name_buf.slice(0, std::min(info.name_length, m_name_buf.size()));
            auto name_view =
                std::string_view(reinterpret_cast<const char*>(name.data()), name.size());
            return {name_view, info.node_id};
        }

        friend bool operator==(const directory_iterator& iter,
                               const directory_iterator_end&) {
            return iter.m_cur == nullptr;
        }

        friend bool operator!=(const directory_iterator& iter,
                               const directory_iterator_end&) {
            return iter.m_cur != nullptr;
        }

        directory_iterator(const directory& dir, span<uint8_t> name_buf)
            : m_dir{&dir}
            , m_name_buf{name_buf}
            , m_cur{dir.iteration_begin()} {
        }

    private:
        const directory* m_dir;
        span<uint8_t> m_name_buf;
        directory_iterator_token* m_cur;
    };

    directory_entry_range(const directory& dir, span<uint8_t> name_buf)
        : m_dir{&dir}
        , m_buf{name_buf} {
    }

    directory_iterator begin() const {
        return directory_iterator(*m_dir, m_buf);
    }

    directory_iterator::directory_iterator_end end() const {
        return {};
    }

private:
    const directory* m_dir;
    span<uint8_t> m_buf;
};
} // namespace tos::vfs