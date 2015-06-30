#pragma once

#include "Core/Core.h"

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct Activator {
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef typename std::add_pointer<T>::type pointer;
    typedef typename std::add_pointer<const T>::type const_pointer;
    typedef typename std::add_lvalue_reference<T>::type reference;
    typedef typename std::add_lvalue_reference<const T>::type const_reference;
    typedef T value_type;

    static void Construct(pointer p, const T& val) {
        ::new ((void**)p) T(val);
    }

    template<typename U, typename... _Args>
    static void Construct(U* p, _Args&&... args) {
        ::new((void*)p) U(std::forward<_Args>(args)...);
    }

    static void Destroy(pointer p) {
        typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
        (void) sizeof(type_must_be_complete);
        p->~T();
    }

    template<typename U>
    static void Destroy(U* p) {
        typedef char type_must_be_complete[sizeof(U) ? 1 : -1];
        (void) sizeof(type_must_be_complete);
        p->~U();
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
