#pragma once

#include "Core_fwd.h"

#include "IO/TextWriter_fwd.h"

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <type_traits>

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

    CONSTEXPR TMemoryView();
    CONSTEXPR TMemoryView(pointer storage, size_type size);

    // enables type promotion between {T(),T(),T()} and TMemoryView<T>
    CONSTEXPR TMemoryView(std::initializer_list<T> list)
        : TMemoryView(list.begin(), std::distance(list.begin(), list.end())) {}

    // enables type promotion between T[] and TMemoryView<T>
    template <size_t _Dim>
    CONSTEXPR TMemoryView(value_type (&staticArray)[_Dim])
        : TMemoryView(staticArray, _Dim) {}

    TMemoryView(const iterator& first, const iterator& last)
        : TMemoryView(std::addressof(*first), std::distance(first, last)) {}

    CONSTEXPR TMemoryView(TMemoryView&& rvalue);
    CONSTEXPR TMemoryView& operator =(TMemoryView&& rvalue);

    CONSTEXPR TMemoryView(const TMemoryView& other);
    CONSTEXPR TMemoryView& operator =(const TMemoryView& other);

    template <typename U>
    CONSTEXPR TMemoryView(const TMemoryView<U>& other);
    template <typename U>
    CONSTEXPR TMemoryView& operator =(const TMemoryView<U>& other);

    pointer Pointer() const { return _storage; }
    size_t SizeInBytes() const { return _size * sizeof(T); }
    size_t StrideInBytes() const { return sizeof(T); }

    pointer data() const { return _storage; }
    size_type size() const { return _size; }
    bool empty() const { return 0 == _size; }

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

    TMemoryView<T> Slice(size_t index, size_t stride) const;
    TMemoryView< Meta::TAddConst<T> > SliceConst(size_t index, size_t stride) const;

    TMemoryView<T> SubRange(size_t offset, size_t count) const;
    TMemoryView< Meta::TAddConst<T> > SubRangeConst(size_t offset, size_t count) const;

    TMemoryView<T> SubRange(iterator first, iterator last) const;
    TMemoryView< Meta::TAddConst<T> > SubRangeConst(iterator first, iterator last) const;

    TMemoryView<T> CutStartingAt(size_t offset) const { return SubRange(offset, _size - offset); }
    TMemoryView< Meta::TAddConst<T> > CutStartingAtConst(size_t offset) const { return SubRangeConst(offset, _size - offset); }

    TMemoryView<T> CutStartingAt(const iterator& it) const {
        Assert_NoAssume(end() == it || AliasesToContainer(it));
        return (end() != it
            ? TMemoryView(std::addressof(*it), std::distance(it, end()))
            : TMemoryView(_storage+_size, size_type(0)) );
    }

    TMemoryView<T> CutStartingAt(const reverse_iterator& it) const {
        Assert_NoAssume(rend() == it || AliasesToContainer(it));
        return (rend() != it
            ? TMemoryView(std::addressof(*it), _storage + _size - std::addressof(*it))
            : TMemoryView(_storage, size_type(0)) );
    }

    TMemoryView<T> CutBefore(size_t offset) const { return SubRange(0, offset); }
    TMemoryView< Meta::TAddConst<T> > CutBeforeConst(size_t offset) const { return SubRangeConst(0, offset); }

    TMemoryView<T> CutBefore(const iterator& it) const {
        Assert_NoAssume(end() == it || AliasesToContainer(it));
        return TMemoryView<T>(_storage, std::distance(begin(), it));
    }

    TMemoryView<T> CutBefore(const reverse_iterator& it) const {
        Assert_NoAssume(rend() == it || AliasesToContainer(it));
        return TMemoryView<T>(_storage, std::addressof(*it) - _storage);
    }

    TMemoryView<T> FirstNElements(size_t count) const { return CutBefore(count); }
    TMemoryView<T> LastNElements(size_t count) const { Assert(_size >= count); return CutStartingAt(_size - count); }

    TMemoryView<T> TrimFirstNElements(size_t count) const { return CutStartingAt(count); }
    TMemoryView<T> TrimLastNElements(size_t count) const { Assert(_size >= count); return CutBefore(_size - count); }

    TMemoryView<T> ShiftBack() const { Assert(_size > 0); return TMemoryView<T>(_storage, _size - 1); }
    TMemoryView<T> ShiftFront() const { Assert(_size > 0); return TMemoryView<T>(_storage + 1, _size - 1); }

    TMemoryView<T> GrowBack() const { return TMemoryView<T>(_storage, _size + 1); }
    TMemoryView<T> GrowFront() const { return TMemoryView<T>(_storage - 1, _size + 1); }

    template <typename U>
    bool IsSubRangeOf(const TMemoryView<U>& parent) const {
        return ((void*)parent.data() <= (void*)_storage &&
                (void*)(parent.data()+parent.size()) >= (void*)(_storage+_size));
    }

    iterator Find(const T& elem) const { return std::find(begin(), end(), elem); }
    reverse_iterator FindR(const T& elem) const { return std::find(rbegin(), rend(), elem); }

    bool Contains(const T& elem) const { return (end() != Find(elem)); }

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

    iterator FindSubRange(const TMemoryView<T>& subrange) const;

    bool EndsWith(const T& suffix) const;
    bool StartsWith(const T& prefix) const;

    bool EndsWith(const TMemoryView<T>& suffix) const;
    bool StartsWith(const TMemoryView<T>& prefix) const;

    template <typename _Pred>
    TMemoryView SplitIf(const _Pred& pred) const { return TMemoryView(_storage, FindFirst(pred)); }
    template <typename _Pred>
    TMemoryView SplitIfNot(const _Pred& pred) const { return TMemoryView(_storage, FindFirstNot(pred)); }

    template <typename _Lambda, class = Meta::TEnableIf<not std::is_const_v<T>> >
    const TMemoryView& Collect(_Lambda&& collect) const {
        forrange(i, 0, _size)
            collect(i, &_storage[i]);
        return (*this);
    }

    bool AliasesToContainer(const_reference v) const { return (_storage <= &v && _storage + _size > &v); }
