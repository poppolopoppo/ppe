#pragma once

#include <iterator>

#include "Core/Meta/TypeTraits.h"
#include "Core/Meta/Warnings.h"

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
template <typename T>
using TIteratorTraits = std::iterator_traits<T>;
//----------------------------------------------------------------------------
template <typename T, typename _Category = std::forward_iterator_tag>
using TIterator = std::iterator<
    _Category,
    TDecay<T>,
    ptrdiff_t,
    TAddPointer<T>,
    TAddReference<T>
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if 0 != _SECURE_SCL
//----------------------------------------------------------------------------
template <typename T>
using TCheckedArrayIterator = stdext::checked_array_iterator< Meta::TAddPointer<T> >;
//----------------------------------------------------------------------------
template <typename T>
TCheckedArrayIterator<T> MakeCheckedIterator(T* ptr, size_t count, size_t index) {
    return TCheckedArrayIterator<T>(ptr, count, index);
}
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
template <typename T>
using TCheckedArrayIterator = Meta::TAddPointer<T>;
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
class TOutputIterator : public Meta::TIterator<
    decltype(std::declval<_Transform>()(*std::declval<_It>())),
    typename std::iterator_traits<_It>::iterator_category
> {
public:
    typedef Meta::TIterator<
        decltype(std::declval<_Transform>()(*std::declval<_It>())),
        typename std::iterator_traits<_It>::iterator_category
    >   parent_type;

    using typename parent_type::iterator_category;
    using typename parent_type::difference_type;
    using typename parent_type::value_type;
    using typename parent_type::pointer;
    using typename parent_type::reference;

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
template <typename _It>
class TKeyIterator : public Meta::TIterator<
    Meta::TAddConst< decltype(std::declval<_It>()->first) >,
    std::forward_iterator_tag
> {
public:
    typedef Meta::TIterator<
        Meta::TAddConst< decltype(std::declval<_It>()->first) >,
        std::forward_iterator_tag
    >   parent_type;

    using typename parent_type::iterator_category;
    using typename parent_type::difference_type;
    using typename parent_type::value_type;
    using typename parent_type::pointer;
    using typename parent_type::reference;

    TKeyIterator() NOEXCEPT {}

    explicit TKeyIterator(_It&& it) : _it(std::move(it)) {}
    explicit TKeyIterator(const _It& it) : _it(it) {}

    TKeyIterator(const TKeyIterator&) = default;
    TKeyIterator& operator =(const TKeyIterator&) = default;

    const _It& inner() const { return _it; }

    TKeyIterator& operator++() /* prefix */ { _it.operator++(); return *this; }
    TKeyIterator operator++(int) /* postfix */ { const auto jt = _it; ++_it; return TKeyIterator(jt); }

    reference operator*() const { return (_it->first); }
    pointer operator->() const { return (&it->first); }

    void swap(TKeyIterator& other) { std::swap(_it, other._it); }
    inline friend void swap(TKeyIterator& lhs, TKeyIterator& rhs) { lhs.swap(rhs); }

    template <typename U>
    bool operator ==(const TKeyIterator<U>& other) const { return (_it == other.inner()); }
    template <typename U>
    bool operator !=(const TKeyIterator<U>& other) const { return (_it != other.inner()); }

private:
    _It _it;
};
//----------------------------------------------------------------------------
template <typename _It>
TKeyIterator<_It> MakeKeyIterator(const _It& it) { return TKeyIterator<_It>(it); }
template <typename _It>
TKeyIterator<_It> MakeKeyIterator(_It&& it) { return TKeyIterator<_It>(std::move(it)); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _It>
class TValueIterator : public Meta::TIterator<
    Meta::TAddConst<  decltype(std::declval<_It>()->second) >,
    std::forward_iterator_tag
> {
public:
    typedef Meta::TIterator<
        Meta::TAddConst<  decltype(std::declval<_It>()->second) >,
        std::forward_iterator_tag
    >   parent_type;

    using typename parent_type::iterator_category;
    using typename parent_type::difference_type;
    using typename parent_type::value_type;
    using typename parent_type::pointer;
    using typename parent_type::reference;

    TValueIterator() NOEXCEPT {}

    explicit TValueIterator(_It&& it) : _it(std::move(it)) {}
    explicit TValueIterator(const _It& it) : _it(it) {}

    TValueIterator(const TValueIterator&) = default;
    TValueIterator& operator =(const TValueIterator&) = default;

    const _It& inner() const { return _it; }

    TValueIterator& operator++() /* prefix */ { _it.operator++(); return *this; }
    TValueIterator operator++(int) /* postfix */ { const auto jt = _it; ++_it; return TValueIterator(jt); }

    reference operator*() const { return (_it->second); }
    pointer operator->() const { return (&it->second); }

    void swap(TValueIterator& other) { std::swap(_it, other._it); }
    inline friend void swap(TValueIterator& lhs, TValueIterator& rhs) { lhs.swap(rhs); }

    template <typename U>
    bool operator ==(const TValueIterator<U>& other) const { return (_it == other.inner()); }
    template <typename U>
    bool operator !=(const TValueIterator<U>& other) const { return (_it != other.inner()); }

private:
    _It _it;
};
//----------------------------------------------------------------------------
template <typename _It>
TValueIterator<_It> MakeValueIterator(const _It& it) { return TValueIterator<_It>(it); }
template <typename _It>
TValueIterator<_It> MakeValueIterator(_It&& it) { return TValueIterator<_It>(std::move(it)); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _It>
class TIterable {
public:
    typedef _It iterator;
    TIterable(iterator begin, iterator end)
        : _begin(std::move(begin))
        , _end(std::move(end)) {
        STATIC_ASSERT(Meta::is_iterator<_It>::value);
    }
    bool empty() const { return (_begin == _end); }
    const iterator& begin() const { return _begin; }
    const iterator& end() const { return _end; }
private:
    _It _begin;
    _It _end;
};
//----------------------------------------------------------------------------
template <typename _It>
TIterable<_It> MakeIterable(_It first, _It last) {
    return TIterable<_It>(first, last);
}
template <typename T>
TIterable< decltype(std::declval<T&>().begin()) > MakeIterable(T& container) {
    return MakeIterable(std::begin(container), std::end(container));
}
template <typename T>
TIterable< decltype(std::declval<const T&>().begin()) > MakeConstIterable(const T& container) {
    return MakeIterable(std::begin(container), std::end(container));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
