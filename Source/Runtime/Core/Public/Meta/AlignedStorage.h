#pragma once

#include <type_traits>

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _SizeInBytes, size_t _Alignement>
using TAlignedStorage = typename std::aligned_storage<_SizeInBytes, _Alignement>::type;
//----------------------------------------------------------------------------
#define ALIGNED_STORAGE(_SIZE_IN_BYTES, _ALIGNMENT) PPE::Meta::TAlignedStorage<_SIZE_IN_BYTES, _ALIGNMENT>
//----------------------------------------------------------------------------
#define POD_STORAGE(T) ALIGNED_STORAGE(sizeof(COMMA_PROTECT(T)), alignof(COMMA_PROTECT(T)))
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