#if USE_PPE_CHECKEDARRAYITERATOR
    bool AliasesToContainer(const iterator& it) const { return (begin() <= it && it < end()); }
    bool AliasesToContainer(const reverse_iterator& it) const { return (rbegin() <= it && it < rend()); }
#endif

    template <typename U>
    TMemoryView<U> Cast() const;
    TMemoryView<const u8> RawView() const { return Cast<const u8>(); }

    TMemoryView<Meta::TAddConst<value_type>> AddConst() const {
        return TMemoryView<Meta::TAddConst<value_type>>(_storage, _size);
    }

    TMemoryView<Meta::TRemoveConst<value_type>> RemoveConst() const {
        typedef TMemoryView<Meta::TRemoveConst<value_type>> nonconst_type;
        return nonconst_type(const_cast<typename nonconst_type::pointer>(_storage), _size);
    }

    template <typename _Transform>
    auto Map(const _Transform& transform) const {
        return MakeIterable(
            MakeOutputIterator(begin(), transform),
            MakeOutputIterator(end(), transform) );
    }

    friend void swap(TMemoryView& lhs, TMemoryView& rhs) {
        std::swap(lhs._storage, rhs._storage);
        std::swap(lhs._size, rhs._size);
    }

    friend bool operator ==(const TMemoryView& lhs, const TMemoryView& rhs) {
        return  lhs._storage == rhs._storage &&
                lhs._size == rhs._size;
    }

    friend bool operator !=(const TMemoryView& lhs, const TMemoryView& rhs) {
        return false == operator ==(lhs, rhs);
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
TMemoryView< T > MakeView(T* pbegin, T* pend) {
    Assert(pend >= pbegin);
    return TMemoryView< T >(pbegin, std::distance(pbegin, pend));
}
//----------------------------------------------------------------------------
template <typename T>
TMemoryView<Meta::TAddConst<T> > MakeConstView(T* pbegin, T* pend) {
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
    return assumePods.Cast<u8>();
}
//----------------------------------------------------------------------------
template <typename T>
TMemoryView<const u8> MakeRawView(const T& assumePod) {
    return TMemoryView<const u8>(reinterpret_cast<const u8*>(&assumePod), sizeof(T));
}
//----------------------------------------------------------------------------
template <typename T>
TMemoryView<const u8> MakeRawView(const TMemoryView<const T>& assumePods) {
    return assumePods.Cast<const u8>();
}
//----------------------------------------------------------------------------
template <typename T>
TMemoryView<const u8> MakeRawConstView(const T& assumePod) {
    return TMemoryView<const u8>(reinterpret_cast<const u8*>(&assumePod), sizeof(T));
}
//----------------------------------------------------------------------------
template <typename T>
TMemoryView<const u8> MakeRawConstView(const TMemoryView<T>& assumePods) {
    return assumePods.Cast<const u8>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename U, typename V>
void Copy(const TMemoryView<U>& dst, const TMemoryView<V>& src) {
    Assert(dst.size() == src.size());
    std::copy(src.begin(), src.end(), dst.begin());
}
//----------------------------------------------------------------------------
template <typename T>
void Move(const TMemoryView<T>& dst, const TMemoryView<T>& src) {
    Assert(dst.size() == src.size());
    std::move(src.begin(), src.end(), dst.begin());
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
