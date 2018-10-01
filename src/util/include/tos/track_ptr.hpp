//
// Created by fatih on 7/16/18.
//

#pragma once

#include <string.h>
#include <type_traits>
#include <cstddef>

namespace tos {
    class tracked;

    template<class T>
    class track_ptr;

    template<class T>
    track_ptr<T> get_ptr(T& obj);

    template<class OutT, class InT>
    track_ptr<OutT> pointer_cast(track_ptr<InT>& obj);

    template<class T>
    track_ptr<const T> get_ptr(const T& obj);

    namespace detail {
        class track_ptr
        {
            friend class tos::tracked;

            tracked* obj;
            track_ptr* next;
            track_ptr* prev;

            void update_obj(::std::nullptr_t)
            {
                update_obj((tracked*) nullptr);
            }

            template<class T>
            void update_obj(T* new_obj)
            {
                for (auto ptr = this; ptr; ptr = ptr->next) {
                    ptr->obj = new_obj;
                }
            }

            void detach()
            {
                if (prev) {
                    prev->next = next;
                }
                if (next) {
                    next->prev = prev;
                }
            }

            void move_from(track_ptr&& rhs)
            {
                memcpy(this, &rhs, sizeof(rhs));

                if (next) {
                    next->prev = this;
                }
                if (prev) {
                    prev->next = this;
                }

                memset(&rhs, 0, sizeof(rhs));
            }

            void copy_from(track_ptr& rhs)
            {
                obj = rhs.obj;
                next = rhs.next;
                prev = &rhs;

                if (next) {
                    next->prev = this;
                }

                rhs.next = this;
            }

            template <class T>
            explicit track_ptr(T* obj) : obj(obj), next{}, prev{} {}

        public:

            track_ptr(::std::nullptr_t) : obj{}, next{}, prev{} {}

            track_ptr(track_ptr& rhs) noexcept
            {
                copy_from(rhs);
            }

            track_ptr(track_ptr&& rhs) noexcept
            {
                move_from(std::move(rhs));
            }

            track_ptr& operator=(track_ptr& rhs) noexcept
            {
                detach();
                copy_from(rhs);
                return *this;
            }

            track_ptr& operator=(track_ptr&& rhs) noexcept
            {
                detach();
                move_from(std::move(rhs));
                return *this;
            }

            bool operator==(const track_ptr& rhs) const noexcept
            {
                return rhs.obj==obj;
            }

            bool operator!=(const track_ptr& rhs) const noexcept
            {
                return rhs.obj!=obj;
            }

            bool operator==(const ::std::nullptr_t&) const noexcept
            {
                return nullptr==obj;
            }

            bool operator!=(const ::std::nullptr_t&) const noexcept
            {
                return nullptr!=obj;
            }

            ~track_ptr()
            {
                // detach from the link upon destruction
                detach();
            }

            explicit operator bool() const
            {
                return *this != nullptr;
            }

        protected:
            tracked* get_base_ptr() const
            {
                return obj;
            }
        };
    }

    template<class T>
    class track_ptr : public detail::track_ptr
    {
        friend track_ptr<T> get_ptr<T>(T&);
        template <class Out, class In> friend track_ptr<Out> pointer_cast(track_ptr<In>&);

        track_ptr(detail::track_ptr& rhs) :
                detail::track_ptr(rhs) {
            static_assert(::std::is_base_of<tracked, T>::value, "Type does not inherit from tracked!");
        }

        track_ptr(detail::track_ptr&& rhs) :
                detail::track_ptr(std::move(rhs)) {
            static_assert(::std::is_base_of<tracked, T>::value, "Type does not inherit from tracked!");
        }


    public:
        track_ptr() : detail::track_ptr(nullptr) {}

        template <class U>
        track_ptr(track_ptr<U>&& rhs) :
                detail::track_ptr(std::move(rhs))
        {
        }

        template <class U>
        track_ptr(track_ptr<U>& rhs) :
                detail::track_ptr(rhs)
        {
        }

        T* get() const noexcept
        { return static_cast<T*>(get_base_ptr()); }

        T& operator*() const noexcept
        { return static_cast<T*>(get_base_ptr()); }

        T* operator->() const noexcept
        { return static_cast<T*>(get_base_ptr()); }

        using detail::track_ptr::operator=;
        using detail::track_ptr::operator==;
        using detail::track_ptr::operator!=;
    };

    class tracked
    {
        template <class T> friend track_ptr<T> get_ptr(T&);
        template <class Out, class In> friend track_ptr<Out> pointer_cast(track_ptr<In>&);

    protected:
        mutable detail::track_ptr m_head;

        tracked() noexcept
                : m_head(this)
        {}

        tracked(const tracked&) noexcept
                : m_head(this)
        {
        }

        tracked(tracked&& rhs) noexcept
                : m_head(std::move(rhs.m_head))
        {
            m_head.update_obj(this); // set all the pointers in the link to us
        }

        tracked& operator=(const tracked&)
        {
            m_head.update_obj(nullptr); // set all the links to null, so when they try to access it, they'll presumably die
            m_head = detail::track_ptr(this);
            return *this;
        }

        tracked& operator=(tracked&& rhs) noexcept
        {
            m_head.update_obj(nullptr); // set all the links to null, so when they try to access it, they'll presumably die
            m_head = std::move(rhs.m_head);
            m_head.update_obj(this); // set all the pointers in the link to us
            return *this;
        }

        ~tracked()
        {
            m_head.update_obj(nullptr); // set all the links to null, so when they try to access it, they'll presumably die
        }
    };

    template <class T>
    track_ptr<T> get_ptr(T& obj)
    {
        return track_ptr<T>(obj.m_head);
    }

    template <class T>
    track_ptr<const T> get_ptr(const T& obj)
    {
        return track_ptr<T>(obj.m_head);
    }

    template <class OutT, class InT>
    track_ptr<OutT> pointer_cast(track_ptr<InT>& in)
    {
        return track_ptr<OutT>(static_cast<OutT*>(in.get())->m_head);
    }
}

