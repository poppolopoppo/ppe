#pragma once

#include <type_traits>

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if PPE_HAS_CXX20
// C++23 has deprecated std::aligned_storage_t... this helper replicates the same functionality
template <size_t _SizeInBytes, size_t _Alignment = alignof(std::max_align_t)>
struct TAlignedStorage {
    alignas(_Alignment) std::byte _Space[_SizeInBytes];
};
#else
template <size_t _SizeInBytes, size_t _Alignment = alignof(std::max_align_t)>
using TAlignedStorage = typename std::aligned_storage<_SizeInBytes, _Alignment>::type;
#endif
//----------------------------------------------------------------------------
#define ALIGNED_STORAGE(_SIZE_IN_BYTES, _ALIGNMENT) ::PPE::Meta::TAlignedStorage<_SIZE_IN_BYTES, _ALIGNMENT>
//----------------------------------------------------------------------------
#define POD_STORAGE(T) ALIGNED_STORAGE(sizeof(COMMA_PROTECT(T)), alignof(COMMA_PROTECT(T)))
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
