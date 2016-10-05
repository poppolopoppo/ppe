#pragma once

#include <iterator>

#include "Core/Meta/Warnings.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if 0 != _SECURE_SCL
//----------------------------------------------------------------------------
template <typename T>
using TCheckedArrayIterator = stdext::checked_array_iterator<typename std::add_pointer<T>::type>;
//----------------------------------------------------------------------------
template <typename T>
TCheckedArrayIterator<T> MakeCheckedIterator(T* ptr, size_t count, size_t index) {
    return TCheckedArrayIterator<T>(ptr, count, index);
}
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
template <typename T>
using TCheckedArrayIterator = typename std::add_pointer<T>::type;
//----------------------------------------------------------------------------
template <typename T>
TCheckedArrayIterator<T> MakeCheckedIterator(T* ptr, size_t count, size_t index) {
#ifdef WITH_CORE_ASSERT
    Assert(index <= count);
    Assert(0 == count || nullptr != ptr);
#else
    UNUSED(count);
#endif
    return ptr + index;
}
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TCheckedArrayIterator<T> MakeCheckedIterator(T (&staticArray)[_Dim], size_t index) {
    return MakeCheckedIterator<T>(&staticArray[0], _Dim, index);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _It, typename _Transform>
class TOutputIterator : public std::iterator<
    typename std::iterator_traits<_It>::iterator_category,
    decltype( std::declval<_Transform>()(*std::declval<_It>()) )
> {
public:
    typedef std::iterator<
        typename std::iterator_traits<_It>::iterator_category,
        decltype( std::declval<_Transform>()(*std::declval<_It>()) )
    >   parent_type;

    using typename parent_type::difference_type;
    using typename parent_type::iterator_category;
    using typename parent_type::pointer;
    using typename parent_type::value_type;

    TOutputIterator(const _It& it, const _Transform& transform) : _it(it), _transform(transform) {}

    TOutputIterator(const TOutputIterator& ) = default;
    TOutputIterator& operator =(const TOutputIterator& ) = default;

    TOutputIterator& operator++() /* prefix */ { ++_it; return *this; }
    TOutputIterator& operator--() /* prefix */ { --_it; return *this; }

    TOutputIterator operator++(int) /* postfix */ { return TOutputIterator(_it++); }
    TOutputIterator operator--(int) /* postfix */ { return TOutputIterator(_it--); }

    TOutputIterator& operator+=(difference_type n) { _it += n; return *this; }
    TOutputIterator& operator-=(difference_type n) { _it -= n; return *this; }

    TOutputIterator operator+(difference_type n) { return TOutputIterator(_it + n); }
    TOutputIterator operator-(difference_type n) { return TOutputIterator(_it - n); }

    decltype( std::declval<_Transform>()(*std::declval<_It>()) ) operator*() const { return _transform(*_it); }
    //pointer operator->() const { return _it.operator ->(); }

    decltype( std::declval<_Transform>()(*std::declval<_It>()) ) operator[](difference_type n) const { return _transform(_it[n]); }

    difference_type operator-(const TOutputIterator& other) const { return checked_cast<difference_type>(_it - other._it); }

    bool operator==(const TOutputIterator& other) const { return (_it == other._it); }
    bool operator!=(const TOutputIterator& other) const { return (_it != other._it); }

    bool operator< (const TOutputIterator& other) const { return (_it <  other._it); }
    bool operator> (const TOutputIterator& other) const { return (_it >  other._it); }

    bool operator<=(const TOutputIterator& other) const { return (_it <= other._it); }
    bool operator>=(const TOutputIterator& other) const { return (_it >= other._it); }

private:
    _It _it;
    _Transform _transform;
};
//----------------------------------------------------------------------------
template <typename _It, typename _Transform>
TOutputIterator<_It, _Transform> MakeOutputIterator(const _It& it, const _Transform& transform) {
    return TOutputIterator<_It, _Transform>(it, transform);
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
    typename std::enable_if< not std::is_same<typename std::iterator_traits<T>::value_type, void>::value >::type
> : public std::true_type {};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template<typename _Iterator, typename T, typename = void>
struct is_iterator_of : public std::false_type {};
//----------------------------------------------------------------------------
template<typename _Iterator, typename T>
struct is_iterator_of<_Iterator, T,
    typename std::enable_if< std::is_same<typename std::iterator_traits<_Iterator>::value_type, T>::value >::type
> : public std::true_type {};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
