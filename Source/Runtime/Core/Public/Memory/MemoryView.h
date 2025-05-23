#pragma once

#include "Core_fwd.h"

#include "IO/TextWriter_fwd.h"
#include "Meta/Iterator.h"

#include <algorithm>
#include <array>
//#include <initializer_list>
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

    CONSTEXPR TMemoryView() = default;

    CONSTEXPR TMemoryView(pointer storage, size_type size) NOEXCEPT;

    // enables type promotion between {T(),T(),T()} and TMemoryView<T>
    CONSTEXPR TMemoryView(std::initializer_list<T> list) NOEXCEPT
        // /!\ std::initializer_list<T> don't guarantee static lifetime !
        // https://en.cppreference.com/w/cpp/utility/initializer_list#:~:text=underlying%20array%20is%20a%20temporary%20array
        : TMemoryView(list.begin(), list.size()) {}

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

    template <typename U, Meta::TEnableIf<std::is_assignable_v<T*&, U*>>* = nullptr>
    CONSTEXPR TMemoryView(const TMemoryView<U>& other) NOEXCEPT;
    template <typename U, Meta::TEnableIf<std::is_assignable_v<T*&, U*>>* = nullptr>
    CONSTEXPR TMemoryView& operator =(const TMemoryView<U>& other) NOEXCEPT;

    CONSTEXPR CONSTF pointer Pointer() const { return _storage; }
    CONSTEXPR CONSTF size_t SizeInBytes() const { return _size * sizeof(T); }
    CONSTEXPR CONSTF size_t StrideInBytes() const { return sizeof(T); }

    CONSTEXPR CONSTF pointer data() const { return _storage; }
    CONSTEXPR CONSTF size_type size() const { return _size; }
    CONSTEXPR CONSTF bool empty() const { return 0 == _size; }

    CONSTEXPR iterator begin() const { return MakeCheckedIterator(_storage, _size, 0); }
    CONSTEXPR iterator end() const { return MakeCheckedIterator(_storage, _size, _size); }

    CONSTEXPR iterator cbegin() const { return begin(); }
    CONSTEXPR iterator cend() const { return end(); }

    CONSTEXPR reverse_iterator rbegin() const { return reverse_iterator(end()); }
    CONSTEXPR reverse_iterator rend() const { return reverse_iterator(begin()); }

    CONSTEXPR reference at(size_type index) const;
    CONSTEXPR reference operator [](size_type index) const { return at(index); }

    CONSTEXPR reference front() const { return at(0); }
    CONSTEXPR reference back() const { return at(_size - 1); }

    CONSTEXPR void CopyTo(const TMemoryView<Meta::TRemoveConst<T>>& dst) const;

    template <size_t _Dim>
    CONSTEXPR void CopyTo(Meta::TRemoveConst<T> (&dst)[_Dim], size_t offset = 0) const {
        Assert(_Dim >= offset + _size);
        CopyTo(TMemoryView<Meta::TRemoveConst<T>>(dst + offset, _size));
    }

    CONSTEXPR TMemoryView Eat(size_t n) {
        AssertRelease(n <= _size);
        const TMemoryView eaten{ _storage, n };
        _storage += n;
        _size -= n;
        return eaten;
    }

    CONSTEXPR bool Eat(const TMemoryView& other) {
        if (StartsWith(other)) {
            Eat(other.size());
            return true;
        }
        return false;
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
    auto* Peek() const {
        Assert_NoAssume(sizeof(U) <= _size);
        using result_t = Meta::TConditional<std::is_const_v<T>, std::add_const_t<U>, U>;
        return reinterpret_cast<result_t*>(_storage);
    }

    NODISCARD CONSTEXPR TMemoryView Slice(size_t index, size_t stride) const;
    NODISCARD CONSTEXPR TMemoryView< Meta::TAddConst<T> > SliceConst(size_t index, size_t stride) const;

    NODISCARD CONSTEXPR TMemoryView SubRange(size_t offset, size_t count) const;
    NODISCARD CONSTEXPR TMemoryView< Meta::TAddConst<T> > SubRangeConst(size_t offset, size_t count) const;

    NODISCARD CONSTEXPR TMemoryView SubRange(iterator first, iterator last) const;
    NODISCARD CONSTEXPR TMemoryView< Meta::TAddConst<T> > SubRangeConst(iterator first, iterator last) const;

    NODISCARD CONSTEXPR TMemoryView CutStartingAt(size_t offset) const { return SubRange(offset, _size - offset); }
    NODISCARD CONSTEXPR TMemoryView< Meta::TAddConst<T> > CutStartingAtConst(size_t offset) const { return SubRangeConst(offset, _size - offset); }

    NODISCARD CONSTEXPR TMemoryView CutStartingAt(iterator it) const {
        Assert_NoAssume(end() == it || AliasesToContainer(it));
        return (end() != it
            ? TMemoryView(std::addressof(*it), std::distance(it, end()))
            : TMemoryView(_storage+_size, size_type(0)) );
    }

    NODISCARD CONSTEXPR TMemoryView CutStartingAt(reverse_iterator it) const {
        Assert_NoAssume(rend() == it || AliasesToContainer(it));
        return (rend() != it
            ? TMemoryView(std::addressof(*it), _storage + _size - std::addressof(*it))
            : TMemoryView(_storage, size_type(0)) );
    }

    NODISCARD CONSTEXPR TMemoryView CutBefore(size_t offset) const { return SubRange(0, offset); }
    NODISCARD CONSTEXPR TMemoryView< Meta::TAddConst<T> > CutBeforeConst(size_t offset) const { return SubRangeConst(0, offset); }

    NODISCARD CONSTEXPR TMemoryView CutBefore(iterator it) const {
        Assert_NoAssume(end() == it || AliasesToContainer(it));
        return TMemoryView(_storage, std::distance(begin(), it));
    }

    NODISCARD CONSTEXPR TMemoryView CutBefore(reverse_iterator it) const {
        Assert_NoAssume(rend() == it || AliasesToContainer(it));
        return TMemoryView(_storage, std::addressof(*it) - _storage);
    }

    NODISCARD CONSTEXPR TMemoryView FirstNElements(size_t count) const { return CutBefore(count); }
    NODISCARD CONSTEXPR TMemoryView LastNElements(size_t count) const { Assert(_size >= count); return CutStartingAt(_size - count); }

    NODISCARD CONSTEXPR TMemoryView TrimFirstNElements(size_t count) const { return CutStartingAt(count); }
    NODISCARD CONSTEXPR TMemoryView TrimLastNElements(size_t count) const { Assert(_size >= count); return CutBefore(_size - count); }

    NODISCARD CONSTEXPR TMemoryView ShiftBack(const size_type n = 1) const { Assert(_size >= n); return TMemoryView(_storage, _size - n); }
    NODISCARD CONSTEXPR TMemoryView ShiftFront(const size_type n = 1) const { Assert(_size >= n); return TMemoryView(_storage + n, _size - n); }

    NODISCARD TMemoryView GrowBack(const size_type n = 1) const { return TMemoryView(_storage, _size + n); }
    NODISCARD TMemoryView GrowFront(const size_type n = 1) const { return TMemoryView(_storage - n, _size + n); }

    template <typename U>
    CONSTEXPR bool IsSubRangeOf(const TMemoryView<U>& parent) const {
        return (static_cast<const void*>(parent.data()) <= static_cast<const void*>(_storage) &&
                static_cast<const void*>(parent.data() + parent.size()) >= static_cast<const void*>(_storage + _size));
    }

    CONSTEXPR iterator Find(const T& elem) const { return std::find(begin(), end(), elem); }
    CONSTEXPR iterator FindAfter(const T& elem, iterator first) const {
        Assert(AliasesToContainer(first));
        const iterator last = end();
        return (first == last ? last : std::find(++first, last, elem));
    }

    CONSTEXPR reverse_iterator FindR(const T& elem) const { return std::find(rbegin(), rend(), elem); }
    CONSTEXPR reverse_iterator FindAfterR(const T& elem, reverse_iterator rfirst) const {
        Assert(AliasesToContainer(rfirst));
        const reverse_iterator rlast = rend();
        return (rfirst == rlast ? rlast : std::find(++rfirst, rlast, elem));
    }

    CONSTEXPR bool Contains(const T& elem) const { return (end() != Find(elem)); }
    CONSTEXPR bool ContainsRef(TPtrRef<const T> p) const {
        return (end() != FindIf([p](const T& elem) {
            return (MakePtrRef(elem) == p);
        }));
    }

    auto MakeRef(iterator it) const { return (it != end() ? MakePtrRef(*it) : nullptr); }

    template <typename _Pred>
    CONSTEXPR auto Any(const _Pred& pred) const { return MakeRef(FindIf(pred)); }
    template <typename _Pred = Meta::TLess<Meta::TDecay<T>>>
    CONSTEXPR auto Min(const _Pred& pred = _Pred{}) { return MakeRef(FindMin(pred)); }
    template <typename _Pred = Meta::TLess<Meta::TDecay<T>>>
    CONSTEXPR auto Max(const _Pred& pred = _Pred{}) { return MakeRef(FindMax(pred)); }

    template <typename _Pred>
    CONSTEXPR iterator FindIf(const _Pred& pred) const { return std::find_if(begin(), end(), pred); }
    template <typename _Pred>
    CONSTEXPR size_type FindFirst(const _Pred& pred) const { return std::distance(begin(), FindIf(pred)); }

    template <typename _Pred>
    CONSTEXPR reverse_iterator FindIfR(const _Pred& pred) const { return std::find_if(rbegin(), rend(), pred); }
    template <typename _Pred>
    CONSTEXPR size_type FindLast(const _Pred& pred) const { return std::distance(rbegin(), FindIfR(pred)); }

    template <typename _Pred>
    CONSTEXPR iterator FindIfNot(const _Pred& pred) const { return std::find_if_not(begin(), end(), pred); }
    template <typename _Pred>
    CONSTEXPR size_type FindFirstNot(const _Pred& pred) const { return std::distance(begin(), FindIfNot(pred)); }

    template <typename _Pred>
    CONSTEXPR reverse_iterator FindIfNotR(const _Pred& pred) const { return std::find_if_not(rbegin(), rend(), pred); }
    template <typename _Pred>
    CONSTEXPR size_type FindLastNot(const _Pred& pred) const { return std::distance(rbegin(), FindIfNotR(pred)); }

    template <typename _Pred>
    CONSTEXPR iterator FindMin(const _Pred& pred = Meta::TLess<Meta::TDecay<T>>{}) const { return std::min_element(begin(), end(), pred); }
    template <typename _Pred>
    CONSTEXPR iterator FindMax(const _Pred& pred = Meta::TLess<Meta::TDecay<T>>{}) const { return std::max_element(begin(), end(), pred); }

    CONSTEXPR iterator FindSubRange(const TMemoryView& subrange) const;

    CONSTEXPR bool EndsWith(const T& suffix) const;
    CONSTEXPR bool StartsWith(const T& prefix) const;

    CONSTEXPR bool EndsWith(const TMemoryView& suffix) const;
    CONSTEXPR bool StartsWith(const TMemoryView& prefix) const;

    CONSTEXPR bool RangeEqual(TMemoryView other) const {
        return std::equal(begin(), end(), other.begin(), other.end());
    }
    template <typename U, typename _Pred = Meta::TEqualTo<T>, Meta::TEnableIf<std::is_assignable_v<T*&, U*>>* = nullptr>
    CONSTEXPR bool RangeEqual(TMemoryView<U> other, _Pred pred = Default) const {
        return std::equal(begin(), end(), other.begin(), other.end(), pred);
    }

    NODISCARD CONSTEXPR TMemoryView Concat(const TMemoryView& other) const;
    NODISCARD CONSTEXPR TMemoryView Concat_AssumeNotEmpty(const TMemoryView& other) const;

    template <typename _Pred>
    NODISCARD CONSTEXPR TMemoryView SplitIf(const _Pred& pred) const { return TMemoryView(_storage, FindFirst(pred)); }
    template <typename _Pred>
    NODISCARD CONSTEXPR TMemoryView SplitIfNot(const _Pred& pred) const { return TMemoryView(_storage, FindFirstNot(pred)); }

    CONSTEXPR bool AliasesToContainer(const_reference v) const { return (_storage <= &v && _storage + _size > &v); }
    CONSTEXPR bool AliasesToContainer(iterator it) const { return (begin() <= it && it < end()); }
    CONSTEXPR bool AliasesToContainer(reverse_iterator it) const { return (rbegin() <= it && it < rend()); }

    template <typename U>
    NODISCARD TMemoryView<U> Cast() const;
    NODISCARD TMemoryView<const u8> RawView() const { return Cast<const u8>(); }

    // implicit cast to TIterable<>
    CONSTEXPR operator TIterable<iterator>() const {
        return Iterable();
    }
    CONSTEXPR TIterable<iterator> Iterable() const {
        return MakeIterable(begin(), end());
    }
    template <typename _Transform>
    CONSTEXPR auto Map(const _Transform& transform) const {
        return Iterable().Map(transform);
    }
    template <typename _Transform, typename _Reduce>
    CONSTEXPR auto MapReduce(const _Transform& transform, _Reduce&& reduce) const {
        return Map(transform).Reduce(std::forward<_Reduce>(reduce));
    }

    CONSTEXPR iterator Rotate(iterator around) const {
        return std::rotate(begin(), around, end());
    }
    template <typename _Pred>
    CONSTEXPR iterator StablePartition(_Pred&& pred) const {
        return std::stable_partition(begin(), end(), pred);
    }

    CONSTEXPR friend void swap(TMemoryView& lhs, TMemoryView& rhs) NOEXCEPT {
        using std::swap;
        swap(lhs._storage, rhs._storage);
        swap(lhs._size, rhs._size);
    }

    CONSTEXPR friend bool operator ==(const TMemoryView& lhs, const TMemoryView& rhs) {
        return (lhs._storage == rhs._storage &&
                lhs._size == rhs._size );
    }
    CONSTEXPR friend bool operator !=(const TMemoryView& lhs, const TMemoryView& rhs) {
        return (not operator ==(lhs, rhs));
    }

protected:
    pointer _storage{ nullptr };
    size_type _size{ 0 };
};
//----------------------------------------------------------------------------
#if PPE_HAS_CXX17
template <typename T>
TMemoryView(T* storage, size_t size) -> TMemoryView<T>;
#endif
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
} //!namespace PPE

