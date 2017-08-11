#pragma once

#include "Core/Container/HashTable.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::operator=(const TBasicHashTable& other) -> TBasicHashTable& {
    if (this == &other)
        return *this;

    typedef typename allocator_traits::propagate_on_container_copy_assignment propagate_type;
    allocator_copy_(other, propagate_type());

    assign(other.begin(), other.end());
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::operator=(TBasicHashTable&& rvalue) noexcept -> TBasicHashTable& {
    if (this == &rvalue)
        return *this;

    typedef typename allocator_traits::propagate_on_container_move_assignment propagate_type;
    allocator_move_(std::move(rvalue), propagate_type());

    assign(std::move(rvalue));
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
float TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::load_factor() const {
    // truely computes load factor by taking tombstones in check
    const_pointer buckets = _data.GetBuckets();
    size_type occupieds = 0;
    forrange(index, 0, capacity()) {
        if (_data.GetState(index) != EBucketState::Inactive)
            occupieds++;
    }
    return (occupieds * 1.0f / capacity());
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::assign(TBasicHashTable&& rvalue) {
    typedef typename allocator_traits::propagate_on_container_move_assignment propagate_type;
    assign_rvalue_(std::move(rvalue), propagate_type());
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::find(const key_type& key) -> iterator {
    /*
    const size_type bucket = FindFilledBucket_(key);
    return (NoIndex != bucket ? MakeIterator_(bucket) : end());
    */
    if (_data.Size) {
        Assert(_data.MaxProbeDist);

        const_pointer buckets = _data.GetBuckets();
        const size_type mask = size_type(_data.CapacityM1);
        const size_type h = _data.HashKey(key);

        forrange(o, 0, size_type(_data.MaxProbeDist)) {
            const size_type index = (h + o) & mask;
            const EBucketState state = _data.GetState(index);

            if (state == EBucketState::Filled &&
                _data.KeyEqual(table_traits::Key(buckets[index]), key)) {
                return iterator(*this, (public_type*)(buckets + index));
            }
            else if (state == EBucketState::Inactive) {
                break; // End of the chain!
            }
        }
    }
    return end();
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
FORCE_INLINE auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::find(const key_type& key) const -> const_iterator {
    /*
    const size_type bucket = FindFilledBucket_(key);
    return (NoIndex != bucket ? MakeIterator_(bucket) : end());
    */
    return const_cast<TBasicHashTable*>(this)->find(key);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::insert(const value_type& value) -> TPair<iterator, bool> {
    reserve_Additional(1);
    const size_type bucket = FindOrAllocateBucket_(table_traits::Key(value));
    Assert(NoIndex != bucket);
    Assert(bucket < capacity());
    const iterator it = MakeIterator_(bucket);
    if (EBucketState::Filled == _data.GetState(bucket)) {
        return MakePair(it, false);
    }
    else {
        _data.Size++;
        _data.SetState(bucket, EBucketState::Filled);
        allocator_traits::construct(*this, (pointer)it.data(), value);
        return MakePair(it, true);
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::insert(value_type&& rvalue) -> TPair<iterator, bool> {
    reserve_Additional(1);
    const size_type bucket = FindOrAllocateBucket_(table_traits::Key(rvalue));
    Assert(NoIndex != bucket);
    Assert(bucket < capacity());
    const iterator it = MakeIterator_(bucket);
    if (EBucketState::Filled == _data.GetState(bucket)) {
        return MakePair(it, false);
    }
    else {
        _data.Size++;
        _data.SetState(bucket, EBucketState::Filled);
        allocator_traits::construct(*this, (pointer)it.data(), std::move(rvalue));
        return MakePair(it, true);
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::insert_or_assign(const value_type& value) -> TPair<iterator, bool> {
    reserve_Additional(1);
    const size_type bucket = FindOrAllocateBucket_(table_traits::Key(value));
    Assert(NoIndex != bucket);
    Assert(bucket < capacity());
    const iterator it = MakeIterator_(bucket);
    if (EBucketState::Filled == _data.GetState(bucket)) {
        *(pointer)it.data() = value;
        return MakePair(it, false);
    }
    else {
        _data.Size++;
        _data.SetState(bucket, EBucketState::Filled);
        allocator_traits::construct(*this, (pointer)it.data(), value);
        return MakePair(it, true);
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::insert_or_assign(value_type&& rvalue) -> TPair<iterator, bool> {
    reserve_Additional(1);
    const size_type bucket = FindOrAllocateBucket_(table_traits::Key(rvalue));
    Assert(NoIndex != bucket);
    Assert(bucket < capacity());
    const iterator it = MakeIterator_(bucket);
    if (EBucketState::Filled == _data.GetState(bucket)) {
        *(pointer)it.data() = std::move(rvalue);
        return MakePair(it, false);
    }
    else {
        _data.Size++;
        _data.SetState(bucket, EBucketState::Filled);
        allocator_traits::construct(*this, (pointer)it.data(), std::move(rvalue));
        return MakePair(it, true);
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename... _Args>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::try_emplace(const key_type& key, _Args&&... args) -> TPair<iterator, bool> {
    reserve_Additional(1);
    const size_type bucket = FindOrAllocateBucket_(key);
    Assert(NoIndex != bucket);
    Assert(bucket < capacity());
    const iterator it = MakeIterator_(bucket);
    if (EBucketState::Filled == _data.GetState(bucket)) {
        return MakePair(it, false);
    }
    else {
        _data.Size++;
        _data.SetState(bucket, EBucketState::Filled);
        allocator_traits::construct(*this, (pointer)it.data(),
            table_traits::Make(key, std::forward<_Args>(args)...) );
        return MakePair(it, true);
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename... _Args>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::try_emplace(key_type&& rkey, _Args&&... args) -> TPair<iterator, bool> {
    reserve_Additional(1);
    const size_type bucket = FindOrAllocateBucket_(rkey);
    Assert(NoIndex != bucket);
    Assert(bucket < capacity());
    const iterator it = MakeIterator_(bucket);
    if (EBucketState::Filled == _data.GetState(bucket)) {
        return MakePair(it, false);
    }
    else {
        _data.Size++;
        _data.SetState(bucket, EBucketState::Filled);
        allocator_traits::construct(*this, (pointer)it.data(),
            table_traits::Make(std::move(rkey), std::forward<_Args>(args)...) );
        return MakePair(it, true);
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::erase(const const_iterator& it) {
    Assert(it.AliasesToContainer(*this));
    Assert(_data.Size > 0);

    const pointer buckets = _data.GetBuckets();
    const size_type bucket = std::distance(reinterpret_cast<const public_type*>(buckets), it.data());
    Assert(_data.GetState(bucket) == EBucketState::Filled);

    _data.Size--;
    _data.SetState(bucket, EBucketState::Tomb);
    allocator_traits::destroy(*this, &buckets[bucket]);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::erase(const key_type& key, value_type* pValueIFP) {
    const size_type bucket = FindFilledBucket_(key);

    if (NoIndex != bucket) {
        Assert(_data.Size > 0);
        Assert(_data.GetState(bucket) == EBucketState::Filled);

        _data.Size--;
        _data.SetState(bucket, EBucketState::Tomb);

        value_type& item = _data.GetBuckets()[bucket];
        if (pValueIFP)
            *pValueIFP = std::move(item);

        allocator_traits::destroy(*this, &item);

        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::clear() {
    if (_data.Size) {
        const size_type n = capacity();
        const_pointer buckets = _data.GetBuckets();
        ONLY_IF_ASSERT(size_type deleteds = 0);
        for (size_type i = 0; i < n; ++i) {
            if (_data.GetState(i) == EBucketState::Filled) {
                allocator_traits::destroy(*this, &buckets[i]);
                ONLY_IF_ASSERT(++deleteds);
            }
        }
        ONLY_IF_ASSERT(Assert(deleteds == _data.Size));
        _data.Size = 0;
        _data.MaxProbeDist = 0;
        memset(_data.StatesAndBuckets, 0x00, FData_::StatesSizeInT(n) * sizeof(value_type)); // Reset all states to Inactive
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::clear_ReleaseMemory() {
    const size_type n = capacity();
    if (n) {
        Assert(_data.StatesAndBuckets);

        clear();

        allocator_traits::deallocate(*this, _data.StatesAndBuckets, FData_::StatesSizeInT(n) + n);
        _data.StatesAndBuckets = nullptr;
        _data.CapacityM1 = u32(-1);
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::rehash(size_type count) {
    const size_type newCapacity = Meta::NextPow2(count);
    if (newCapacity > capacity()) {
        RelocateRehash_(newCapacity);
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::reserve(size_type count) {
    if (size_type newCapacity = GrowIFN_ReturnNewCapacity_(count)) {
        RelocateRehash_(newCapacity);
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::shrink_to_fit() {
    if (size_type newCapacity = ShrinkIFN_ReturnNewCapacity_(_data.Size)) {
        RelocateRehash_(newCapacity);
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::swap(TBasicHashTable& other) {
    typedef typename allocator_traits::propagate_on_container_swap propagate_type;
    if (this != &other)
        swap_(other, propagate_type());
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::CheckInvariants() const {
    STATIC_ASSERT(sizeof(value_type) == sizeof(public_type));
    STATIC_ASSERT(MaxLoadFactor < 100);
    STATIC_ASSERT(sizeof(value_type) >= sizeof(EBucketState));
#ifndef NDEBUG
    if (0 != capacity() && false == Meta::IsPow2(capacity()))
        return false;
    if (nullptr == _data.StatesAndBuckets && (size() || capacity()))
        return false;
    if (size() > capacity())
        return false;
#endif
    return true;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::AliasesToContainer(const_pointer p) const {
    const_pointer buckets = _data.GetBuckets();
    return (buckets <= p && buckets + capacity() > p);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::operator ==(const TBasicHashTable& other) const {
    if (size() != other.size())
        return false;

    const_pointer buckets = _data.GetBuckets();
    forrange(i, 0, capacity()) {
        if (_data.GetState(i) == EBucketState::Filled) {
            const value_type& elt = buckets[i];
            const auto it = other.find(table_traits::Key(elt));
            if (other.end() == it || table_traits::Value(*it) != table_traits::Value(elt))
                return false;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::allocator_copy_(const allocator_type& other, std::true_type ) {
    if (allocator_() != other) {
        clear_ReleaseMemory();
        Assert(0 == _data.Size);
        Assert(nullptr == _data.StatesAndBuckets);
        allocator_type::operator=(other);
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::allocator_move_(allocator_type&& rvalue, std::true_type ) {
    if (allocator_() != rvalue) {
        clear_ReleaseMemory();
        Assert(0 == _data.Size);
        Assert(nullptr == _data.StatesAndBuckets);
        allocator_type::operator=(std::move(rvalue));
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::assign_rvalue_(TBasicHashTable&& rvalue, std::true_type) {
    clear_ReleaseMemory();
    Assert(0 == _data.Size);
    Assert(nullptr == _data.StatesAndBuckets);
    _data.Swap(rvalue._data);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::assign_rvalue_(TBasicHashTable&& rvalue, std::false_type) {
    if (allocator_() == rvalue.allocator_())
        assign_rvalue_(std::move(rvalue), std::true_type());
    else
        assign(std::make_move_iterator(rvalue.begin()), std::make_move_iterator(rvalue.end()));
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename _It>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::insert_(_It first, _It last, std::forward_iterator_tag) {
    forrange(it, first, last)
        insert(*it);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename _It>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::insert_(_It first, _It last, std::bidirectional_iterator_tag) {
    insert_(first, last, std::forward_iterator_tag());
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename _It>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::insert_(_It first, _It last, std::random_access_iterator_tag) {
    reserve_Additional(std::distance(first, last));
    insert_(first, last, std::forward_iterator_tag());
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::swap_(TBasicHashTable& other, std::true_type) {
    _data.Swap(other._data);
    std::swap(static_cast<allocator_type&>(*this), static_cast<allocator_type&>(other));
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::swap_(TBasicHashTable& other, std::false_type) {
    TBasicHashTable cpy(std::move(*this));
    assign(std::move(other));
    other.assign(std::move(cpy));
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::GrowIFN_ReturnNewCapacity_(size_type atleast) const -> size_type {
    atleast += (atleast * SlackFactor) >> 7;// (atleast * (100 - MaxLoadFactor))/100;
    return ((atleast >= (_data.CapacityM1 + 1)) ? Meta::NextPow2(atleast) : 0);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::ShrinkIFN_ReturnNewCapacity_(size_type atleast) const -> size_type {
    atleast += (atleast * SlackFactor) >> 7;// (atleast * (100 - MaxLoadFactor))/100;
    atleast = Meta::NextPow2(atleast);
    return ((atleast < (_data.CapacityM1 + 1)) ? atleast : 0);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::FindFilledBucket_(const key_type& key) const -> size_type {
    if (0 == _data.Size)
        return NoIndex;

    Assert(_data.MaxProbeDist);
    const size_type h = _data.HashKey(key);
    const size_type mask = size_type(_data.CapacityM1);
    const_pointer buckets = _data.GetBuckets();

    forrange(o, 0, size_type(_data.MaxProbeDist)) {
        const size_type index = (h + o) & mask;
        const EBucketState state = _data.GetState(index);

        if (state == EBucketState::Filled &&
            _data.KeyEqual(table_traits::Key(buckets[index]), key) ) {
            return index;
        }
        else if (state == EBucketState::Inactive) {
            break; // End of the chain!
        }
    }

    return NoIndex;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::FindEmptyBucket_(const key_type& key) const -> size_type {
    Assert(_data.Size < (_data.CapacityM1 + 1));
    const size_type h = _data.HashKey(key);
    const size_type mask = size_type(_data.CapacityM1);

    for (size_type o = 0; ; ++o) {
        const size_type index = (h + o) & mask;
        if (_data.GetState(index) != EBucketState::Filled) {
            if (o >= _data.MaxProbeDist) {
                _data.MaxProbeDist = (o + 1);
            }
            return index;
        }
    }

    AssertNotReached();
    return NoIndex;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::FindOrAllocateBucket_(const key_type& key) const -> size_type {
    const size_type h = _data.HashKey(key);
    const size_type mask = size_type(_data.CapacityM1);
    const const_pointer buckets = _data.GetBuckets();

    size_type hole = NoIndex;
    size_type offset = 0;
    for (; offset < _data.MaxProbeDist; ++offset) {
        const size_type index = (h + offset) & mask;
        const EBucketState state = _data.GetState(index);

        if (state == EBucketState::Filled) {
            if (_data.KeyEqual(table_traits::Key(buckets[index]), key))
                return index;
        }
        else if (state == EBucketState::Inactive) {
            return index;
        }
        else { // ACTIVE: keep searching
            if (hole == NoIndex)
                hole = index;
        }
    }

    // No key found - but maybe a hole for it
    Assert(_data.MaxProbeDist == offset);
    if (hole != NoIndex)
        return hole;

    // No hole found within _data.MaxProbeDist
    for (;; ++offset) {
        const size_type index = (h + offset) & mask;
        const EBucketState state = _data.GetState(index);

        if (state != EBucketState::Filled) {
            _data.MaxProbeDist = checked_cast<u8>(offset + 1);
            return index;
        }
    }

    AssertNotReached();
    return NoIndex;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::RelocateRehash_(size_type newCapacity) {
    Assert(newCapacity > 0 && Meta::IsPow2(newCapacity));
    Assert(_data.Size <= newCapacity);

    const FData_ oldData = _data;
    const size_type oldCapacity = capacity();
    const size_type newStatesSizeInT = FData_::StatesSizeInT(newCapacity);

    _data.StatesAndBuckets = allocator_traits::allocate(*this, newStatesSizeInT + newCapacity);
    _data.CapacityM1 = u32(newCapacity) - 1;
    _data.Size = 0;
    _data.MaxProbeDist = 0;

    AssertRelease(_data.StatesAndBuckets);

    memset(_data.StatesAndBuckets, 0x00, newStatesSizeInT * sizeof(value_type)); // Initialize all states to Inactive

    if (oldData.Size) {
        Assert(oldData.StatesAndBuckets);
        Assert(oldCapacity > 0 && Meta::IsPow2(oldCapacity));
        const pointer newBuckets = _data.GetBuckets();
        const_pointer oldBuckets = oldData.GetBuckets();

        for (size_type i = 0; i < oldCapacity; ++i) {
            if (oldData.GetState(i) == EBucketState::Filled) {
                const size_type j = FindEmptyBucket_(table_traits::Key(oldBuckets[i]));
                Assert(NoIndex != j);

                ONLY_IF_ASSERT(_data.Size++);
                _data.SetState(j, EBucketState::Filled);

                allocator_traits::construct(*this, &newBuckets[j], std::move(oldBuckets[i]));
                allocator_traits::destroy(*this, &oldBuckets[i]);
            }
        }

#ifdef WITH_CORE_ASSERT
        Assert(_data.Size == oldData.Size);
#else
        _data.Size = oldData.Size;
#endif
    }

    if (oldData.CapacityM1 != u32(-1)) {
        Assert(oldData.StatesAndBuckets);
        allocator_traits::deallocate(*this, oldData.StatesAndBuckets, FData_::StatesSizeInT(oldCapacity) + oldCapacity);
    }
    else {
        Assert(nullptr == oldData.StatesAndBuckets);
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::FData_::GetState(size_type index) const -> EBucketState {
    Assert(index < (CapacityM1 + 1));
    //index *= BitsPerState_;
    index <<= BitsPerStateLog2_;
    return EBucketState(
        (((const size_type*)StatesAndBuckets)[index >> WordBitShift] >>
        (index & WordBitMask)) & BitsStateMask_
    );
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::FData_::SetState(size_type index, EBucketState state) {
    Assert(index < (CapacityM1 + 1));
    //index *= BitsPerState_;
    index <<= BitsPerStateLog2_;
    size_type& w = ((size_type*)StatesAndBuckets)[index >> WordBitShift];
    w &= ~(BitsStateMask_ << index);
    w |= size_type(state) << index;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::FData_::NextBucket(size_type bucket) const -> size_type {
    Assert(bucket <= (CapacityM1 + 1));
    for (   const size_type n = (CapacityM1 + 1);
            bucket < n && GetState(bucket) != EBucketState::Filled;
            bucket++ );
    return bucket;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::FData_::Swap(FData_& other) {
    std::swap(StatesAndBuckets, other.StatesAndBuckets);
    std::swap(CapacityM1, other.CapacityM1);
    u32 tmp;
    {
        tmp = Size;
        Size = other.Size;
        other.Size = tmp;
    }
    {
        tmp = MaxProbeDist;
        MaxProbeDist = other.MaxProbeDist;
        other.MaxProbeDist = tmp;
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename T>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::TIterator_<T>::GotoNextBucket_() -> TIterator_& {
    Assert(_p);
    Assert(_m);
    Assert(_m->AliasesToContainer(reinterpret_cast<const_pointer>(_p)));

    STATIC_ASSERT(std::is_same<
        typename parent_type::difference_type,
        typename TBasicHashTable::difference_type
    >::value);

    const pointer buckets = (pointer)_m->_data.GetBuckets();
    _p = buckets + _m->_data.NextBucket(size_type(std::distance(buckets, _p) + 1));
    return *this;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
