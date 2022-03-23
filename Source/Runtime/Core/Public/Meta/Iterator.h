#pragma once

#include "Meta/Algorithm.h"
#include "Meta/Hash_fwd.h"
#include "Meta/TypeTraits.h"
#include "HAL/PlatformMacros.h"

#include <iterator>
#include <numeric>

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
#if USE_PPE_ASSERT
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
template <typename _Int, _Int _Inc = _Int(1)>
class TCountingIterator : public Meta::TIterator<_Int, std::random_access_iterator_tag> {
public:
    typedef Meta::TIterator<_Int, std::random_access_iterator_tag> parent_type;

    using typename parent_type::iterator_category;
    using difference_type = decltype(_Int(0) - _Int(1));
    using typename parent_type::value_type;
    using typename parent_type::pointer;
    using typename parent_type::reference;

    CONSTEXPR explicit TCountingIterator(_Int it) : _it(it) {}

    CONSTEXPR TCountingIterator(const TCountingIterator&) = default;
    CONSTEXPR TCountingIterator& operator =(const TCountingIterator&) = default;

    CONSTEXPR TCountingIterator& operator++() /* prefix */ { _it += _Inc; return *this; }
    CONSTEXPR TCountingIterator& operator--() /* prefix */ { _it -= _Inc; return *this; }

    CONSTEXPR TCountingIterator operator++(int) /* postfix */ { const auto jt(_it); _it += _Inc; return TCountingIterator(jt); }
    CONSTEXPR TCountingIterator operator--(int) /* postfix */ { const auto jt(_it); _it -= _Inc; return TCountingIterator(jt); }

    CONSTEXPR TCountingIterator& operator+=(difference_type n) { _it += n * _Inc; return *this; }
    CONSTEXPR TCountingIterator& operator-=(difference_type n) { _it -= n * _Inc; return *this; }

    CONSTEXPR TCountingIterator operator+(difference_type n) const { return TCountingIterator(_it + n * _Inc); }
    CONSTEXPR TCountingIterator operator-(difference_type n) const { return TCountingIterator(_it - n * _Inc); }

    CONSTEXPR _Int operator*() const { return _it; }
    //pointer operator->() const { return _it.operator ->(); }

    CONSTEXPR _Int operator[](difference_type n) const { return (_it + n * _Inc); }

    CONSTEXPR difference_type operator-(const TCountingIterator& other) const { return checked_cast<difference_type>(_it - other._it); }

    CONSTEXPR bool operator==(const TCountingIterator& other) const { return (_it == other._it); }
    CONSTEXPR bool operator!=(const TCountingIterator& other) const { return (_it != other._it); }

    CONSTEXPR bool operator< (const TCountingIterator& other) const { return (_it < other._it); }
    CONSTEXPR bool operator> (const TCountingIterator& other) const { return (_it > other._it); }

