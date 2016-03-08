#pragma once

#include <iterator>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if 0 != _SECURE_SCL
//----------------------------------------------------------------------------
template <typename T>
using CheckedArrayIterator = stdext::checked_array_iterator<typename std::add_pointer<T>::type>;
//----------------------------------------------------------------------------
template <typename T>
CheckedArrayIterator<T> MakeCheckedIterator(T* ptr, size_t count, size_t index) {
    return CheckedArrayIterator<T>(ptr, count, index);
}
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
template <typename T>
using CheckedArrayIterator = typename std::add_pointer<T>::type;
//----------------------------------------------------------------------------
template <typename T>
CheckedArrayIterator<T> MakeCheckedIterator(T* ptr, size_t count, size_t index) {
    Assert(index <= count);
    Assert(0 == count || nullptr != ptr);
    return ptr + index;
}
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CheckedArrayIterator<T> MakeCheckedIterator(T (&staticArray)[_Dim], size_t index) {
    return MakeCheckedIterator<T>(&staticArray[0], _Dim, index);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template<typename T, typename = void>
struct is_iterator : public std::false_type {};
//----------------------------------------------------------------------------
template<typename T>
struct is_iterator<T,
    typename std::enable_if<not std::is_same<typename std::iterator_traits<T>::value_type, void>::value>::type
> : public std::true_type {};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
