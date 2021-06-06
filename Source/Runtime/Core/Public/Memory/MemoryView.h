#pragma once

#include "Core_fwd.h"

#include "IO/TextWriter_fwd.h"
#include "Meta/Iterator.h"

#include <algorithm>
#include <array>
#include <initializer_list>
#include <iterator>
#include <type_traits>

#include "PtrRef.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TMemoryView {
public:
    template <typename U>
    friend class TMemoryView;

    typedef T value_type;
    typedef Meta::TAddPointer<T> pointer;
    typedef Meta::TAddPointer<const T> const_pointer;
    typedef Meta::TAddReference<T> reference;
    typedef Meta::TAddReference<const T> const_reference;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef TCheckedArrayIterator<value_type> iterator;
    typedef std::random_access_iterator_tag iterator_category;
    typedef std::reverse_iterator<iterator> reverse_iterator;

    CONSTEXPR TMemoryView() NOEXCEPT;
    CONSTEXPR TMemoryView(pointer storage, size_type size) NOEXCEPT;

    // enables type promotion between {T(),T(),T()} and TMemoryView<T>
    CONSTEXPR TMemoryView(std::initializer_list<T> list) NOEXCEPT
        : TMemoryView(list.begin(), std::distance(list.begin(), list.end())) {}

    // enables type promotion between std::array<T, _Dim> and TMemoryView<T>
    template <size_t _Dim>
    CONSTEXPR TMemoryView(std::array<T, _Dim>& arr) NOEXCEPT
        : TMemoryView(arr.data(), _Dim) {}
    template <size_t _Dim>
    CONSTEXPR TMemoryView(const std::array<Meta::TRemoveConst<T>, _Dim>& arr) NOEXCEPT
        : TMemoryView(arr.data(), _Dim) {}

    // enables type promotion between T[] and TMemoryView<T>
    template <size_t _Dim>
    CONSTEXPR TMemoryView(value_type (&staticArray)[_Dim]) NOEXCEPT
        : TMemoryView(staticArray, _Dim) {}

    TMemoryView(const iterator& first, const iterator& last) NOEXCEPT
        : TMemoryView(std::addressof(*first), std::distance(first, last)) {}

    CONSTEXPR TMemoryView(TMemoryView&& rvalue) NOEXCEPT;
    CONSTEXPR TMemoryView& operator =(TMemoryView&& rvalue) NOEXCEPT;

    CONSTEXPR TMemoryView(const TMemoryView& other) NOEXCEPT;
    CONSTEXPR TMemoryView& operator =(const TMemoryView& other) NOEXCEPT;

    template <typename U>
    CONSTEXPR TMemoryView(const TMemoryView<U>& other) NOEXCEPT;
    template <typename U>
    CONSTEXPR TMemoryView& operator =(const TMemoryView<U>& other) NOEXCEPT;

    CONSTEXPR pointer Pointer() const { return _storage; }
    CONSTEXPR size_t SizeInBytes() const { return _size * sizeof(T); }
    CONSTEXPR size_t StrideInBytes() const { return sizeof(T); }

    CONSTEXPR pointer data() const { return _storage; }
    CONSTEXPR size_type size() const { return _size; }
    CONSTEXPR bool empty() const { return 0 == _size; }

    iterator begin() const { return MakeCheckedIterator(_storage, _size, 0); }
    iterator end() const { return MakeCheckedIterator(_storage, _size, _size); }

    iterator cbegin() const { return begin(); }
    iterator cend() const { return end(); }

    reverse_iterator rbegin() const { return reverse_iterator(end()); }
    reverse_iterator rend() const { return reverse_iterator(begin()); }

    reference at(size_type index) const;
    reference operator [](size_type index) const { return at(index); }

    reference front() const { return at(0); }
    reference back() const { return at(_size - 1); }

    void CopyTo(const TMemoryView<Meta::TRemoveConst<T>>& dst) const;

    template <size_t _Dim>
    void CopyTo(Meta::TRemoveConst<T> (&dst)[_Dim], size_t offset = 0) const {
        Assert(_Dim >= offset + _size);
        CopyTo(TMemoryView<Meta::TRemoveConst<T>>(dst + offset, _size));
    }

    CONSTEXPR TMemoryView<T> Eat(size_t n) {
        AssertRelease(n <= _size);
        const TMemoryView<T> eaten{ _storage, n };
        _storage += n;
        _size -= n;
        return eaten;
    }

    template <typename _Pod>
    TMemoryView& EatRaw(_Pod* pDst, size_t n = sizeof(_Pod)) {
        Assert(pDst);
        Assert(n <= sizeof(_Pod));
        STATIC_ASSERT(Meta::is_pod_v<_Pod>);
        auto src = Eat(n);
        src.CopyTo(TMemoryView<_Pod>(pDst, 1).template Cast<Meta::TRemoveConst<T>>());
        return (*this);
    }

    template <typename U>
    auto Peek() const {
        Assert_NoAssume(sizeof(U) <= _size);
        using result_t = Meta::TConditional<std::is_const_v<T>, std::add_const_t<U>, U>;
        return reinterpret_cast<result_t*>(_storage);
    }

    NODISCARD TMemoryView<T> Slice(size_t index, size_t stride) const;
    NODISCARD TMemoryView< Meta::TAddConst<T> > SliceConst(size_t index, size_t stride) const;

    NODISCARD TMemoryView<T> SubRange(size_t offset, size_t count) const;
    NODISCARD TMemoryView< Meta::TAddConst<T> > SubRangeConst(size_t offset, size_t count) const;

    NODISCARD TMemoryView<T> SubRange(iterator first, iterator last) const;
    NODISCARD TMemoryView< Meta::TAddConst<T> > SubRangeConst(iterator first, iterator last) const;

    NODISCARD TMemoryView<T> CutStartingAt(size_t offset) const { return SubRange(offset, _size - offset); }
    NODISCARD TMemoryView< Meta::TAddConst<T> > CutStartingAtConst(size_t offset) const { return SubRangeConst(offset, _size - offset); }

    NODISCARD TMemoryView<T> CutStartingAt(iterator it) const {
        Assert_NoAssume(end() == it || AliasesToContainer(it));
        return (end() != it
            ? TMemoryView(std::addressof(*it), std::distance(it, end()))
            : TMemoryView(_storage+_size, size_type(0)) );
    }

    NODISCARD TMemoryView<T> CutStartingAt(reverse_iterator it) const {
        Assert_NoAssume(rend() == it || AliasesToContainer(it));
        return (rend() != it
            ? TMemoryView(std::addressof(*it), _storage + _size - std::addressof(*it))
            : TMemoryView(_storage, size_type(0)) );
    }

    NODISCARD TMemoryView<T> CutBefore(size_t offset) const { return SubRange(0, offset); }
    NODISCARD TMemoryView< Meta::TAddConst<T> > CutBeforeConst(size_t offset) const { return SubRangeConst(0, offset); }

    NODISCARD TMemoryView<T> CutBefore(iterator it) const {
        Assert_NoAssume(end() == it || AliasesToContainer(it));
        return TMemoryView<T>(_storage, std::distance(begin(), it));
    }

    NODISCARD TMemoryView<T> CutBefore(reverse_iterator it) const {
        Assert_NoAssume(rend() == it || AliasesToContainer(it));
        return TMemoryView<T>(_storage, std::addressof(*it) - _storage);
    }

    NODISCARD TMemoryView<T> FirstNElements(size_t count) const { return CutBefore(count); }
    NODISCARD TMemoryView<T> LastNElements(size_t count) const { Assert(_size >= count); return CutStartingAt(_size - count); }

    NODISCARD TMemoryView<T> TrimFirstNElements(size_t count) const { return CutStartingAt(count); }
    NODISCARD TMemoryView<T> TrimLastNElements(size_t count) const { Assert(_size >= count); return CutBefore(_size - count); }

    NODISCARD TMemoryView<T> ShiftBack(const size_type n = 1) const { Assert(_size >= n); return TMemoryView<T>(_storage, _size - n); }
    NODISCARD TMemoryView<T> ShiftFront(const size_type n = 1) const { Assert(_size >= n); return TMemoryView<T>(_storage + n, _size - n); }

    NODISCARD TMemoryView<T> GrowBack(const size_type n = 1) const { return TMemoryView<T>(_storage, _size + n); }
    NODISCARD TMemoryView<T> GrowFront(const size_type n = 1) const { return TMemoryView<T>(_storage - n, _size + n); }

    template <typename U>
    bool IsSubRangeOf(const TMemoryView<U>& parent) const {
        return ((void*)parent.data() <= (void*)_storage &&
                (void*)(parent.data()+parent.size()) >= (void*)(_storage+_size));
    }

    iterator Find(const T& elem) const { return std::find(begin(), end(), elem); }
    iterator FindAfter(const T& elem, iterator first) const {
        Assert(AliasesToContainer(first));
        const iterator last = end();
        return (first == last ? last : std::find(++first, last, elem));
    }

    reverse_iterator FindR(const T& elem) const { return std::find(rbegin(), rend(), elem); }
    reverse_iterator FindAfterR(const T& elem, reverse_iterator rfirst) const {
        Assert(AliasesToContainer(rfirst));
        const reverse_iterator rlast = rend();
        return (rfirst == rlast ? rlast : std::find(++rfirst, rlast, elem));
    }

    bool Contains(const T& elem) const { return (end() != Find(elem)); }

    auto MakeRef(iterator it) const { return (it != end() ? MakePtrRef(*it) : nullptr); }

    template <typename _Pred>
    auto Any(const _Pred& pred) const { return MakeRef(FindIf(pred)); }
    template <typename _Pred>
    auto Min(const _Pred& pred = Meta::TLess<Meta::TDecay<T>>{}) { return MakeRef(FindMin(pred)); }
    template <typename _Pred>
    auto Max(const _Pred& pred = Meta::TLess<Meta::TDecay<T>>{}) { return MakeRef(FindMax(pred)); }

    template <typename _Pred>
    iterator FindIf(const _Pred& pred) const { return std::find_if(begin(), end(), pred); }
    template <typename _Pred>
    size_type FindFirst(const _Pred& pred) const { return std::distance(begin(), FindIf(pred)); }

    template <typename _Pred>
    reverse_iterator FindIfR(const _Pred& pred) const { return std::find_if(rbegin(), rend(), pred); }
    template <typename _Pred>
    size_type FindLast(const _Pred& pred) const { return std::distance(rbegin(), FindIfR(pred)); }

    template <typename _Pred>
    iterator FindIfNot(const _Pred& pred) const { return std::find_if_not(begin(), end(), pred); }
    template <typename _Pred>
    size_type FindFirstNot(const _Pred& pred) const { return std::distance(begin(), FindIfNot(pred)); }

    template <typename _Pred>
    reverse_iterator FindIfNotR(const _Pred& pred) const { return std::find_if_not(rbegin(), rend(), pred); }
    template <typename _Pred>
    size_type FindLastNot(const _Pred& pred) const { return std::distance(rbegin(), FindIfNotR(pred)); }

    template <typename _Pred>
    iterator FindMin(const _Pred& pred = Meta::TLess<Meta::TDecay<T>>{}) const { return std::min_element(begin(), end(), pred); }
    template <typename _Pred>
    iterator FindMax(const _Pred& pred = Meta::TLess<Meta::TDecay<T>>{}) const { return std::max_element(begin(), end(), pred); }

    iterator FindSubRange(const TMemoryView<T>& subrange) const;

    bool EndsWith(const T& suffix) const;
    bool StartsWith(const T& prefix) const;

    bool EndsWith(const TMemoryView<T>& suffix) const;
    bool StartsWith(const TMemoryView<T>& prefix) const;

    NODISCARD TMemoryView Concat(const TMemoryView& other) const;
    NODISCARD TMemoryView Concat_AssumeNotEmpty(const TMemoryView& other) const;

    template <typename _Pred>
    NODISCARD TMemoryView SplitIf(const _Pred& pred) const { return TMemoryView(_storage, FindFirst(pred)); }
    template <typename _Pred>
    NODISCARD TMemoryView SplitIfNot(const _Pred& pred) const { return TMemoryView(_storage, FindFirstNot(pred)); }

    CONSTEXPR bool AliasesToContainer(const_reference v) const { return (_storage <= &v && _storage + _size > &v); }
    CONSTEXPR bool AliasesToContainer(iterator it) const { return (begin() <= it && it < end()); }
    CONSTEXPR bool AliasesToContainer(reverse_iterator it) const { return (rbegin() <= it && it < rend()); }

    template <typename U>
    NODISCARD TMemoryView<U> Cast() const;
    NODISCARD TMemoryView<const u8> RawView() const { return Cast<const u8>(); }

    template <typename _Map>
    auto Map(_Map&& map) const {
        return MakeIterable(
            MakeOutputIterator(begin(), map),
            MakeOutputIterator(end(), map) );
    }

    template <typename _Map, typename _Reduce>
    auto MapReduce(_Map&& map, _Reduce&& reduce,
        Meta::TDecay<decltype(std::declval<_Map>()(std::declval<T>()))>&& init = {} ) const {
        auto reduced{ std::move(init) };
        for (T& elt : *this)
            reduced = reduce(reduced, map(elt));
        return reduced;
    }

    template <typename _Map>
    auto Sum(_Map&& map,
        Meta::TDecay<decltype(std::declval<_Map>()(std::declval<T>()))>&& init = {} ) const {
        return MapReduce(std::move(map),
            [](auto a, auto b) CONSTEXPR NOEXCEPT{ return a + b; },
            std::move(init) );
    }

    auto Sum() const {
        return MapReduce(
            [](auto x) { return x; },
            [](auto a, auto b) CONSTEXPR NOEXCEPT{ return a + b; } );
    }

    // implicit cast to TIterable<>
    CONSTEXPR operator TIterable<iterator>() const {
        return MakeIterable(begin(), end());
    }

    CONSTEXPR friend void swap(TMemoryView& lhs, TMemoryView& rhs) NOEXCEPT {
        std::swap(lhs._storage, rhs._storage);
        std::swap(lhs._size, rhs._size);
    }

    CONSTEXPR friend bool operator ==(const TMemoryView& lhs, const TMemoryView& rhs) {
        return (lhs._storage == rhs._storage &&
                lhs._size == rhs._size );
    }

    CONSTEXPR friend bool operator !=(const TMemoryView& lhs, const TMemoryView& rhs) {
        return (not operator ==(lhs, rhs));
    }