    CONSTEXPR bool operator<=(const TCountingIterator& other) const { return (_it <= other._it); }
    CONSTEXPR bool operator>=(const TCountingIterator& other) const { return (_it >= other._it); }

private:
    _Int _it;
};
//----------------------------------------------------------------------------
template <typename _Int, _Int _Inc = _Int(1)>
CONSTEXPR TCountingIterator<_Int, _Inc> MakeCountingIterator(_Int it) {
    return TCountingIterator<_Int, _Inc>(it);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _It, typename _Filter>
class EMPTY_BASES TFilterIterator :
    public Meta::TIterator<
        typename Meta::TIteratorTraits<_It>::value_type,
        std::forward_iterator_tag
    >
{
public:
    typedef Meta::TIterator<
        typename Meta::TIteratorTraits<_It>::value_type,
        std::forward_iterator_tag
    >   parent_type;

    using filter_type = _Filter;

    using typename parent_type::iterator_category;
    using typename parent_type::difference_type;
    using typename parent_type::value_type;
    using typename parent_type::pointer;
    using typename parent_type::reference;

    CONSTEXPR TFilterIterator() NOEXCEPT {}

    CONSTEXPR explicit TFilterIterator(_It first, _It last)
    :   _first(first)
    ,   _last(last) {
        NextFiltered_();
    }

    CONSTEXPR explicit TFilterIterator(_It first, _It last, _Filter filter)
    :   _filter(std::forward<_Filter>(filter))
    ,   _first(first)
    ,   _last(last) {
        NextFiltered_();
    }

    CONSTEXPR TFilterIterator(const TFilterIterator&) = default;
    CONSTEXPR TFilterIterator& operator =(const TFilterIterator&) = default;

    CONSTEXPR const _It& inner() const { return _first; }

    CONSTEXPR TFilterIterator& operator++() /* prefix */ { Advance_(); return *this; }
    CONSTEXPR TFilterIterator operator++(int) /* postfix */ { const auto tmp{ *this }; Advance_(); return tmp; }

    CONSTEXPR auto operator*() const { return (*_first); }
    CONSTEXPR auto operator->() const { return std::addressof(*_first); }

    CONSTEXPR void swap(TFilterIterator& other) NOEXCEPT {
        using std::swap;
        swap(static_cast<_Filter&>(*this), static_cast<_Filter&>(other));
        swap(_first, other._first);
        swap(_last, other._last);
    }
    CONSTEXPR inline friend void swap(TFilterIterator& lhs, TFilterIterator& rhs) NOEXCEPT {
        lhs.swap(rhs);
    }

    CONSTEXPR bool operator ==(const TFilterIterator& other) const {
        Assert(other._last == _last);
        return (inner() == other.inner());
    }
    CONSTEXPR bool operator !=(const TFilterIterator& other) const {
        return (not operator ==(other));
    }

private:
    _Filter _filter;
    _It _first;
    _It _last;

    void Advance_() NOEXCEPT {
        Assert(_first != _last);
        ++_first;
        NextFiltered_();
    }
    void NextFiltered_() NOEXCEPT {
        for (; _first != _last; ++_first) {
            if (_filter(*_first))
                break;
        }
    }
};
//----------------------------------------------------------------------------
template <typename _It, typename _Filter>
CONSTEXPR TFilterIterator<_It, _Filter> MakeFilterIterator(_It first, _It last, _Filter&& rfilter) {
    return TFilterIterator<_It, _Filter>(
        std::forward<_It>(first),
        std::forward<_It>(last),
        std::move(rfilter) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _It, typename _Transform>
class EMPTY_BASES TOutputIterator :
    public Meta::TIterator<
        decltype(std::declval<_Transform>()(*std::declval<_It>())),
        typename std::iterator_traits<_It>::iterator_category
    >
{
public:
    typedef Meta::TIterator<
        decltype(std::declval<_Transform>()(*std::declval<_It>())),
        typename std::iterator_traits<_It>::iterator_category
    >   parent_type;

    using transform_type = _Transform;

    using typename parent_type::iterator_category;
    using typename parent_type::difference_type;
    using typename parent_type::value_type;
    using typename parent_type::pointer;
    using reference = decltype(std::declval<_Transform>()(*std::declval<_It>()));

    TOutputIterator() = default;

    CONSTEXPR TOutputIterator(_It it) : _it(it) {}
    CONSTEXPR TOutputIterator(_It it, _Transform transform) : _transform(std::forward<_Transform>(transform)), _it(it) {}

    CONSTEXPR TOutputIterator(const TOutputIterator& ) = default;
    CONSTEXPR TOutputIterator& operator =(const TOutputIterator& ) = default;

    CONSTEXPR TOutputIterator& operator++() /* prefix */ { ++_it; return *this; }
    CONSTEXPR TOutputIterator& operator--() /* prefix */ { --_it; return *this; }

    CONSTEXPR TOutputIterator operator++(int) /* postfix */ { const auto jt(_it); ++_it; return TOutputIterator(jt); }
    CONSTEXPR TOutputIterator operator--(int) /* postfix */ { const auto jt(_it); --_it; return TOutputIterator(jt); }

    CONSTEXPR TOutputIterator& operator+=(difference_type n) { _it += n; return *this; }
    CONSTEXPR TOutputIterator& operator-=(difference_type n) { _it -= n; return *this; }

    CONSTEXPR TOutputIterator operator+(difference_type n) const { return TOutputIterator(_it + n); }
    CONSTEXPR TOutputIterator operator-(difference_type n) const { return TOutputIterator(_it - n); }

    CONSTEXPR reference operator*() const { return _transform(*_it); }
    //pointer operator->() const { return _it.operator ->(); }

    CONSTEXPR reference operator[](difference_type n) const { return _transform(_it[n]); }

    CONSTEXPR difference_type operator-(const TOutputIterator& other) const { return checked_cast<difference_type>(_it - other._it); }

    CONSTEXPR bool operator==(const TOutputIterator& other) const { return (_it == other._it); }
    CONSTEXPR bool operator!=(const TOutputIterator& other) const { return (_it != other._it); }

    CONSTEXPR bool operator< (const TOutputIterator& other) const { return (_it <  other._it); }
    CONSTEXPR bool operator> (const TOutputIterator& other) const { return (_it >  other._it); }

    CONSTEXPR bool operator<=(const TOutputIterator& other) const { return (_it <= other._it); }
    CONSTEXPR bool operator>=(const TOutputIterator& other) const { return (_it >= other._it); }

private:
    _Transform _transform;
    _It _it;
};
//----------------------------------------------------------------------------
template <typename _It, typename _Transform>
CONSTEXPR TOutputIterator<_It, _Transform> MakeOutputIterator(const _It& it, const _Transform& transform) {
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

    CONSTEXPR TKeyIterator() NOEXCEPT {}

    CONSTEXPR explicit TKeyIterator(_It&& it) : _it(std::move(it)) {}
    CONSTEXPR explicit TKeyIterator(const _It& it) : _it(it) {}

    CONSTEXPR TKeyIterator(const TKeyIterator&) = default;
    CONSTEXPR TKeyIterator& operator =(const TKeyIterator&) = default;

    CONSTEXPR const _It& Inner() const { return _it; }

    CONSTEXPR TKeyIterator& operator++() /* prefix */ { ++_it; return *this; }
    CONSTEXPR TKeyIterator operator++(int) /* postfix */ { const auto jt = _it; ++_it; return TKeyIterator(jt); }

    CONSTEXPR reference operator*() const { return (_it->first); }
    CONSTEXPR pointer operator->() const { return (&_it->first); }

    CONSTEXPR void swap(TKeyIterator& other) NOEXCEPT { std::swap(_it, other._it); }
    CONSTEXPR inline friend void swap(TKeyIterator& lhs, TKeyIterator& rhs) NOEXCEPT { lhs.swap(rhs); }

    template <typename U>
    CONSTEXPR bool operator ==(const TKeyIterator<U>& other) const { return (_it == other.Inner()); }
    template <typename U>
    CONSTEXPR bool operator !=(const TKeyIterator<U>& other) const { return (_it != other.Inner()); }

private:
    _It _it;
};
//----------------------------------------------------------------------------
template <typename _It>
CONSTEXPR TKeyIterator<_It> MakeKeyIterator(const _It& it) { return TKeyIterator<_It>(it); }
template <typename _It>
CONSTEXPR TKeyIterator<_It> MakeKeyIterator(_It&& it) { return TKeyIterator<_It>(std::move(it)); }
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

    TValueIterator() = default;

    CONSTEXPR explicit TValueIterator(_It&& it) : _it(std::move(it)) {}
    CONSTEXPR explicit TValueIterator(const _It& it) : _it(it) {}

    CONSTEXPR TValueIterator(const TValueIterator&) = default;
    CONSTEXPR TValueIterator& operator =(const TValueIterator&) = default;

    CONSTEXPR const _It& Inner() const { return _it; }

    CONSTEXPR TValueIterator& operator++() /* prefix */ { ++_it; return *this; }
    CONSTEXPR TValueIterator operator++(int) /* postfix */ { const auto jt = _it; ++_it; return TValueIterator(jt); }

    CONSTEXPR reference operator*() const { return (_it->second); }
    CONSTEXPR pointer operator->() const { return (&_it->second); }

    CONSTEXPR void swap(TValueIterator& other) NOEXCEPT { std::swap(_it, other._it); }
    CONSTEXPR friend void swap(TValueIterator& lhs, TValueIterator& rhs) NOEXCEPT { lhs.swap(rhs); }

    template <typename U>
    CONSTEXPR bool operator ==(const TValueIterator<U>& other) const { return (_it == other.Inner()); }
    template <typename U>
    CONSTEXPR bool operator !=(const TValueIterator<U>& other) const { return (_it != other.Inner()); }

private:
    _It _it;
};
//----------------------------------------------------------------------------
template <typename _It>
CONSTEXPR TValueIterator<_It> MakeValueIterator(const _It& it) { return TValueIterator<_It>(it); }
template <typename _It>
CONSTEXPR TValueIterator<_It> MakeValueIterator(_It&& it) { return TValueIterator<_It>(std::move(it)); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _It>
class TIterable {
public:
    using iterator = _It;

    using value_type = typename std::iterator_traits<_It>::value_type;
    using pointer = typename std::iterator_traits<_It>::pointer;
    using reference = typename std::iterator_traits<_It>::reference;
    using const_pointer = Meta::TAddPointer<Meta::TAddConst<value_type>>;
    using const_reference = Meta::TAddReference<Meta::TAddConst<value_type>>;
    using iterator_category = typename std::iterator_traits<_It>::iterator_category;

    template <typename _Jt>
    friend class TIterable;

    TIterable() = default;

    CONSTEXPR TIterable(iterator begin, iterator end) NOEXCEPT
        : _begin(std::move(begin))
        , _end(std::move(end)) {
        STATIC_ASSERT(Meta::is_iterator<_It>::value);
    }

    CONSTEXPR operator const void* () const { return (_begin != _end ? this : nullptr); }

    CONSTEXPR bool empty() const NOEXCEPT { return (_begin == _end); }
    CONSTEXPR size_t size() const NOEXCEPT { return std::distance(_begin, _end); }

    CONSTEXPR const iterator& begin() const NOEXCEPT { return _begin; }
    CONSTEXPR const iterator& end() const NOEXCEPT { return _end; }

    auto Reverse() const {
        return TIterable<std::reverse_iterator<_It>>{ std::make_reverse_iterator(_end), std::make_reverse_iterator(_begin) };
    }

    template <typename _Outp>
    CONSTEXPR const TIterable& CopyTo(_Outp dst) const {
        std::copy(begin(), end(), dst);
        return (*this);
    }
    template <typename _Outp>
    CONSTEXPR const TIterable& UninitializedCopyTo(_Outp dst) const {
        std::uninitialized_copy(begin(), end(), dst);
        return (*this);
    }

    template <typename _Pred = Meta::TEqualTo<value_type> >
    CONSTEXPR bool Equals(const TIterable& other, _Pred pred = _Pred{}) const NOEXCEPT {
        return std::equal(_begin, _end, other._begin, other._end, pred);
    }
    template <typename _Jt, typename _Pred = Meta::TEqualTo<value_type> >
    CONSTEXPR bool Equals(const TIterable<_Jt>& other, _Pred pred = _Pred{}) const NOEXCEPT {
        return std::equal(_begin, _end, other._begin, other._end, pred);
    }

    template <typename _Transform>
    CONSTEXPR auto Map(_Transform&& transform) const NOEXCEPT;
    template <typename _Reduce>
    CONSTEXPR value_type Reduce(_Reduce&& reduce, value_type init = Meta::MakeForceInit<value_type>()) const NOEXCEPT;
    template <typename _Transform, typename _Reduce>
    CONSTEXPR auto MapReduce(_Transform&& transform, _Reduce&& reduce) const NOEXCEPT;

    template <typename _Filter>
    CONSTEXPR auto FilterBy(_Filter&& filter) const NOEXCEPT;

    template <typename _Selector>
    CONSTEXPR auto Select(_Selector&& selector) const NOEXCEPT;

    CONSTEXPR auto Accumulate(value_type init = Meta::MakeForceInit<value_type>()) const { return std::accumulate(begin(), end(), init); }

    CONSTEXPR auto Count(const_reference value) const { return std::count(begin(), end(), value); }
    template <typename _Pred>
    CONSTEXPR auto CountIf(_Pred&& pred) const { return std::count_if(begin(), end(), std::forward<_Pred>(pred)); }

    template <typename _Pred>
    CONSTEXPR bool Any(_Pred&& pred) const { return std::find_if(begin(), end(), std::forward<_Pred>(pred)) != end(); }
    CONSTEXPR iterator Find(const_reference value) const { return std::find(begin(), end(), value); }
    template <typename _Pred>
    CONSTEXPR iterator FindIf(_Pred&& pred) const { return std::find_if(begin(), end(), std::forward<_Pred>(pred)); }

    CONSTEXPR bool Contains(const_reference value) const { return Find(value) != end(); }

    CONSTEXPR auto LowerBound(const_reference value) const { return Meta::LowerBound(begin(), end(), value); }
    template <typename _Pred>
    CONSTEXPR auto LowerBound(_Pred&& pred) const { return Meta::LowerBound(begin(), end(), std::forward<_Pred>(pred)); }

    CONSTEXPR auto MaxElement() const { return std::max_element(begin(), end()); }
    CONSTEXPR auto MinElement() const { return std::min_element(begin(), end()); }
    CONSTEXPR auto MinMaxElement() const { return std::minmax_element(begin(), end()); }

    CONSTEXPR friend bool operator ==(const TIterable& a, const TIterable& b) NOEXCEPT { return (a.Equals(b)); }
    CONSTEXPR friend bool operator !=(const TIterable& a, const TIterable& b) NOEXCEPT { return (not operator ==(a, b)); }

    // see TEnumerable<T>
    CONSTEXPR bool Consume(pointer* current) NOEXCEPT {
        Assert(current);
        if (Likely(_begin != _end)) {
            *current = *_begin++;
            return true;
        }

        *current = nullptr;
        return false;
    }

    friend void swap(TIterable& lhs, TIterable& rhs) NOEXCEPT {
        using std::swap;
        swap(lhs._begin, rhs._begin);
        swap(lhs._end, rhs._end);
    }

private:
    _It _begin;
    _It _end;
};
//----------------------------------------------------------------------------
#if PPE_HAS_CXX17
template <typename _It>
TIterable(_It first, _It last) -> TIterable<_It>;
#endif
//----------------------------------------------------------------------------
template <typename _It>
CONSTEXPR TIterable<_It> MakeIterable(_It first, _It last) NOEXCEPT {
    return TIterable<_It>(first, last);
}
template <typename T, size_t N>
CONSTEXPR auto MakeIterable(T (&arr)[N]) NOEXCEPT {
    return MakeIterable(
        MakeCheckedIterator(arr, 0),
        MakeCheckedIterator(arr, N) );
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TIterable< decltype(std::declval<T&>().begin()) > MakeIterable(T& container) NOEXCEPT {
    return MakeIterable(std::begin(container), std::end(container));
}
template <typename T>
CONSTEXPR TIterable< decltype(std::declval<const T&>().begin()) > MakeConstIterable(const T& container) NOEXCEPT {
    return MakeIterable(std::begin(container), std::end(container));
}
//----------------------------------------------------------------------------
template <typename _It, typename _Transform>
CONSTEXPR TIterable<TOutputIterator<_It, _Transform>> MakeOutputIterable(_It first, _It last, _Transform transform) NOEXCEPT {
    return MakeIterable(
        TOutputIterator<_It, _Transform>(first, transform),
        TOutputIterator<_It, _Transform>(last, transform) );
}
//----------------------------------------------------------------------------
template <typename _It, typename _Filter>
CONSTEXPR TIterable<TFilterIterator<_It, _Filter>> MakeFilteredIterable(_It first, _It last, _Filter filter) NOEXCEPT {
    return MakeIterable(
        TFilterIterator<_It, _Filter>(first, last, filter),
        TFilterIterator<_It, _Filter>(last, last, filter) );
}
//----------------------------------------------------------------------------
template <typename _Int, _Int _Inc = _Int(1)>
CONSTEXPR TIterable<TCountingIterator<_Int, _Inc>> MakeInterval(_Int first, _Int last) NOEXCEPT {
    return MakeIterable(TCountingIterator<_Int, _Inc>(first),
                        TCountingIterator<_Int, _Inc>(last) );
}
//----------------------------------------------------------------------------
template <typename _Int, _Int _Inc = _Int(1)>
CONSTEXPR TIterable<TCountingIterator<_Int, _Inc>> MakeInterval(_Int count) NOEXCEPT {
    return MakeInterval(_Int(0), count);
}
//----------------------------------------------------------------------------
template <typename _It>
template <typename _Transform>
CONSTEXPR auto TIterable<_It>::Map(_Transform&& transform) const NOEXCEPT {
    return MakeOutputIterable(_begin, _end, std::forward<_Transform>(transform));
}
//----------------------------------------------------------------------------
template <typename _It>
template <typename _Reduce>
CONSTEXPR auto TIterable<_It>::Reduce(_Reduce&& reduce, value_type init) const NOEXCEPT -> value_type {
    return std::reduce(begin(), end(), init, reduce);
}
//----------------------------------------------------------------------------
template <typename _It>
template <typename _Transform, typename _Reduce>
constexpr auto TIterable<_It>::MapReduce(_Transform&& transform, _Reduce&& reduce) const NOEXCEPT {
    return Map(std::forward<_Transform>(transform))
        .Reduce(std::forward<_Reduce>(reduce));
}
//----------------------------------------------------------------------------
template <typename _It>
template <typename _Filter>
CONSTEXPR auto TIterable<_It>::FilterBy(_Filter&& filter) const NOEXCEPT {
    return MakeFilteredIterable(_begin, _end, std::forward<_Filter>(filter));
}
//----------------------------------------------------------------------------
template <typename _It>
template <typename _Selector>
CONSTEXPR auto TIterable<_It>::Select(_Selector&& select) const NOEXCEPT {
    using optional_t = decltype(select(std::declval<reference>()));
    return Map(std::move(select))
        .FilterBy([](const optional_t& optional) NOEXCEPT -> bool { return !!optional; })
        .Map([](const optional_t& filtered) NOEXCEPT { return *filtered; });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _It>
CONSTEXPR std::move_iterator<_It> MakeMoveIterator(_It it) NOEXCEPT {
    return std::make_move_iterator(it);
}
//----------------------------------------------------------------------------
template <typename _It>
CONSTEXPR TIterable<std::move_iterator<_It>> MakeMoveIterable(_It first, _It last) NOEXCEPT {
    return MakeIterable(MakeMoveIterator(first), MakeMoveIterator(last));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Meta {
//----------------------------------------------------------------------------
// Construct/Destroy ranges
//----------------------------------------------------------------------------
template <typename _It, typename... _Args>
CONSTEXPR void Construct(const TIterable<_It>& range, _Args&&... args) {
    using value_type = typename TIterable<_It>::value_type;
    IF_CONSTEXPR(sizeof...(args) || not Meta::has_trivial_constructor<value_type>::value) {
        for (auto& it : range)
            Meta::Construct(&it, args...);
    }
    else {
        UNUSED(range);
    }
}
//----------------------------------------------------------------------------
template <typename _It>
CONSTEXPR void Destroy(const TIterable<_It>& range) {
    using value_type = typename TIterable<_It>::value_type;
    IF_CONSTEXPR(not Meta::has_trivial_destructor<value_type>::value) {
        for (auto& it : range)
            Meta::Destroy(&it);
    }
    else {
        UNUSED(range);
    }
}
//----------------------------------------------------------------------------
} //!namespace Meta
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
