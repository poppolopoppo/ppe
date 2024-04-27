#pragma once

#include "Meta/Aliases.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TFunction<> is using an insitu storage to avoid allocation, unlike std::function<>
//----------------------------------------------------------------------------
inline CONSTEXPR size_t GFunctionInSitu = (4 * sizeof(intptr_t) - sizeof(void*));
//----------------------------------------------------------------------------
template <typename T, size_t _InSitu = GFunctionInSitu>
class TFunction;
//----------------------------------------------------------------------------
// TSmallFunction<> is small enough to be stored in-situ in TFunction<>
//----------------------------------------------------------------------------
template <typename T>
using TSmallFunction = TFunction<T, sizeof(intptr_t) * 2>;
//----------------------------------------------------------------------------
// TTinyFunction<> has only 1 pointer worth of in-situ storage
//----------------------------------------------------------------------------
template <typename T>
using TTinyFunction = TFunction<T, sizeof(intptr_t)>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
