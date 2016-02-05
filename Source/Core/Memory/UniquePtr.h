#pragma once

#include "Core/Core.h"

#include "Core/Meta/Delete.h"

#include <memory.h>

#define FWD_UNIQUEPTR(T) \
    class T; \
    typedef Core::UniquePtr<T> CONCAT(U, T); \
    typedef Core::UniquePtr<const T> CONCAT(UC, T);

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Deleter = checked_deleter<T> >
using UniquePtr = std::unique_ptr<T, _Deleter>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Memory/RefPtr-inl.h"
