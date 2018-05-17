//
// Created by Mehmet Fatih BAKIR on 16/05/2018.
//

#pragma once

namespace tos
{
    namespace std
    {
        /**
         * Calls the destructor of the object pointed to by p, as if by p->~T().
         * @param p pointer to the object to destroy
         */
        template <class T>
        void destroy_at(T* p) noexcept;
    }
}

namespace tos
{
    namespace std
    {
        template<class T>
        void destroy_at(T* p) noexcept
        {
            p->~T();
        }
    }
}