#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct no_deleter {
    void operator ()(T*) const {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Verify that types are complete for increased safety (must have !)
// http://www.boost.org/doc/libs/1_46_1/boost/checked_delete.hpp
//----------------------------------------------------------------------------
template<class T>
inline void checked_delete(T * x) {
    // intentionally complex - simplification causes regressions
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void) sizeof(type_must_be_complete);
    delete x;
}
//----------------------------------------------------------------------------
template<class T>
inline void checked_delete_ref(T *& x) {
    checked_delete(x);
    x = nullptr;
}
//----------------------------------------------------------------------------
template<class T>
inline void checked_array_delete(T * x) {
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void) sizeof(type_must_be_complete);
    delete[] x;
}
//----------------------------------------------------------------------------
template<class T> struct checked_deleter {
    typedef void result_type;
    typedef T * argument_type;
    void operator ()(T * x) const {
        // Core:: disables ADL
        Core::checked_delete(x);
    }
};
//----------------------------------------------------------------------------
template<typename T> struct checked_deleter<T[]> {
    typedef void result_type;
    typedef T argument_type[];
    void operator ()(T x[]) const {
        // Core:: disables ADL
        Core::checked_array_delete(x);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