#include "Memory/MemoryView-inl.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Meta {
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
} //!namespace Meta
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTEXPR u32 lengthof(const TMemoryView<T>& view) { return checked_cast<u32>(view.size()); }
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
NODISCARD CONSTEXPR TMemoryView<T> MakeView(T(&staticArray)[_Dim]) {
    return TMemoryView<T>(&staticArray[0], _Dim);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
NODISCARD CONSTEXPR TMemoryView<Meta::TAddConst<T> > MakeConstView(T(&staticArray)[_Dim]) {
    return TMemoryView<Meta::TAddConst<T> >(&staticArray[0], _Dim);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
NODISCARD CONSTEXPR TMemoryView<T> MakeView(std::array<T, _Dim>& arr) {
    return TMemoryView<T>(arr.data(), _Dim);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
NODISCARD CONSTEXPR TMemoryView<const T> MakeView(const std::array<T, _Dim>& arr) {
    return TMemoryView<const T>(arr.data(), _Dim);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
NODISCARD CONSTEXPR TMemoryView<Meta::TAddConst<T> > MakeConstView(const std::array<T, _Dim>& arr) {
    return TMemoryView<Meta::TAddConst<T> >(arr.data(), _Dim);
}
//----------------------------------------------------------------------------
template <typename _It>
NODISCARD
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
NODISCARD TMemoryView<typename _VectorLike::value_type> MakeView(_VectorLike& container) {
    if (container.begin() != container.end())
        return TMemoryView<typename _VectorLike::value_type>(std::addressof(*std::begin(container)), std::distance(std::begin(container), std::end(container)) );
    else
        return TMemoryView<typename _VectorLike::value_type>();
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
NODISCARD TMemoryView<const typename _VectorLike::value_type> MakeView(const _VectorLike& container) {
    if (container.begin() != container.end())
        return TMemoryView<const typename _VectorLike::value_type>(std::addressof(*std::begin(container)), std::distance(std::begin(container), std::end(container)) );
    else
        return TMemoryView<const typename _VectorLike::value_type>();
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
NODISCARD TMemoryView<const typename _VectorLike::value_type> MakeConstView(const _VectorLike& container) {
    if (container.begin() != container.end())
        return TMemoryView<const typename _VectorLike::value_type>(std::addressof(*std::begin(container)), std::distance(std::begin(container), std::end(container)) );
    else
        return TMemoryView<const typename _VectorLike::value_type>();
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTEXPR TMemoryView< T > MakeView(T* pbegin, T* pend) {
    Assert(pend >= pbegin);
    return TMemoryView< T >(pbegin, std::distance(pbegin, pend));
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTEXPR TMemoryView<Meta::TAddConst<T> > MakeConstView(T* pbegin, T* pend) {
    Assert(pend >= pbegin);
    return TMemoryView<Meta::TAddConst<T> >(pbegin, std::distance(pbegin, pend));
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTF TMemoryView<Meta::TEnableIf<std::is_standard_layout_v<T>, u8>> MakePodView(T& assumePod) {
    return { reinterpret_cast<u8*>(&assumePod), sizeof(T) };
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTF TMemoryView<Meta::TEnableIf<std::is_standard_layout_v<T>, const u8>> MakePodView(const T& assumePod) {
    return { reinterpret_cast<const u8*>(&assumePod), sizeof(T) };
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTF TMemoryView<Meta::TEnableIf<std::is_standard_layout_v<T>, const u8>> MakePodConstView(const T& assumePod) {
    return { reinterpret_cast<const u8*>(&assumePod), sizeof(T) };
}
//----------------------------------------------------------------------------
NODISCARD inline TMemoryView<u8> MakeRawView(void* data, size_t sizeInBytes) {
    return TMemoryView<u8>{ static_cast<u8*>(data), sizeInBytes };
}
//----------------------------------------------------------------------------
NODISCARD inline TMemoryView<const u8> MakeRawView(const void* data, size_t sizeInBytes) {
    return TMemoryView<const u8>{ static_cast<const u8*>(data), sizeInBytes };
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TMemoryView<u8> MakeRawView(const TMemoryView<T>& assumePods) {
    return assumePods.template Cast<u8>();
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TMemoryView<const u8> MakeRawView(const TMemoryView<const T>& assumePods) {
    return assumePods.template Cast<const u8>();
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TMemoryView<const u8> MakeRawConstView(const TMemoryView<T>& assumePods) {
    return assumePods.template Cast<const u8>();
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
NODISCARD auto MakeRawView(T(&staticArray)[_Dim]) {
    return MakeRawView(MakeView(staticArray));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
NODISCARD auto MakeRawConstView(T(&staticArray)[_Dim]) {
    return MakeRawConstView(MakeView(staticArray));
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TMemoryView<Meta::TRemoveConst<T>> RemoveConstView(const TMemoryView<T>& view) {
    return TMemoryView<Meta::TRemoveConst<T>>(const_cast<Meta::TRemoveConst<T>*>(view.data()), view.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename U, typename V, std::decay_t<decltype(std::declval<std::add_lvalue_reference_t<U>>() = std::declval<std::add_rvalue_reference_t<V>>())>* = nullptr>
CONSTEXPR void Broadcast(const TMemoryView<U>& dst, V&& value) {
    std::fill(dst.begin(), dst.end(), std::forward<V>(value));
}
//----------------------------------------------------------------------------
template <typename U, typename V, std::decay_t<decltype(std::declval<std::add_lvalue_reference_t<U>>() = std::declval<std::add_rvalue_reference_t<V>>())>* = nullptr>
CONSTEXPR void Copy(const TMemoryView<U>& dst, const TMemoryView<V>& src) {
    Assert(dst.size() == src.size());
    IF_CONSTEXPR(std::is_same_v<U, V>) {
        src.CopyTo(dst); // CopyTo<>() is specialized to use memcpy for raw memory views
    }
    else {
        std::copy(src.begin(), src.end(), dst.begin());
    }
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR void Move(const TMemoryView<T>& dst, const TMemoryView<T>& src) {
    Assert(dst.size() == src.size());
    std::move(src.begin(), src.end(), dst.begin());
}
//----------------------------------------------------------------------------
template <typename T, typename _Lambda, decltype(std::declval<_Lambda&&>()(std::declval<size_t>(), std::declval<T*>()))* = nullptr>
CONSTEXPR void Collect(const TMemoryView<T>& dst, _Lambda&& collect) {
    forrange(i, 0, dst.size())
        collect(i, &dst[i]);
}
//----------------------------------------------------------------------------
template <typename T, typename U, decltype(std::declval<std::add_rvalue_reference_t<T>>() == std::declval<std::add_rvalue_reference_t<U>>())* = nullptr>
NODISCARD CONSTEXPR bool Contains(const TMemoryView<T>& v, const U& elt) {
    return (v.end() != std::find(v.begin(), v.end(), elt));
}
//----------------------------------------------------------------------------
template <typename T, Meta::TEnableIf<std::is_trivial_v<T>>* = nullptr>
NODISCARD CONSTEXPR int Memcmp(const TMemoryView<T>& lhs, const TMemoryView<T>& rhs) {
    const int cmp = std::memcmp(lhs.data(), rhs.data(),
        Min(lhs.SizeInBytes(), rhs.SizeInBytes()));
    if (cmp != 0)
        return cmp;
    return (lhs.size() < rhs.size() ? -1 : (lhs.size() == rhs.size() ? 0 : 1));
}
//----------------------------------------------------------------------------
template <typename T, typename U, decltype(std::declval<std::add_rvalue_reference_t<T>>() == std::declval<std::add_rvalue_reference_t<U>>())* = nullptr>
NODISCARD CONSTEXPR size_t IndexOf(const TMemoryView<T>& v, const U& elt) {
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
