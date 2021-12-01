//
// Created by Mehmet Fatih BAKIR on 14/05/2018.
//

#include <new>
#include <tos/components/allocator.hpp>
#include <tos/context.hpp>
#include <tos/ft.hpp>

using tos::allocator_component;

namespace tos {
void out_of_memory_handler(int sz);

[[gnu::weak]] void out_of_memory_handler(int sz) {
    LOG_WARN("Memory allocation failed!", sz);
}
} // namespace tos

void operator delete(void* pt, size_t) {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        alloc->allocator->free(pt);
        return;
    }
}

void operator delete(void* pt) {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        alloc->allocator->free(pt);
        return;
    }
}

void operator delete[](void* pt) {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        alloc->allocator->free(pt);
        return;
    }
}

void operator delete[](void* pt, size_t) {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        alloc->allocator->free(pt);
        return;
    }
}

// void operator delete(void* pt, std::align_val_t) {
//}
// void operator delete[](void* pt, std::align_val_t) {
//}
//
// void* operator new(size_t sz, std::align_val_t align) {
//    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc)
//    {
//        auto res = alloc->allocator->allocate(sz);
//        if (res == nullptr) {
//            tos::out_of_memory_handler();
//        }
//        return res;
//    }
//    tos::out_of_memory_handler();
//    return nullptr;
//}
//
// void* operator new[](size_t sz, std::align_val_t align) {
//    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc)
//    {
//        auto res = alloc->allocator->allocate(sz);
//        if (res == nullptr) {
//            tos::out_of_memory_handler();
//        }
//        return res;
//    }
//    tos::out_of_memory_handler();
//    return nullptr;
//}

void* operator new(size_t sz) {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        auto res = alloc->allocator->allocate(sz);
        if (res == nullptr) {
            tos::out_of_memory_handler(sz);
        }
        return res;
    }
    tos::out_of_memory_handler(sz);
    return nullptr;
}

void* operator new[](size_t sz) {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        auto res = alloc->allocator->allocate(sz);
        if (res == nullptr) {
            tos::out_of_memory_handler(sz);
        }
        return res;
    }
    tos::out_of_memory_handler(sz);
    return nullptr;
}

void* operator new(size_t sz, const std::nothrow_t&) noexcept {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        auto res = alloc->allocator->allocate(sz);
        if (res == nullptr) {
            tos::out_of_memory_handler(sz);
        }
        return res;
    }
    tos::out_of_memory_handler(sz);
    return nullptr;
}

void* operator new[](size_t sz, const std::nothrow_t&) noexcept {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        auto res = alloc->allocator->allocate(sz);
        if (res == nullptr) {
            tos::out_of_memory_handler(sz);
        }
        return res;
    }
    tos::out_of_memory_handler(sz);
    return nullptr;
}

// On ESP8266, the vendor lib supplies malloc symbols (ugh)
#if !defined(TOS_PLATFORM_esp8266) && !defined(TOS_PLATFORM_x86_hosted)
extern "C" {
void* _malloc_r(struct _reent*, size_t sz) {
    return new (std::nothrow) char[sz];
}

void* malloc(size_t sz) {
    return new (std::nothrow) char[sz];
}

void* realloc(void* oldptr, size_t newsz) {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        auto res = alloc->allocator->realloc(oldptr, newsz);
        if (res == nullptr) {
            tos::out_of_memory_handler(newsz);
        }
        return res;
    }
    tos::out_of_memory_handler(newsz);
    return nullptr;
}

void* calloc(size_t sz, size_t cnt) {
    return new char[sz * cnt]{};
}

void free(void* ptr) {
    if (!ptr) {
        return;
    }
    delete[] static_cast<char*>(ptr);
}
}
#endif