protected:
    pointer _storage;
    size_type _size;
};
//----------------------------------------------------------------------------
// All memory views are considered as pods
//----------------------------------------------------------------------------
PPE_ASSUME_TEMPLATE_AS_POD(TMemoryView<T>, typename T)
//----------------------------------------------------------------------------
// Inplace reindexation, beware that indices will be modified too !
//----------------------------------------------------------------------------
template <typename T, typename _Index>
Meta::TEnableIf< std::is_integral<_Index>::value > ReindexMemoryView(const TMemoryView<T>& data, const TMemoryView<_Index>& indices) {
    Assert(data.size() == indices.size());

    forrange(i, 0, checked_cast<_Index>(indices.size())) {
        if (i == indices[i])
            continue;

        T x = std::move(data[i]);
        _Index j = i;

        while (true) {
            const _Index k = indices[j];
            indices[j] = j;
            if (k == i)
                break;

            data[j] = std::move(data[k]);
            j = k;
        }

        data[j] = std::move(x);
    }
}
//----------------------------------------------------------------------------
using FRawMemory = TMemoryView<u8>;
using FRawMemoryConst = TMemoryView<const u8>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CONSTEXPR TMemoryView<T> MakeView(T(&staticArray)[_Dim]) {
    return TMemoryView<T>(&staticArray[0], _Dim);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CONSTEXPR TMemoryView<Meta::TAddConst<T> > MakeConstView(T(&staticArray)[_Dim]) {
    return TMemoryView<Meta::TAddConst<T> >(&staticArray[0], _Dim);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CONSTEXPR TMemoryView<T> MakeView(std::array<T, _Dim>& arr) {
    return TMemoryView<T>(arr.data(), _Dim);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CONSTEXPR TMemoryView<const T> MakeView(const std::array<T, _Dim>& arr) {
    return TMemoryView<const T>(arr.data(), _Dim);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CONSTEXPR TMemoryView<Meta::TAddConst<T> > MakeConstView(const std::array<T, _Dim>& arr) {
    return TMemoryView<Meta::TAddConst<T> >(arr.data(), _Dim);
}
//----------------------------------------------------------------------------
template <typename _It>
typename std::enable_if<
    Meta::is_iterator<_It>::value,
    TMemoryView< typename std::iterator_traits<_It>::value_type >
>::type MakeView(_It first, _It last) {
    typedef std::iterator_traits<_It> traits_type;
    STATIC_ASSERT(std::is_same<typename traits_type::iterator_category, std::random_access_iterator_tag>::value);
    return TMemoryView< typename traits_type::value_type >(std::addressof(*first), std::distance(first, last));
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
TMemoryView<typename _VectorLike::value_type> MakeView(_VectorLike& container) {
    if (container.begin() != container.end())
        return TMemoryView<typename _VectorLike::value_type>(std::addressof(*std::begin(container)), std::distance(std::begin(container), std::end(container)) );
    else
        return TMemoryView<typename _VectorLike::value_type>();
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
TMemoryView<const typename _VectorLike::value_type> MakeView(const _VectorLike& container) {
    if (container.begin() != container.end())
        return TMemoryView<const typename _VectorLike::value_type>(std::addressof(*std::begin(container)), std::distance(std::begin(container), std::end(container)) );
    else
        return TMemoryView<const typename _VectorLike::value_type>();
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
TMemoryView<const typename _VectorLike::value_type> MakeConstView(const _VectorLike& container) {
    if (container.begin() != container.end())
        return TMemoryView<const typename _VectorLike::value_type>(std::addressof(*std::begin(container)), std::distance(std::begin(container), std::end(container)) );
    else
        return TMemoryView<const typename _VectorLike::value_type>();
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TMemoryView< T > MakeView(T* pbegin, T* pend) {
    Assert(pend >= pbegin);
    return TMemoryView< T >(pbegin, std::distance(pbegin, pend));
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TMemoryView<Meta::TAddConst<T> > MakeConstView(T* pbegin, T* pend) {
    Assert(pend >= pbegin);
    return TMemoryView<Meta::TAddConst<T> >(pbegin, std::distance(pbegin, pend));
}
//----------------------------------------------------------------------------
template <typename T>
TMemoryView<u8> MakeRawView(T& assumePod) {
    return TMemoryView<u8>(reinterpret_cast<u8*>(&assumePod), sizeof(T));
}
//----------------------------------------------------------------------------
template <typename T>
TMemoryView<u8> MakeRawView(const TMemoryView<T>& assumePods) {
    return assumePods.template Cast<u8>();
}
//----------------------------------------------------------------------------
template <typename T>
TMemoryView<const u8> MakeRawView(const T& assumePod) {
    return TMemoryView<const u8>(reinterpret_cast<const u8*>(&assumePod), sizeof(T));
}
//----------------------------------------------------------------------------
template <typename T>
TMemoryView<const u8> MakeRawView(const TMemoryView<const T>& assumePods) {
    return assumePods.template Cast<const u8>();
}
//----------------------------------------------------------------------------
template <typename T>
TMemoryView<const u8> MakeRawConstView(const T& assumePod) {
    return TMemoryView<const u8>(reinterpret_cast<const u8*>(&assumePod), sizeof(T));
}
//----------------------------------------------------------------------------
template <typename T>
TMemoryView<const u8> MakeRawConstView(const TMemoryView<T>& assumePods) {
    return assumePods.template Cast<const u8>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename U, typename V>
CONSTEXPR void Broadcast(const TMemoryView<U>& dst, V&& value) {
    std::fill(dst.begin(), dst.end(), std::forward<V>(value));
}
//----------------------------------------------------------------------------
template <typename U, typename V>
CONSTEXPR void Copy(const TMemoryView<U>& dst, const TMemoryView<V>& src) {
    Assert(dst.size() == src.size());
    std::copy(src.begin(), src.end(), dst.begin());
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR void Move(const TMemoryView<T>& dst, const TMemoryView<T>& src) {
    Assert(dst.size() == src.size());
    std::move(src.begin(), src.end(), dst.begin());
}
//----------------------------------------------------------------------------
template <typename T, typename _Lambda>
CONSTEXPR void Collect(const TMemoryView<T>& dst, _Lambda&& collect) {
    forrange(i, 0, dst.size())
        collect(i, &dst[i]);
}
//----------------------------------------------------------------------------
template <typename T, typename U>
bool Contains(const TMemoryView<T>& v, const U& elt) {
    return (v.end() != std::find(v.begin(), v.end(), elt));
}
//----------------------------------------------------------------------------
template <typename T, typename U>
size_t IndexOf(const TMemoryView<T>& v, const U& elt) {
    const auto it = std::find(v.begin(), v.end(), elt);
    return (v.end() == it ? std::distance(v.begin(), it) : INDEX_NONE);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename T >
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TMemoryView<T>& view) {
    for (const auto& it : view)
        oss << it;
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Memory/MemoryView-inl.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Construct/Destroy views
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
void Construct(const TMemoryView<T>& view, _Args&& ... args) {
    Construct(MakeIterable(view.begin(), view.end()), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename T>
void Destroy(const TMemoryView<T>& view) NOEXCEPT {
    Destroy(MakeIterable(view.begin(), view.end()));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
