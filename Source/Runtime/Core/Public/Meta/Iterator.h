#pragma once

#include "Meta/TypeTraits.h"
#include "Meta/Warnings.h"

#include <iterator>

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename = void>
struct is_iterator : public std::false_type {};
//----------------------------------------------------------------------------
template <typename T>
struct is_iterator<T,
    typename std::enable_if< not std::is_same<typename std::iterator_traits<T>::value_type, void>::value >::type
> : public std::true_type {};
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR bool is_iterator_v = is_iterator<T>::value;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Iterator, typename T, typename = void>
struct is_iterator_of : public std::false_type {};
//----------------------------------------------------------------------------
template <typename _Iterator, typename T>
struct is_iterator_of<_Iterator, T,
    typename std::enable_if< std::is_same<typename std::iterator_traits<_Iterator>::value_type, T>::value >::type
> : public std::true_type {};
//----------------------------------------------------------------------------
template <typename _Iterator, typename T>
CONSTEXPR bool is_iterator_of_v = is_iterator_of<_Iterator, T>::value;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _It>
using is_forward_iterator = typename std::is_same<
    std::forward_iterator_tag,
    typename std::iterator_traits<_It>::iterator_category
>::type;
//----------------------------------------------------------------------------
template <typename _It>
CONSTEXPR bool is_forward_iterator_v = is_forward_iterator<_It>::value;
//----------------------------------------------------------------------------
template <typename _It>
using is_random_access_iterator = typename std::is_same<
    std::random_access_iterator_tag,
    typename std::iterator_traits<_It>::iterator_category
>::type;
//----------------------------------------------------------------------------
template <typename _It>
CONSTEXPR bool is_random_access_iterator_v = is_random_access_iterator<_It>::value;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using TIteratorTraits = std::iterator_traits<T>;
//----------------------------------------------------------------------------
template <typename T, typename _Category = std::forward_iterator_tag>
struct TIterator {
    using iterator_category = _Category;
    using value_type = TRemoveConst<TRemoveReference<T>>;
    using difference_type = ptrdiff_t;
    using pointer = TAddPointer<T>;
    using reference = TAddReference<T>;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef _SECURE_SCL
#   define USE_PPE_CHECKEDARRAYITERATOR _SECURE_SCL
#else
#   define USE_PPE_CHECKEDARRAYITERATOR 0
#endif
//----------------------------------------------------------------------------
#if USE_PPE_CHECKEDARRAYITERATOR
//----------------------------------------------------------------------------
// Sadly all of this is needed simply because stdext::checked_array_iterator<>
// isn't compliant with std requirements of const_iterator = iterator.
// And at the same time many algorithms in MSVC STL require iterators to be
// "safe", aka being a stdext::checked_array_iterator<>.
//----------------------------------------------------------------------------
template <typename T>
class TCheckedArrayIterator : public stdext::checked_array_iterator< Meta::TAddPointer<T> > {
    typedef stdext::checked_array_iterator< Meta::TAddPointer<T> > parent_type;
public:
    using typename parent_type::iterator_category;
    using typename parent_type::value_type;
    using typename parent_type::difference_type;
    using typename parent_type::pointer;
    using typename parent_type::reference;

    using parent_type::operator ==;
    using parent_type::operator !=;
    using parent_type::operator <;
    using parent_type::operator >;
    using parent_type::operator <=;
    using parent_type::operator >=;
    using parent_type::operator *;
    using parent_type::operator ->;
    using parent_type::operator ++;
    using parent_type::operator --;
    using parent_type::operator +;
    using parent_type::operator -;
    using parent_type::operator +=;
    using parent_type::operator -=;
    using parent_type::operator [];

    TCheckedArrayIterator() {}

    TCheckedArrayIterator(const TCheckedArrayIterator&) = default;
    TCheckedArrayIterator& operator=(const TCheckedArrayIterator&) = default;

    template <typename U, class = Meta::TEnableIf< std::is_convertible<U*, T*>::value> >
    TCheckedArrayIterator(const TCheckedArrayIterator<U>& other)
        : parent_type(reinterpret_cast<const parent_type&>(other))
    {}

    template <typename U, class = Meta::TEnableIf< std::is_convertible<U*, T*>::value> >
    TCheckedArrayIterator& operator=(const TCheckedArrayIterator<U>& other) {
        parent_type::operator=(reinterpret_cast<const parent_type&>(other));
        return *this;
    }

    // needed for implicit conversion
    TCheckedArrayIterator(const parent_type& other) : parent_type(other) {}
    TCheckedArrayIterator& operator =(const parent_type& other) {
        parent_type::operator=(other);
        return *this;
    }

