#pragma once

#include "Core_fwd.h"

#include "Allocator/Allocation.h"
#include "Allocator/InSituAllocator.h"
#include "Memory/MemoryView.h"
#include "Meta/Iterator.h"

#include <algorithm>
#include <initializer_list>
#include <utility>

#include "Meta/TypeTraits.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Uses the default allocator
#define ARRAY(_DOMAIN, T) \
    ::PPE::TArray<COMMA_PROTECT(T), ALLOCATOR(_DOMAIN)>
//----------------------------------------------------------------------------
// Don't allocate for first N elements, use inline storage instead
#define ARRAYINSITU(_DOMAIN, T, N) \
    ::PPE::TArray<COMMA_PROTECT(T), INLINE_ALLOCATOR(_DOMAIN, COMMA_PROTECT(T), N) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct TStaticArray {
    typedef T value_type;
    typedef Meta::TAddPointer<T> pointer;
    typedef Meta::TAddPointer<const T> const_pointer;
    typedef Meta::TAddReference<T> reference;
    typedef Meta::TAddReference<T> const_reference;

    typedef TCheckedArrayIterator<T> iterator;
    typedef TCheckedArrayIterator<const T> const_iterator;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    T Data[_Dim];

    NODISCARD iterator begin() { return MakeCheckedIterator(Data, 0); }
    NODISCARD iterator end() { return MakeCheckedIterator(Data, _Dim); }

    NODISCARD const_iterator begin() const { return MakeCheckedIterator(Data, 0); }
    NODISCARD const_iterator end() const { return MakeCheckedIterator(Data, _Dim); }

    NODISCARD reverse_iterator rbegin() { return reverse_iterator(end()); }
    NODISCARD reverse_iterator rend() { return reverse_iterator(begin()); }

    NODISCARD const_reverse_iterator rbegin() const { return reverse_iterator(end()); }
    NODISCARD const_reverse_iterator rend() const { return reverse_iterator(begin()); }

    CONSTEXPR reference at(size_t index) { Assert(index < _Dim); return Data[index]; }
    CONSTEXPR const_reference at(size_t index) const { Assert(index < _Dim); return Data[index]; }

    CONSTEXPR reference operator [](size_t index) { return at(index); }
    CONSTEXPR const_reference operator [](size_t index) const { return at(index); }

    CONSTEXPR pointer data() { return (&Data[0]); }
    CONSTEXPR const_pointer data() const { return (&Data[0]); }

    CONSTEXPR TMemoryView<T> MakeView() { return PPE::MakeView(Data); }
    CONSTEXPR TMemoryView<const T> MakeView() const { return PPE::MakeView(Data); }
    CONSTEXPR TMemoryView<const T> MakeConstView() const { return PPE::MakeView(Data); }

    CONSTEXPR inline friend bool operator ==(const TStaticArray& lhs, const TStaticArray& rhs) {
        return std::equal(lhs.begin(), lhs.end(), rhs.Data);
    }
    CONSTEXPR inline friend bool operator !=(const TStaticArray& lhs, const TStaticArray& rhs) {
        return (not operator ==(lhs, rhs));
    }

    CONSTEXPR inline friend hash_t hash_value(const TStaticArray& arr) {
        return hash_fwdit_constexpr(arr.Data, arr.Data + _Dim);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
class TArray : public _Allocator {
public:
    template <typename U, typename _OtherAllocator>
    friend class TArray;

    typedef _Allocator allocator_type;
    typedef TAllocatorTraits<allocator_type> allocator_traits;

    typedef T value_type;

    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    using size_type = size_t;
    using difference_type = ptrdiff_t;

    typedef TCheckedArrayIterator<value_type> iterator;
    typedef TCheckedArrayIterator<Meta::TAddConst<value_type>> const_iterator;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    TArray() NOEXCEPT : _data(nullptr), _size(0), _capacity(0) {}
    ~TArray() { clear(); }

    TArray(TArray&& rvalue) NOEXCEPT : TArray() { assign(std::move(rvalue)); }
    TArray& operator =(TArray&& rvalue) NOEXCEPT {
        assign(std::move(rvalue));
        return (*this);
    }

    TArray(const TArray& other) NOEXCEPT : TArray() { operator =(other); }
    TArray& operator =(const TArray& other) NOEXCEPT {
        typedef typename allocator_traits::propagate_on_container_copy_assignment propagate_type;
        allocator_copy_(other, propagate_type());
        assign(other.begin(), other.end());
        return *this;
    }

    size_type size() const { return _size; }
    size_type max_size() const { return std::numeric_limits<size_type>::max(); }
    bool empty() const { return (0 == _size); }

    pointer data() { return _data; }
    const_pointer data() const { return _data; }

    iterator begin() { return MakeCheckedIterator(_data, _size, 0); }
    iterator end() { return MakeCheckedIterator(_data, _size, _size); }

    const_iterator begin() const { return MakeCheckedIterator((const_pointer)_data, _size, 0); }
    const_iterator end() const { return MakeCheckedIterator((const_pointer)_data, _size, _size); }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end; }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }

    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

    const allocator_type& get_allocator() const { return static_cast<const allocator_type&>(*this); }

    reference at(size_type pos) { Assert(pos < _size); return _data[pos]; }
    const_reference at(size_type pos) const { Assert(pos < _size); return _data[pos]; }

    reference operator[](size_type pos) { return at(pos); }
    const_reference operator[](size_type pos) const { return at(pos); }

    reference front() { return at(0); }
    const_reference front() const { return at(0); }

    reference back() { return at(_size - 1); }
    const_reference back() const { return at(_size - 1); }

    template <typename _It>
    typename std::enable_if<Meta::is_iterator<_It>::value>::type
        assign(_It first, _It last);
    void assign(size_type count, const T& value);
    void assign(std::initializer_list<T> ilist) { assign(ilist.begin(), ilist.end()); }
    void assign(TArray&& rvalue);

    template <typename _It>
    auto assign(TIterable<_It> range) { return assign(range.begin(), range.end()); }

    template <typename U>
    void assign(const TMemoryView<U>& view) { assign(view.begin(), view.end()); }
    void assign(const TMemoryView<value_type>& view) { assign(std::make_move_iterator(view.begin()), std::make_move_iterator(view.end())); }
    void assign(const TMemoryView<const value_type>& view) { assign(view.begin(), view.end()); }

    void clear();

    void resize(size_type count);
    void resize(size_type count, const_reference value);
    void resize_Uninitialized(size_type count);

    void swap(TArray& other) NOEXCEPT;
    friend void swap(TArray& lhs, TArray& rhs) NOEXCEPT { lhs.swap(rhs); }

    operator TMemoryView<Meta::TAddConst<value_type>>() const { return MakeConstView(); }

    TMemoryView<value_type> MakeView() const { return TMemoryView<value_type>(_data, _size); }
    TMemoryView<Meta::TAddConst<value_type>> MakeConstView() const { return TMemoryView<Meta::TAddConst<value_type>>(_data, _size); }

    bool AliasesToContainer(const_reference v) const { return ((&v >= _data) && (&v < _data + _size)); }
    bool AliasesToContainer(const iterator& it) const { return (it >= begin() && it < end()); }
    bool AliasesToContainer(const const_iterator& it) const { return (it >= begin() && it < end()); }

    friend hash_t hash_value(const TArray& ar) { return hash_range(ar._data, ar._size); }

private:
    allocator_type& allocator_() { return static_cast<allocator_type&>(*this); }

    void allocator_copy_(const allocator_type& other, std::true_type );
    void allocator_copy_(const allocator_type& other, std::false_type ) { UNUSED(other); }

    pointer _data;
    size_type _size;
    size_type _capacity;
};
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
auto TArray<T, _Allocator>::assign(_It first, _It last)
    -> typename std::enable_if<Meta::is_iterator<_It>::value>::type {
    resize(std::distance(first, last));
    std::copy(first, last, begin());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TArray<T, _Allocator>::assign(size_type count, const T& value) {
    resize(count);
    std::fill(begin(), end(), value);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TArray<T, _Allocator>::assign(TArray&& rvalue) {
    Assert(&rvalue != this);

    const FAllocatorBlock b{ rvalue._data, rvalue._capacity * sizeof(value_type) };
    const bool moved = MoveAllocatorBlock(&allocator_traits::Get(*this), allocator_traits::Get(rvalue), b);

    if (moved) {
        clear();

        _data = rvalue._data;
        _size = rvalue._size;
        _capacity = rvalue._capacity;

        rvalue._data = nullptr;
        rvalue._capacity = rvalue._size = 0;
    }
    else {
        assign(
            MakeMoveIterator(rvalue.begin()),
            MakeMoveIterator(rvalue.end()) );

        rvalue.clear();
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TArray<T, _Allocator>::clear() {
    if (_data) {
        Meta::Destroy(MakeIterable(_data, _data + _size));
        allocator_traits::DeallocateT(*this, _data, _capacity);
        _data = nullptr;
        _size = 0;
        _capacity = 0;
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TArray<T, _Allocator>::resize(size_type count) {
    if (_size > count)
        Meta::Destroy(MakeView().CutStartingAt(count));

    const size_type n = allocator_traits::template SnapSizeT<value_type>(count);
    FAllocatorBlock b{ _data, _capacity * sizeof(value_type) };
    allocator_traits::Reallocate(allocator_(), b, n * sizeof(value_type));

    if (_size < count)
        Meta::Construct(MakeView().CutStartingAt(_size));

    _data = static_cast<pointer>(b.Data);
    _size = count;
    _capacity = n;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TArray<T, _Allocator>::resize(size_type count, const_reference value) {
    if (_size > count)
        Meta::Destroy(MakeView().CutStartingAt(count));

    const size_type n = allocator_traits::template SnapSizeT<value_type>(count);
    FAllocatorBlock b{ _data, _capacity * sizeof(value_type) };
    allocator_traits::Reallocate(allocator_(), b, n * sizeof(value_type));

    if (_size < count)
        Meta::Construct(MakeView().CutStartingAt(_size), value);

    _data = static_cast<pointer>(b.Data);
    _size = count;
    _capacity = n;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TArray<T, _Allocator>::resize_Uninitialized(size_type count) {
    if (_size > count)
        Meta::Destroy(MakeView().CutStartingAt(count));

    const size_type n = allocator_traits::template SnapSizeT<value_type>(count);
    FAllocatorBlock b{ _data, _capacity * sizeof(value_type) };
    allocator_traits::Reallocate(allocator_(), b, n * sizeof(value_type));

    _data = static_cast<pointer>(b.Data);
    _size = count;
    _capacity = n;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TArray<T, _Allocator>::swap(TArray& other) NOEXCEPT {
    typedef typename allocator_traits::propagate_on_container_swap propagate_type;
    if (this != &other) {
        IF_CONSTEXPR(propagate_type::value) {
            std::swap(_data, other._data);
            std::swap(_size, other._size);
            std::swap(_capacity, other._capacity);
            allocator_traits::Swap(*this, other);
        }
        else {
            TArray tmp{ std::move(*this) };
            assign(std::move(other));
            other.assign(std::move(tmp));
        }
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TArray<T, _Allocator>::allocator_copy_(const allocator_type& other, std::true_type ) {
    if (not allocator_traits::Equals(*this, other)) {
        clear();
        Assert(nullptr == _data);
        Assert(0 == _size);
        Assert(0 == _capacity);
        allocator_traits::Copy(this, other);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
