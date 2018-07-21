#pragma once

#include "Core/Core.h"

#include "Core/Meta/Delete.h"

#include <memory.h>

#define _FWD_UNIQUEPTR_IMPL(T, _PREFIX)                                     \
    class CONCAT(_PREFIX, T);                                               \
    typedef ::Core::TUniquePtr<CONCAT(_PREFIX, T)>          CONCAT(U,  T);  \
    typedef ::Core::TUniquePtr<const CONCAT(_PREFIX, T)>    CONCAT(UC, T)

#define FWD_UNIQUEPTR(T_WITHOUT_F)              _FWD_UNIQUEPTR_IMPL(T_WITHOUT_F, F)
#define FWD_INTEFARCE_UNIQUEPTR(T_WITHOUT_I)    _FWD_UNIQUEPTR_IMPL(T_WITHOUT_I, I)

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Deleter = checked_deleter<T> >
using TUniquePtr = std::unique_ptr<T, _Deleter>;
//----------------------------------------------------------------------------
template <typename T, typename _Deleter = checked_deleter<T> >
TUniquePtr<T, _Deleter> MakeUnique(T* ptr) {
    return TUniquePtr<T, _Deleter>(ptr);
}
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
Meta::TEnableIf< not std::is_array_v<T>, TUniquePtr<T> > MakeUnique(_Args&&... args) {
    return TUniquePtr<T>(new T{ std::forward<_Args>(args)... });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Memory/RefPtr-inl.h"
