#include <tos/algorithm.hpp>
#include <tos/debug/log.hpp>
#include <tos/paging/physical_page_allocator.hpp>

namespace tos {
namespace {
physical_page_allocator* global_allocator;
}
physical_page_allocator::physical_page_allocator(size_t num_pages)
    : m_remaining{num_pages}
    , m_num_pages{num_pages} {
    for (auto& page : get_table()) {
        new (&page) physical_page();
    }
    physical_range this_obj;
    this_obj.base = physical_address(reinterpret_cast<uintptr_t>(this));
    this_obj.size = sizeof *this + get_table().size_bytes();
    for (auto& page : get_table()) {
        free_list.push_back(page);
    }
    mark_unavailable(this_obj);
    global_allocator = this;
}

physical_page_allocator* physical_page_allocator::instance() {
    return global_allocator;
}

physical_page* physical_page_allocator::allocate(int count, int align) {
    if (static_cast<size_t>(count) > m_remaining) {
        tos::debug::error("Page allocation failed!");
        return nullptr;
    }

    if (align != 1)
        return nullptr;

    physical_page* base = nullptr;
    if (count == 1) [[likely]] {
        if (free_list.empty()) {
            return nullptr;
        }

        base = &free_list.front();
    } else {
        auto it = tos::consecutive_find_if(get_table().begin(),
                                           get_table().end(),
                                           count,
                                           [](auto& p) { return p.free(); });

        if (it == get_table().end()) {
            tos::debug::error("Page allocation failed!");
            return nullptr;
        }

        base = &*it;
    }

    for (int i = 0; i < count; ++i) {
        free_list.erase(free_list.unsafe_find(base[i]));
        intrusive_ref(base + i);
    }

    m_remaining -= count;
    return base;
}

void physical_page_allocator::free(span<physical_page> pages) {
    for (auto& pg : pages) {
        free_list.push_front(pg);
        intrusive_unref(&pg);
        m_remaining += 1;
    }
}

physical_address physical_page_allocator::address_of(const physical_page& page) const {
    return physical_address{page_num(page) * page_size()};
}

physical_range physical_page_allocator::range_of(const physical_page& page) const {
    return physical_range{.base = physical_address(page_num(page) * page_size()),
                          .size = std::ptrdiff_t(page_size())};
}

int physical_page_allocator::page_num(const physical_page& page) const {
    return std::distance(get_table().data(), &page);
}

void physical_page_allocator::mark_unavailable(const physical_range& len) {
    auto begin_num = align_nearest_down_pow2(len.base.address(), page_size()) / page_size();
    auto end_num = align_nearest_up_pow2(len.end().address(), page_size()) / page_size();
    begin_num = std::min<int>(m_num_pages, begin_num);
    end_num = std::min<int>(m_num_pages, end_num);
    for (size_t i = begin_num; i < end_num; ++i) {
        m_remaining -= 1;
        intrusive_ref(&get_table()[i]);
        free_list.erase(free_list.unsafe_find(get_table()[i]));
    }
}

physical_page* physical_page_allocator::info(physical_address ptr) {
    return info(ptr.addr / page_size());
}

physical_page* physical_page_allocator::info(int32_t page_num) {
    if (static_cast<size_t>(page_num) >= m_num_pages) {
        return nullptr;
    }

    return &get_table()[page_num];
}
} // namespace tos