#include <tos/algorithm.hpp>
#include <tos/paging/physical_page_allocator.hpp>
#include <tos/debug/log.hpp>

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
    memory_range this_obj;
    this_obj.base = reinterpret_cast<uintptr_t>(this);
    this_obj.size = sizeof *this + get_table().size_bytes();
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

    auto it = tos::consecutive_find_if(
        get_table().begin(), get_table().end(), count, [](auto& p) { return p.free(); });

    if (it == get_table().end()) {
        tos::debug::error("Page allocation failed!");
        return nullptr;
    }

    for (int i = 0; i < count; ++i) {
        intrusive_ref(&*(it + i));
    }
    m_remaining -= count;
    return &*it;
}

void physical_page_allocator::free(span<physical_page> pages) {
    for (auto& pg : pages) {
        intrusive_unref(&pg);
        m_remaining += 1;
    }
}

void* physical_page_allocator::address_of(const physical_page& page) const {
    return reinterpret_cast<void*>(page_num(page) * page_size());
}

memory_range physical_page_allocator::range_of(const physical_page& page) const {
    return memory_range{.base = page_num(page) * page_size(),
                        .size = std::ptrdiff_t(page_size())};
}

int physical_page_allocator::page_num(const physical_page& page) const {
    return std::distance(get_table().data(), &page);
}

void physical_page_allocator::mark_unavailable(const memory_range& len) {
    auto begin_num = align_nearest_down_pow2(len.base, page_size()) / page_size();
    auto end_num = align_nearest_up_pow2(len.end(), page_size()) / page_size();
    begin_num = std::min<int>(m_num_pages, begin_num);
    end_num = std::min<int>(m_num_pages, end_num);
    for (size_t i = begin_num; i < end_num; ++i) {
        m_remaining -= 1;
        intrusive_ref(&get_table()[i]);
    }
}

physical_page* physical_page_allocator::info(void* ptr) {
    return info(reinterpret_cast<uintptr_t>(ptr) / page_size());
}

physical_page* physical_page_allocator::info(int32_t page_num) {
    if (static_cast<size_t>(page_num) >= m_num_pages) {
        return nullptr;
    }

    return &get_table()[page_num];
}
} // namespace tos