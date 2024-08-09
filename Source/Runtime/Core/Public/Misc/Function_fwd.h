#pragma once

#include "Meta/Aliases.h"
#include "Meta/Functor.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TFunctionRef<> is wrapping references callable objects, it cannot be stored!
//----------------------------------------------------------------------------
template <class T, class = typename decltype(Meta::TCallableObject{ std::declval<T>() })::function_type >
class TFunctionRef;
//----------------------------------------------------------------------------
// TFunction<> is using an insitu storage to avoid allocation, unlike std::function<>
//----------------------------------------------------------------------------
inline CONSTEXPR size_t GFunctionInSitu = (4 * sizeof(void*));
//----------------------------------------------------------------------------
template <class T, size_t _InSituSize = GFunctionInSitu, class = typename decltype(Meta::TCallableObject{ std::declval<T>() })::function_type >
class TFunction;
//----------------------------------------------------------------------------
// TSmallFunction<> is small enough to be stored in-situ in TFunction<>
//----------------------------------------------------------------------------
template <typename T>
using TSmallFunction = TFunction<T, sizeof(void*) * 2>;
//----------------------------------------------------------------------------
// TTinyFunction<> has only 1 pointer worth of in-situ storage
//----------------------------------------------------------------------------
template <typename T>
using TTinyFunction = TFunction<T, sizeof(void*)>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