    // overload stdext::checked_array_iterator<> for this constructor
    // std needs iterator = const_iterator which is supported by default
    template <typename U>
    TCheckedArrayIterator(U* ptr, size_t count, size_t index)
        : parent_type(ptr, count, index)
    {}

    // All those are specialized to return a TCheckArrayIterator instead of stdext::checked_array_iterator

    TCheckedArrayIterator& operator++() {
        parent_type::operator++();
        return (*this);
    }

    TCheckedArrayIterator operator++(int) {
        TCheckedArrayIterator tmp(*this);
        ++(*this);
        return tmp;
    }

    TCheckedArrayIterator& operator--() {
        parent_type::operator--();
        return (*this);
    }

    TCheckedArrayIterator operator--(int) {
        TCheckedArrayIterator tmp(*this);
        --(*this);
        return tmp;
    }

    TCheckedArrayIterator& operator+=(const difference_type offset) {
        parent_type::operator+=(offset);
        return (*this);
    }

    TCheckedArrayIterator operator+(const difference_type offset) const {
        TCheckedArrayIterator tmp(*this);
        return (tmp += offset);
    }

    TCheckedArrayIterator& operator-=(const difference_type offset) {
        parent_type::operator-=(offset);
        return (*this);
    }

    TCheckedArrayIterator operator-(const difference_type offset) const {
        TCheckedArrayIterator tmp(*this);
        return (tmp -= offset);
    }

    // bidirectional comparisons
    template <typename U>
    using TCastable = Meta::TEnableIf<
        std::is_convertible<U*, T*>::value ||
        std::is_convertible<T*, U*>::value
    >;

    template <typename U, class = TCastable<U> >
    difference_type operator-(const TCheckedArrayIterator<U>& other) const {
        return parent_type::operator-(reinterpret_cast<const parent_type&>(other));
    }

    template <typename U, class = TCastable<U> >
    bool operator==(const TCheckedArrayIterator<U>& other) const {
        return parent_type::operator==(reinterpret_cast<const parent_type&>(other));
    }

    template <typename U, class = TCastable<U> >
    bool operator!=(const TCheckedArrayIterator<U>& other) const {
        return parent_type::operator!=(reinterpret_cast<const parent_type&>(other));
    }

    template <typename U, class = TCastable<U> >
    bool operator<(const TCheckedArrayIterator<U>& other) const {
        return parent_type::operator<(reinterpret_cast<const parent_type&>(other));
    }

    template <typename U, class = TCastable<U> >
    bool operator>(const TCheckedArrayIterator<U>& other) const {
        return parent_type::operator>(reinterpret_cast<const parent_type&>(other));
    }

    template <typename U, class = TCastable<U> >
    bool operator<=(const TCheckedArrayIterator<U>& other) const {
        return parent_type::operator<=(reinterpret_cast<const parent_type&>(other));
    }

    template <typename U, class = TCastable<U> >
    bool operator>=(const TCheckedArrayIterator<U>& other) const {
        return parent_type::operator>=(reinterpret_cast<const parent_type&>(other));
    }
};
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
#ifdef WITH_PPE_ASSERT
    Assert(index <= count);
    Assert(0 == count || nullptr != ptr);
#else
    UNUSED(count);
#endif
    return ptr + index;
}
//----------------------------------------------------------------------------
#endif //!USE_PPE_CHECKEDARRAYITERATOR
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

    TOutputIterator(const _It& it) : _it(it) {}
    TOutputIterator(const _It& it, const _Transform& transform) : _it(it), _transform(transform) {}

    TOutputIterator(const TOutputIterator& ) = default;
    TOutputIterator& operator =(const TOutputIterator& ) = default;

    TOutputIterator& operator++() /* prefix */ { ++_it; return *this; }
    TOutputIterator& operator--() /* prefix */ { --_it; return *this; }

    TOutputIterator operator++(int) /* postfix */ { const auto jt(_it); ++_it; return TOutputIterator(jt); }
    TOutputIterator operator--(int) /* postfix */ { const auto jt(_it); --_it; return TOutputIterator(jt); }

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
    pointer operator->() const { return (&_it->first); }

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
    Meta::TAddConst< decltype(std::declval<_It>()->second) >,
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
    pointer operator->() const { return (&_it->second); }

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
template <typename _It>
std::move_iterator<_It> MakeMoveIterator(_It it) {
    return std::make_move_iterator(it);
}
//----------------------------------------------------------------------------
template <typename _It>
TIterable<std::move_iterator<_It>> MakeMoveIterable(_It first, _It last) {
    return MakeIterable(MakeMoveIterator(first), MakeMoveIterator(last));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
