#pragma once

#include "Container/Vector.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TVector<T, _Allocator>::operator =(const TVector& other) -> TVector& {
    if (this == &other)
        return *this;

    typedef typename allocator_traits::propagate_on_container_copy_assignment propagate_type;
    allocator_copy_(other, propagate_type());

    assign(other.begin(), other.end());
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TVector<T, _Allocator>::operator =(TVector&& rvalue) NOEXCEPT -> TVector& {
    if (this == &rvalue)
        return *this;

    assign(std::move(rvalue));
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::allocator_copy_(const allocator_type& other, std::true_type ) {
    if (not allocator_traits::Equals(*this, other)) {
        clear_ReleaseMemory();
        Assert(nullptr == _data);
        Assert(0 == _numElements);
        Assert(0 == _allocationSize);
        allocator_traits::Copy(this, other);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::assign(TVector&& rvalue) {
    Assert(&rvalue != this);

    if (MoveAllocatorBlock(&allocator_traits::Get(*this), allocator_traits::Get(rvalue), rvalue.allocator_block_())) {
        clear_ReleaseMemory();

        _data = rvalue._data;
        _numElements = rvalue._numElements;
        _allocationSize = rvalue._allocationSize;

        rvalue._data = nullptr;
        rvalue._allocationSize = rvalue._numElements = 0;
    }
    else {
        assign(MakeMoveIterator(rvalue.begin()), MakeMoveIterator(rvalue.end()) );

        rvalue.clear_ReleaseMemory();
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
auto TVector<T, _Allocator>::assign(_It first, _It last)
    -> Meta::TEnableIf<Meta::is_iterator_v<_It>>  {
#if USE_PPE_DEBUG
    IF_CONSTEXPR(std::is_same_v<Meta::TDecay<_It>, iterator> || std::is_same_v<Meta::TDecay<_It>, const_iterator>) {
        Assert_NoAssume(first == last || not AliasesToContainer(*first));
    }
#endif

    typedef typename std::iterator_traits<_It>::iterator_category iterator_category;
    assign_(first, last, iterator_category{});

    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
void TVector<T, _Allocator>::assign_(_It first, _It last, std::input_iterator_tag ) {
    u32 count = 0;
    for (; count < _numElements && first != last; ++count, ++first)
        _data[count] = (*first);

    Assert(first == last || count == _numElements);
    for (; first != last; ++first) {
        Assert(count == _numElements);
        reserve_AtLeast(++count);
        Meta::Construct(push_back_Uninitialized(), *first);
    }

    if (_numElements > count) {
        Meta::Destroy(MakeView().CutStartingAt(count));
        _numElements = count;
    }
    else {
        Assert_NoAssume(_numElements == count);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It, typename _ItCat>
void TVector<T, _Allocator>::assign_(_It first, _It last, _ItCat ) {
    const u32 count = checked_cast<u32>(std::distance(first, last));

    if (_numElements >= count) {
        Assert_NoAssume(count <= capacity());

        std::copy(first, last, MakeCheckedIterator(_data, count, 0));
        Meta::Destroy(MakeView().CutStartingAt(count));
    }
    else {
        if (count * sizeof(T) <= _allocationSize) {
            _It pivot = first;
            std::advance(pivot, _numElements);
            std::copy(first, pivot, MakeCheckedIterator(_data, capacity(), 0));
            std::uninitialized_copy(pivot, last, MakeCheckedIterator(_data, capacity(), _numElements));
        }
        else {
            clear_ReleaseMemory();
            reserve_Exactly(count);

            Assert_NoAssume(_numElements == 0);
            std::uninitialized_copy(first, last, MakeCheckedIterator(_data, capacity(), 0));
        }
    }

    _numElements = count;
}

//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
auto TVector<T, _Allocator>::assign_AssumeEmpty(_It first, _It last)
    -> Meta::TEnableIf<Meta::is_iterator_v<_It>>  {
#if USE_PPE_DEBUG
    IF_CONSTEXPR(std::is_same_v<Meta::TDecay<_It>, iterator> || std::is_same_v<Meta::TDecay<_It>, const_iterator>) {
        Assert_NoAssume(first == last || not AliasesToContainer(*first));
    }
#endif

    const u32 n = checked_cast<u32>(std::distance(first, last));
    reserve_AssumeEmpty(n);
    Assert_NoAssume(n * sizeof(T) <= _allocationSize);

    auto dest = MakeCheckedIterator(_data, capacity(), 0);
    IF_CONSTEXPR(std::is_rvalue_reference_v<typename Meta::TIteratorTraits<_It>::reference>)
        std::uninitialized_move(first, last, dest);
    else
        std::uninitialized_copy(first, last, dest);

    _numElements = n;

    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::assign(size_type count, const_reference value) {
    Assert_NoAssume(not AliasesToContainer(value));

    if (count * sizeof(T) <= _allocationSize) {
        Meta::Destroy(MakeView());

        const u32 ucount = checked_cast<u32>(count);

        auto dest = MakeCheckedIterator(_data, capacity(), 0);
        std::fill_n(dest, Min(_numElements, ucount), value);

        if (_numElements < ucount) {
            std::advance(dest, _numElements);
            std::uninitialized_fill_n(dest, count - _numElements, value);
        }
        else {
            Meta::Destroy(MakeView().CutStartingAt(count));
        }
    }
    else {
        clear_ReleaseMemory();
        reserve_Exactly(count);

        Assert_NoAssume(_numElements == 0 && count * sizeof(T) <= _allocationSize);
        std::uninitialized_fill_n(MakeCheckedIterator(_data, capacity(), 0), count, value);
    }

    _numElements = checked_cast<u32>(count);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <class... _Args>
auto TVector<T, _Allocator>::emplace(const_iterator pos, _Args&&... args) -> iterator {
    Assert_NoAssume(cend() == pos || AliasesToContainer(pos));

    const size_type i = std::distance<const_iterator>(begin(), pos);
    emplace_back(std::forward<_Args>(args)...);

    std::rotate(begin() + i, begin() + (_numElements - 1), end());

    return MakeCheckedIterator(_data, _numElements, i);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <class... _Args>
void TVector<T, _Allocator>::emplace_back(_Args&&... args) {
    reserve_Additional(1);
    emplace_back_AssumeNoGrow(std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::emplace_back(const_reference value) {
    Assert_NoAssume(not AliasesToContainer(value));

    reserve_Additional(1);
    emplace_back_AssumeNoGrow(value);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::emplace_back(T&& rvalue) {
    Assert_NoAssume(not AliasesToContainer(rvalue));

    reserve_Additional(1);
    emplace_back_AssumeNoGrow(std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <class... _Args>
void TVector<T, _Allocator>::emplace_back_AssumeNoGrow(_Args&&... args) {
    Assert_NoAssume(_numElements < capacity());

    Meta::Construct(&_data[_numElements++], std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::emplace_back_AssumeNoGrow(const_reference value) {
    Assert_NoAssume(_numElements < capacity());

    Meta::Construct(&_data[_numElements++], value);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::emplace_back_AssumeNoGrow(T&& rvalue) {
    Assert_NoAssume(_numElements < capacity());

    Meta::Construct(&_data[_numElements++], std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::push_back_AssumeNoGrow(const_reference value) {
    AssertRelease_NoAssume(_numElements < capacity());

    Meta::Construct(&_data[_numElements++], value);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::push_back_AssumeNoGrow(T&& rvalue) {
    AssertRelease_NoAssume(_numElements < capacity());

    Meta::Construct(&_data[_numElements++], std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TVector<T, _Allocator>::erase(const_iterator pos) -> iterator {
    Assert_NoAssume(cend() == pos || AliasesToContainer(pos));

    const size_t index = std::distance(cbegin(), pos);
    {
        const iterator it = begin() + index;
        std::rotate(it, it + 1, end());
    }

    pop_back();
    return (begin() + index);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TVector<T, _Allocator>::erase(const_iterator first, const_iterator last) -> iterator {
    Assert_NoAssume(AliasesToContainer(first));
    Assert_NoAssume(AliasesToContainer(last));
    Assert(first <= last);

    const size_type p = std::distance(cbegin(), first);
    const size_type n = std::distance(first, last);
    Assert_NoAssume(p + n <= _numElements);

    std::rotate((const iterator&)(first), (const iterator&)(last), end());

    Meta::Destroy(MakeView().LastNElements(n));

    _numElements = checked_cast<u32>(_numElements - n);
    return MakeCheckedIterator(_data, _numElements, p);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::erase_DontPreserveOrder(const_iterator pos) {
    Assert_NoAssume(AliasesToContainer(pos));
    Assert(_numElements > 0);

    const iterator& p = (const iterator&)(pos);
    const iterator b = end() - 1;
    if (p != b)
        std::iter_swap(p, b);

    pop_back();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
auto TVector<T, _Allocator>::insert(const_iterator pos, _It first, _It last)
    -> Meta::TEnableIf<Meta::is_iterator_v<_It>, iterator> {
    typedef typename std::iterator_traits<_It>::iterator_category iterator_category;
    const iterator it = insert_(pos, first, last, iterator_category{});
    Assert(CheckInvariants());
    return it;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
auto TVector<T, _Allocator>::insert_(const_iterator pos, _It first, _It last, std::input_iterator_tag ) -> iterator {
    Assert_NoAssume(end() == pos || AliasesToContainer(pos));
    Assert_NoAssume(not AliasesToContainer(std::addressof(*first)));
    Assert_NoAssume(not AliasesToContainer(std::addressof(*last)));

    const size_type p = std::distance<const_iterator>(begin(), pos);
    const u32 o = _numElements;

    u32 count = 0;
    for (; first != last; ++count, ++first) {
        Assert_NoAssume(_numElements == o + count);

        reserve_AtLeast(static_cast<size_t>(_numElements) + 1);
        Meta::Construct(push_back_Uninitialized(), *first);
    }
    Assert_NoAssume(p <= count);
    Assert(o + count == _numElements);

    std::rotate(begin() + p, begin() + o, begin() + (o + count));
    return MakeCheckedIterator(_data, _numElements, p);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It, typename _ItCat>
auto TVector<T, _Allocator>::insert_(const_iterator pos, _It first, _It last, _ItCat ) -> iterator {
    Assert_NoAssume(cend() == pos || AliasesToContainer(pos));

    const u32 count = checked_cast<u32>(std::distance(first, last));
    const size_type p = std::distance<const_iterator>(begin(), pos);
    Assert_NoAssume(p <= _numElements);

    if (0 < count) {
        reserve_Additional(count);

        const u32 o = _numElements;

        _numElements = checked_cast<u32>(_numElements + count);
        Assert_NoAssume(_allocationSize >= _numElements * sizeof(T));

        std::uninitialized_copy(first, last, MakeCheckedIterator(_data, capacity(), o));
        std::rotate(begin() + p, begin() + o, begin() + _numElements);
    }
    return MakeCheckedIterator(_data, _numElements, p);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TVector<T, _Allocator>::insert(const_iterator pos, size_type count, const T& value) -> iterator {
    Assert_NoAssume(cend() == pos || AliasesToContainer(pos));
    Assert_NoAssume(not AliasesToContainer(value));

    const size_type p = std::distance<const_iterator>(begin(), pos);

    if (count) {
        reserve_Additional(count);

        const size_t o = _numElements;
        _numElements = checked_cast<u32>(_numElements + count);
        Assert_NoAssume(_allocationSize >= _numElements * sizeof(T));

        std::uninitialized_fill_n(MakeCheckedIterator(_data, capacity(), o), count, value);
        std::rotate(begin() + p, begin() + o, begin() + _numElements);
    }

    return MakeCheckedIterator(_data, capacity(), p);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::pop_back() {
    Assert(_data);
    AssertRelease(_numElements > 0);
    Meta::Destroy(&_data[--_numElements]);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TVector<T, _Allocator>::pop_back_ReturnBack() -> value_type {
    value_type result = std::move(back());
    pop_back();
    return result;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::clear() {
    Meta::Destroy(MakeView());
    _numElements = 0;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::clear_ReleaseMemory() {
    clear();
    Assert(0 == _numElements);

    if (nullptr != _data) {
        Assert(sizeof(T) <= _allocationSize);
        allocator_traits::Deallocate(allocator_(), allocator_block_());
        _data = nullptr;
        _allocationSize = 0;
    }
    Assert(0 == _allocationSize);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::reserve(size_type count) {
    if (_allocationSize < count * sizeof(T))
        reserve_Exactly(count);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::reserve_AtLeast(size_type count) {
    if (_allocationSize < count * sizeof(T)) {
        size_type newCapacity = capacity();

        // growth heuristic faster than n^2 bellow 1024 and slower above
        for(; newCapacity < count; newCapacity = ((1 + newCapacity) * 9) / 5);

        reserve_Exactly(newCapacity);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::reserve_AssumeEmpty(size_type count) {
    Assert(0 == _numElements);

    size_t newSizeInBytes = count * sizeof(T);
    if (_allocationSize < newSizeInBytes) {
        if (_data) {
            Assert(sizeof(T) <= _allocationSize);
            allocator_traits::Deallocate(allocator_(), allocator_block_());
        }

        newSizeInBytes = allocator_traits::SnapSize(allocator_(), newSizeInBytes);
        const FAllocatorBlock alloc = allocator_traits::Allocate(allocator_(), newSizeInBytes);

        _data = static_cast<pointer>(alloc.Data);
        _allocationSize = checked_cast<u32>(alloc.SizeInBytes);

        Assert(nullptr != _data);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::reserve_Exactly(size_type count) {
    if (count < _numElements)
        count = _numElements;

    const size_t newSizeInBytes = allocator_traits::SnapSize(allocator_(), count * sizeof(T));
    Assert_NoAssume(count * sizeof(T) <= newSizeInBytes);

    if (_allocationSize != newSizeInBytes) {
        FAllocatorBlock alloc = allocator_block_();

        IF_CONSTEXPR(std::is_trivially_move_constructible_v<T> || Meta::is_pod_v<T>) {
            IF_CONSTEXPR(allocator_traits::reallocate_can_fail::value) {
                VerifyRelease(allocator_traits::Reallocate(allocator_(), alloc, newSizeInBytes));
            } else {
                allocator_traits::Reallocate(allocator_(), alloc, newSizeInBytes);
            }
        }
        else {
            const FAllocatorBlock newAlloc = allocator_traits::Allocate(allocator_(), newSizeInBytes);

            std::uninitialized_move(begin(), end(), MakeCheckedIterator(pointer(newAlloc.Data), count, 0));
            Meta::Destroy(MakeView());

            allocator_traits::Deallocate(allocator_(), alloc);
            alloc = newAlloc;
        }

        _data = static_cast<pointer>(alloc.Data);
        _allocationSize = checked_cast<u32>(alloc.SizeInBytes);

        Assert(nullptr != _data);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::resize(size_type count) {
    if (_numElements == count)
        return;

    if (_numElements >= count) {
        Meta::Destroy(MakeView().CutStartingAt(count));
    }
    else {
        reserve(count);
        Meta::Construct(TMemoryView<T>(_data + _numElements, count - _numElements));
    }

    _numElements = checked_cast<u32>(count);
    Assert_NoAssume(_numElements * sizeof(T) <= _allocationSize);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::resize(size_type count, const_reference value) {
    Assert_NoAssume(not AliasesToContainer(value));

    if (_numElements == count)
        return;

    if (_numElements >= count) {
        Meta::Destroy(MakeView().CutStartingAt(count));
    }
    else {
        reserve(count);
        Meta::Construct(TMemoryView<T>(_data + _numElements, count - _numElements), value);
    }

    _numElements = checked_cast<u32>(count);
    Assert_NoAssume(_numElements * sizeof(T) <= _allocationSize);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::resize_Uninitialized(size_type count) {
    if (_numElements == count)
        return;

    if (_numElements >= count)
        Meta::Destroy(MakeView().CutStartingAt(count));
    else
        reserve(count);

    _numElements = checked_cast<u32>(count);
    Assert_NoAssume(_numElements * sizeof(T) <= _allocationSize);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::resize_AssumeEmpty(size_type count) {
    reserve_AssumeEmpty(count);
    Assert(0 == _numElements);

    Meta::Construct(TMemoryView<T>(_data, count));

    _numElements = checked_cast<u32>(count);
    Assert_NoAssume(_numElements * sizeof(T) <= _allocationSize);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::resize_AssumeEmpty(size_type count, const_reference value) {
    Assert(0 == _numElements);

    Meta::Construct(TMemoryView<T>(_data, count), value);

    _numElements = checked_cast<u32>(count);
    Assert_NoAssume(_numElements * sizeof(T) <= _allocationSize);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::shrink_to_fit() {
    if (Likely(_numElements))
        reserve_Exactly(_numElements);
    else
        clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::swap(TVector& other) NOEXCEPT {
    typedef typename allocator_traits::propagate_on_container_swap propagate_type;
    if (this != &other)
        swap_(other, propagate_type());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::swap_(TVector& other, std::true_type ) NOEXCEPT {
    using std::swap;
    swap(_data, other._data);
    swap(_numElements, other._numElements);
    swap(_allocationSize, other._allocationSize);
    allocator_traits::Swap(*this, other);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::swap_(TVector& other, std::false_type ) NOEXCEPT {
    TVector* plhs = nullptr;
    TVector* prhs = nullptr;
    if (_numElements < other._numElements) {
        plhs = this;
        prhs = &other;
    }
    else {
        plhs = &other;
        prhs = this;
    }

    Assert(plhs->_numElements <= prhs->_numElements);
    Assert(plhs->_allocationSize <= prhs->_allocationSize);
    std::swap_ranges(plhs->begin(), plhs->end(), MakeCheckedIterator(prhs->_data, prhs->capacity(), 0));

    if (plhs->_numElements != prhs->_numElements) {
        Assert(plhs->_numElements < prhs->_numElements);
        plhs->reserve(prhs->_numElements);

        const u32 n = prhs->_numElements;
        for (u32 i = plhs->_numElements; i < n; ++i) {
            Meta::Construct(plhs->_data + i, std::move(prhs->_data[i]));
            Meta::Destroy(prhs->_data + i);
        }

        std::swap(plhs->_numElements, prhs->_numElements);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool TVector<T, _Allocator>::CheckInvariants() const {
#ifndef NDEBUG
    if (nullptr == _data && (_numElements || _allocationSize))
        return false;
    if (nullptr != _data && 0 == _allocationSize)
        return false;
    if (_numElements * sizeof(T) > _allocationSize)
        return false;
#endif
    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool operator==(const TVector<T, _Allocator>& lhs, const TVector<T, _Allocator>& rhs) {
    return (lhs.size() == rhs.size())
        ? std::equal(lhs.begin(), lhs.end(), rhs.begin())
        : false;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool operator!=(const TVector<T, _Allocator>& lhs, const TVector<T, _Allocator>& rhs) {
    return false == operator==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool operator< (const TVector<T, _Allocator>& lhs, const TVector<T, _Allocator>& rhs) {
    using size_type = typename TVector<T, _Allocator>::size_type;
    const size_type k = std::min(lhs.size(), rhs.size());
    const T* plhs = lhs.data();
    const T* prhs = rhs.data();
    for (size_t i = 0; i < k; ++i)
        if (plhs[i] >= prhs[i])
            return false;
    return (lhs.size() <= rhs.size());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool operator>=(const TVector<T, _Allocator>& lhs, const TVector<T, _Allocator>& rhs) {
    return false == operator< (lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool operator> (const TVector<T, _Allocator>& lhs, const TVector<T, _Allocator>& rhs) {
    using size_type = typename TVector<T, _Allocator>::size_type;
    const size_type k = std::min(lhs.size(), rhs.size());
    const T* plhs = lhs.data();
    const T* prhs = rhs.data();
    for (size_t i = 0; i < k; ++i)
        if (plhs[i] <= prhs[i])
            return false;
    return (lhs.size() >= rhs.size());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool operator<=(const TVector<T, _Allocator>& lhs, const TVector<T, _Allocator>& rhs) {
    return false == operator> (lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Append(TVector<T, _Allocator>& v, const TMemoryView<const T>& elts) {
    v.insert(v.end(), elts.begin(), elts.end());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _It>
void Append(TVector<T, _Allocator>& v, _It first, _It last) {
    v.insert(v.end(), first, last);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _It>
void Assign(TVector<T, _Allocator>& v, _It first, _It last) {
    v.assign(first, last);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename U>
bool Contains(const TVector<T, _Allocator>& v, const U& elt) {
    return (v.end() != std::find(v.begin(), v.end(), elt));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename U>
size_t IndexOf(const TVector<T, _Allocator>& v, const U& elt) {
    const auto it = std::find(v.begin(), v.end(), elt);
    return (v.end() == it ? std::distance(v.begin(), it) : INDEX_NONE);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool FindElementIndexIFP(size_t *pIndex, TVector<T, _Allocator>& v, const Meta::TDontDeduce<T>& elt) {
    Assert(pIndex);
    const auto it = std::find(v.begin(), v.end(), elt);
    if (v.end() != it) {
        *pIndex = std::distance(v.begin(),it);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _Pred>
bool FindPredicateIndexIFP(size_t *pIndex, TVector<T, _Allocator>& v, const _Pred& pred) {
    Assert(pIndex);
    const auto it = std::find_first_of(v.begin(), v.end(), pred);
    if (v.end() != it) {
        *pIndex = std::distance(v.begin(), it);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(TVector<T, _Allocator>& v, const Meta::TDontDeduce<T>& elt) {
    Assert(!Contains(v, elt));
    v.emplace_back(elt);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(TVector<T, _Allocator>& v, Meta::TDontDeduce<T>&& elt) {
    Assert(!Contains(v, elt));
    v.emplace_back(std::move(elt));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename... _Args>
auto Emplace_Back(TVector<T, _Allocator>& v, _Args&&... args) -> typename TVector<T, _Allocator>::iterator {
    v.emplace_back(std::forward<_Args>(args)...);
    return (v.end() - 1);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Add_Unique(TVector<T, _Allocator>& v, Meta::TDontDeduce<T>&& elt) {
    if (not Contains(v, elt)) {
        v.emplace_back(std::move(elt));
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _Pred>
size_t Remove_If(TVector<T, _Allocator>& v, _Pred&& pred) {
    size_t numDeleted = 0;
    for (auto it = v.rbegin(); it != v.rend(); ) {
        if (pred(*it)) {
            numDeleted++;
            it = v.erase(it);
            continue;
        }

        ++it;
    }
    return numDeleted;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _Pred>
size_t Remove_If_DontPreserveOrder(TVector<T, _Allocator>& v, _Pred&& pred) {
    // avoid iterator checking
    size_t numDeleted = 0;
    for (size_t i = 0; i < v.size(); ) {
        if (pred(v[i])) {
            numDeleted++;
            v.erase_DontPreserveOrder(v.begin() + i);
        }
        else
            ++i;
    }
    return numDeleted;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Remove_AssertExists(TVector<T, _Allocator>& v, const Meta::TDontDeduce<T>& elt) {
    auto it = std::find(v.begin(), v.end(), elt);
    Assert(it != v.end());
    v.erase(it);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists(TVector<T, _Allocator>& v, const Meta::TDontDeduce<T>& elt) {
    auto it = std::find(v.begin(), v.end(), elt);
    if (it == v.end())
        return false;

    v.erase(it);
    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Remove_DontPreserveOrder(TVector<T, _Allocator>& v, const Meta::TDontDeduce<T>& elt) {
    auto it = std::find(v.begin(), v.end(), elt);
    Assert(v.end() != it);
    v.erase_DontPreserveOrder(it);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists_DontPreserveOrder(TVector<T, _Allocator>& v, const Meta::TDontDeduce<T>& elt) {
    auto it = std::find(v.begin(), v.end(), elt);
    if (v.end() != it) {
        v.erase_DontPreserveOrder(it);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(TVector<T, _Allocator>& v, size_t index) {
    v.erase_DontPreserveOrder(v.begin() + index);
}
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(TVector<T, _Allocator>& v, const typename TVector<T, _Allocator>::const_iterator& it) {
    v.erase_DontPreserveOrder(it);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Clear(TVector<T, _Allocator>& v) {
    v.clear();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Clear_ReleaseMemory(TVector<T, _Allocator>& v) {
    v.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Reserve(TVector<T, _Allocator>& v, size_t capacity) {
    v.reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
hash_t hash_value(const TVector<T, _Allocator>& vector) {
    return hash_range(vector.begin(), vector.end());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename T, typename _Allocator>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TVector<T, _Allocator>& vector) {
    if (vector.empty()) {
        return oss << STRING_LITERAL(_Char, "[]");
    }
    else {
        auto it = vector.begin();
        oss << STRING_LITERAL(_Char, '[') << *it++;
        for (const auto end = vector.end(); it != end; ++it)
            oss << STRING_LITERAL(_Char, ", ") << *it;
        return oss << STRING_LITERAL(_Char, ']');
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
