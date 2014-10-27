#pragma once

#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define ALIGNED_STORAGE(_SIZE_IN_BYTES, _ALIGNMENT) \
    std::aligned_storage<(_SIZE_IN_BYTES), (_ALIGNMENT)>::type
//----------------------------------------------------------------------------
#define POD_STORAGE(T) ALIGNED_STORAGE(sizeof(T), std::alignment_of<T>::value)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// gave up using on those, compilers will crash or fail in very strange ways...
/*
template <size_t _SizeInBytes, size_t _Alignement>
using AlignedStorage = typename std::aligned_storage<_SizeInBytes, _Alignement>::type;
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment = std::alignment_of<T>::value >
using PODStorage = AlignedStorage<sizeof(T), _Alignment>;
*/
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
