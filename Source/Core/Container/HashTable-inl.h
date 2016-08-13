#pragma once

#include "Core/Container/HashTable.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
size_t HashTableBase<_Key, _Value>::GrowIFN_ReturnAllocationCount_(size_type atleast) {
    Assert(atleast <= details::HashTableCapacityForValue_[probe_type::MaxCapacityIndex]);
    if (atleast <= capacity()) {
        return 0;
    }
    else {
        Assert(0 < atleast);
        size_type capacityIndex = _size_capacityIndex&probe_type::MaskCapacityIndex+1;
        for (; details::HashTableCapacityForValue_[capacityIndex] < atleast; ++capacityIndex);
        _size_capacityIndex = (_size_capacityIndex&probe_type::MaskSize)|(capacityIndex);
        Assert(capacity() >= atleast);
        return AllocationCountWIndicesFor_(capacityIndex);
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
size_t HashTableBase<_Key, _Value>::ShrinkToFitIFN_ReturnAllocationCount_(size_type atleast) {
    Assert(atleast >= size());
    size_type capacityIndex = _size_capacityIndex&probe_type::MaskCapacityIndex;
    if (0 == capacityIndex || details::HashTableCapacityForValue_[capacityIndex-1] < atleast) {
        return 0;
    }
    else {
        --capacityIndex;
        for (; capacityIndex && details::HashTableCapacityForValue_[capacityIndex-1] > atleast; --capacityIndex);
        Assert(capacityIndex < (_size_capacityIndex&probe_type::MaskCapacityIndex));
        _size_capacityIndex = (_size_capacityIndex&probe_type::MaskSize)|(capacityIndex);
        Assert(capacity() >= atleast);
        return AllocationCountWIndicesFor_(capacityIndex);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
auto HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::operator=(const HashTable& other) -> HashTable& {
    if (this == &other)
        return *this;

    typedef typename allocator_traits::propagate_on_container_copy_assignment propagate_type;
    allocator_copy_(other, propagate_type());

    assign(other.begin(), other.end());
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
auto HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::operator=(HashTable&& rvalue) noexcept -> HashTable& {
    if (this == &rvalue)
        return *this;

    typedef typename allocator_traits::propagate_on_container_move_assignment propagate_type;
    allocator_move_(std::move(rvalue), propagate_type());

    assign(std::move(rvalue));
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::allocator_copy_(const allocator_type& other, std::true_type ) {
    if (allocator_() != other) {
        clear_ReleaseMemory();
        Assert(0 == _size_capacityIndex);
        Assert(nullptr == _values_hashIndices);
        allocator_type::operator=(other);
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::allocator_move_(allocator_type&& rvalue, std::true_type ) {
    if (allocator_() != rvalue) {
        clear_ReleaseMemory();
        Assert(0 == _size_capacityIndex);
        Assert(nullptr == _values_hashIndices);
        allocator_type::operator=(std::move(rvalue));
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::assign(HashTable&& rvalue) {
    typedef typename allocator_traits::propagate_on_container_move_assignment propagate_type;
    assign_rvalue_(std::move(rvalue), propagate_type());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::assign_rvalue_(HashTable&& rvalue, std::true_type ) {
    clear_ReleaseMemory();
    Assert(0 == _size_capacityIndex);
    Assert(nullptr == _values_hashIndices);

    std::swap(_size_capacityIndex, rvalue._size_capacityIndex);
    std::swap(_values_hashIndices, rvalue._values_hashIndices);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::assign_rvalue_(HashTable&& rvalue, std::false_type ) {
    if (allocator_() == rvalue.allocator_())
        assign_rvalue_(std::move(rvalue), std::true_type());
    else
        assign(std::make_move_iterator(rvalue._values_hashIndices), std::make_move_iterator(rvalue._values_hashIndices + rvalue.size()));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
bool HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::CheckInvariants() const {
#ifndef NDEBUG
    const details::HashTableProbe_ probe = MakeProbe_();
    if (0 != probe.ValuesCapacity && false == IS_POW2(probe.ValuesCapacity))
        return false;
    if (nullptr == _values_hashIndices && (probe.Size || probe.ValuesCapacity))
        return false;
    if (nullptr != _values_hashIndices && 0 == probe.ValuesCapacity)
        return false;
    if (probe.Size > probe.ValuesCapacity)
        return false;
    if (probe.HashCapacity < probe.ValuesCapacity)
        return false;
#endif
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
auto HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::insert(const value_type& value) -> Pair<iterator, bool> {
    size_type dataIndex;
    if (Insert_ReturnIfExists_(table_traits::Key(value), &dataIndex)) {
        return MakePair(begin() + dataIndex, false);
    }
    else {
        Assert(dataIndex < size());
        allocator_traits::construct(*this, _values_hashIndices+dataIndex, value);
        return MakePair(begin() + dataIndex, true);
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
auto HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::insert(value_type&& rvalue) -> Pair<iterator, bool> {
    size_type dataIndex;
    if (Insert_ReturnIfExists_(table_traits::Key(rvalue), &dataIndex)) {
        return MakePair(begin() + dataIndex, false);
    }
    else {
        Assert(dataIndex < size());
        allocator_traits::construct(*this, _values_hashIndices+dataIndex, std::move(rvalue));
        return MakePair(begin() + dataIndex, true);
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
auto HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::insert_or_assign(const value_type& value) -> Pair<iterator, bool> {
    size_type dataIndex;
    if (Insert_ReturnIfExists_(table_traits::Key(value), &dataIndex)) {
        _values_hashIndices[dataIndex] = value;
        return MakePair(begin() + dataIndex, false);
    }
    else {
        Assert(dataIndex < size());
        allocator_traits::construct(*this, _values_hashIndices+dataIndex, value);
        return MakePair(begin() + dataIndex, true);
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
auto HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::insert_or_assign(value_type&& rvalue) -> Pair<iterator, bool> {
    size_type dataIndex;
    if (Insert_ReturnIfExists_(table_traits::Key(rvalue), &dataIndex)) {
        _values_hashIndices[dataIndex] = std::move(rvalue);
        return MakePair(begin() + dataIndex, false);
    }
    else {
        Assert(dataIndex < size());
        allocator_traits::construct(*this, _values_hashIndices+dataIndex, std::move(rvalue));
        return MakePair(begin() + dataIndex, true);
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
template <typename... _Args>
auto HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::try_emplace(const key_type& key, _Args&&... args) -> Pair<iterator, bool> {
    size_type dataIndex;
    if (Insert_ReturnIfExists_(key, &dataIndex)) {
        return MakePair(begin() + dataIndex, false);
    }
    else {
        Assert(dataIndex < size());
        value_type tmp = table_traits::Make(key, std::forward<_Args>(args)...);
        allocator_traits::construct(*this, _values_hashIndices+dataIndex, std::move(tmp));
        return MakePair(begin() + dataIndex, true);
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
template <typename... _Args>
auto HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::try_emplace(key_type&& rkey, _Args&&... args) -> Pair<iterator, bool> {
    size_type dataIndex;
    if (Insert_ReturnIfExists_(rkey, &dataIndex)) {
        return MakePair(begin() + dataIndex, false);
    }
    else {
        Assert(dataIndex < size());
        value_type tmp = table_traits::Make(std::move(rkey), std::forward<_Args>(args)...);
        allocator_traits::construct(*this, _values_hashIndices+dataIndex, std::move(tmp));
        return MakePair(begin() + dataIndex, true);
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
bool HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::erase(const key_type& key, value_type* pValueIFP) {
    const details::HashTableProbe_ probe = MakeProbe_();
    Assert(0 < probe.Size);
    UNUSED(probe);

    UNUSED(key);
    UNUSED(pValueIFP);

    /*
    const size_type hashValue = KeyHash_(key);

    size_type dataIndex;
    Pair<size_type, bool> it(size_type(-1), false);

    do {
        it = FindUsingProbe_(probe, key, &dataIndex, it.first);

        if (it.second && KeyEqual_(key, _values_hashIndices[dataIndex])) {
            Assert(dataIndex < probe.Size);

            if (dataIndex + 1 != probe.Size) {
                const value_type& backValue = _values_hashIndices[probe.Size - 1];

                size_t backBucket, backDataIndex;
                if (not FindUsingProbe_(probe, table_traits::Key(backValue), &backBucket, &backDataIndex))
                    AssertNotReached();
                Assert(backDataIndex + 1 == probe.Size);

                probe.SwapDataIndex(it.first, backBucket);
                std::swap(_values_hashIndices[dataIndex], _values_hashIndices[backDataIndex]);

                dataIndex = backDataIndex;
            }

            probe.EraseBucket(it.first, hashValue);

            Assert(dataIndex + 1 == probe.Size);
            if (pValueIFP)
                *pValueIFP = std::move(_values_hashIndices[dataIndex]);

            allocator_traits::destroy(*this, _values_hashIndices+dataIndex);

            DecSize_();
            Assert(CheckInvariants());

            return true;
        }
    }
    while (it.second);

    */

    AssertNotImplemented(); // TODO: bubble down instead of tombstones
    return false;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::clear() {
    const details::HashTableProbe_ probe = MakeProbe_();
    if (probe.Size) {
        probe.ClearBuckets();

        forrange(i, 0, probe.Size)
            allocator_traits::destroy(*this, _values_hashIndices+i);

        SetSize_(0);
    }
    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::clear_ReleaseMemory() {
    if (_values_hashIndices) {
        Assert(size());
        Assert(capacity());

        forrange(i, 0, size())
            allocator_traits::destroy(*this, _values_hashIndices+i);

        allocator_traits::deallocate(*this, _values_hashIndices, AllocationCountWIndicesFor_(_size_capacityIndex&probe_type::MaskCapacityIndex));

        _size_capacityIndex = 0;
        _values_hashIndices = nullptr;
    }
    Assert(0 == _size_capacityIndex);
    Assert(nullptr == _values_hashIndices);
    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
bool HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::FindUsingProbe_(const details::HashTableProbe_& probe, const key_type& key, size_type* pSlotIndex, size_type* pDataIndex) const {
    Assert(AliasesToContainer_(probe));
    Assert(pDataIndex);

    return (probe.UseHashIndices64
        ? FindUsingProbe_(probe, probe.HashIndices64(), key, pSlotIndex, pDataIndex)
        : FindUsingProbe_(probe, probe.HashIndices32(), key, pSlotIndex, pDataIndex) );
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
template <typename _HashWIndices>
//NO_INLINE
bool HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::FindUsingProbe_(const details::HashTableProbe_& probe, const MemoryView<_HashWIndices>& hashWIndices, const key_type& key, size_type* pSlotIndex, size_type* pDataIndex) const {

    typedef typename _HashWIndices::size_type hash_type;
    const hash_type hashValue = hash_type(KeyHash_(key));

    size_type bucket = probe.DesiredPos(hashValue);
    size_type inc = probe.DesiredInc(hashValue);
    size_type distance = 0;

    while(true) {
        Assert(distance < probe.HashCapacity);
        const _HashWIndices& it = hashWIndices[bucket];

        if (it.empty() || not probe.ProbeAvailable(it.hash_value, bucket, distance)) {
            Assert(it.hash_value != hashValue);
            break;
        }
        else if (it.hash_value == hashValue && KeyEqual_(key, _values_hashIndices[it.data_index])) {
            Assert(it.data_index < probe.Size);
            Assert(probe.ProbeDistance(it.hash_value, bucket) == distance);
            *pSlotIndex = bucket;
            *pDataIndex = it.data_index;
            return true;
        }

        bucket = probe.NextBucket(bucket, inc);
        distance++;
    }

    Assert(CheckInvariants());
    return false;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
bool HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::InsertUsingProbe_AssumeEnoughCapacity_(details::HashTableProbe_& probe, const key_type& key, size_type* pDataIndex) {
    Assert(AliasesToContainer_(probe));
    Assert(pDataIndex);
    Assert(probe.Size < probe.ValuesCapacity);

    return (probe.UseHashIndices64
        ? InsertUsingProbe_AssumeEnoughCapacity_(probe, probe.HashIndices64(), key, pDataIndex)
        : InsertUsingProbe_AssumeEnoughCapacity_(probe, probe.HashIndices32(), key, pDataIndex) );
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
template <typename _HashWIndices>
bool HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::InsertUsingProbe_AssumeEnoughCapacity_(details::HashTableProbe_& probe, const MemoryView<_HashWIndices>& hashWIndices, const key_type& key, size_type* pDataIndex) {
    Assert(probe.Size < probe.ValuesCapacity);

    _HashWIndices idx = _HashWIndices::Make(KeyHash_(key), probe.Size);
    *pDataIndex = idx.data_index;

    size_type bucket = probe.DesiredPos(idx.hash_value);
    size_type inc = probe.DesiredInc(idx.hash_value);
    size_type distance = 0;

    for (;;) {
        Assert(distance < probe.HashCapacity);
        _HashWIndices& it = hashWIndices[bucket];

        if (it.empty()) {
            it = idx;
            break;
        }

        Assert(it.data_index < probe.Size);

        const size_type it_distance = probe.ProbeDistance(it.hash_value, bucket);
        if (it_distance < distance) {
            Assert(it.hash_value != idx.hash_value);
            std::swap(it, idx);
            distance = it_distance;
            inc = probe.DesiredInc(idx.hash_value);
            Assert(distance == probe.ProbeDistance(idx.hash_value, bucket));
        }
        else if (it.hash_value == idx.hash_value && KeyEqual_(key, _values_hashIndices[it.data_index])) {
            *pDataIndex = it.data_index;
            return true;
        }

        bucket = probe.NextBucket(bucket, inc);
        distance++;
    }

    IncSize_();
    probe.Size++;

    Assert(size() == probe.Size);
    Assert(*pDataIndex + 1 == probe.Size);
    Assert(CheckInvariants());
    return false;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
bool HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::Insert_ReturnIfExists_(const key_type& key, size_type* pDataIndex) {
    reserve_Additional(1);
    details::HashTableProbe_ probe = MakeProbe_();
    const bool exists = InsertUsingProbe_AssumeEnoughCapacity_(probe, key, pDataIndex);
    Assert(exists || size() == *pDataIndex + 1);
    return exists;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
template <typename _It>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::InsertRange_(_It first, _It last, std::input_iterator_tag ) {
    for (; first != last; ++first) {
        value_type tmp(*first);
        insert(std::move(tmp));
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
template <typename _It, typename _ItCat>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::InsertRange_(_It first, _It last, _ItCat ) {
    const size_type count = checked_cast<size_type>(std::distance(first, first));
    reserve(size() + count);

    details::HashTableProbe_ probe = MakeProbe_();
    for (; first != last; ++first) {
        value_type tmp(*first);
        size_type dataIndex;
        if (not InsertUsingProbe_AssumeEnoughCapacity_(probe, table_traits::Key(tmp), &dataIndex)) {
            Assert(probe.Size == dataIndex + 1);
            allocator_traits::construct(*this, _values_hashIndices+dataIndex, std::move(tmp));
        }
    }
    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::shrink_to_fit() {
    if (empty()) {
        clear_ReleaseMemory();
    }
    else {
        const size_type oldCapacityIndex = _size_capacityIndex&probe_type::MaskCapacityIndex;
        if (size_type allocationCount = ShrinkToFitIFN_ReturnAllocationCount_(size()))
            RelocateAndRehash_(oldCapacityIndex, allocationCount);
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::reserve(size_type count) {
    const size_type oldCapacityIndex = _size_capacityIndex&probe_type::MaskCapacityIndex;
    if (size_type allocationCount = GrowIFN_ReturnAllocationCount_(count))
        RelocateAndRehash_(oldCapacityIndex, allocationCount);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::Reserve_AssumeEmpty_(size_type count) {
    Assert(empty());
    Assert(nullptr == _values_hashIndices);
    if (size_type allocationCount = GrowIFN_ReturnAllocationCount_(count)) {
        _values_hashIndices = allocator_traits::allocate(*this, allocationCount);
        Assert(_values_hashIndices);
    }
    Assert(empty());
    Assert(capacity() >= count);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::RelocateAndRehash_(size_type oldCapacityIndex, size_type allocationCount) {
    // HashTable<> doesn't use Relocate() because of hashWIndices packing
    pointer const oldData = _values_hashIndices;

    _values_hashIndices = (allocationCount
        ? allocator_traits::allocate(*this, allocationCount)
        : nullptr );

    details::HashTableProbe_ probe = MakeProbe_();
    Assert(0 == probe.Size || nullptr != _values_hashIndices);
    Assert(0 == probe.Size || nullptr != oldData);
    Assert(0 == probe.Size || 0 != allocationCount);

    forrange(i, 0, probe.Size) {
        allocator_traits::construct(*this, _values_hashIndices+i, std::move(oldData[i]));
        allocator_traits::destroy(*this, oldData+i);
    }

    if (oldData) {
        AssertNotImplemented(); // TODO: reuse old hash values if the hwindices type didn't change
        allocator_traits::deallocate(*this, oldData, AllocationCountWIndicesFor_(oldCapacityIndex));
    }

    RehashUsingProbe_(probe);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::RehashUsingProbe_(details::HashTableProbe_& probe) {
    Assert(AliasesToContainer_(probe));

    const size_type n = probe.Size;
    UNUSED(n);

    SetSize_(0);
    probe.Size = 0;
    probe.ClearBuckets();

    forrange(i, 0, probe.Size) {
        size_t dataIndex = i;
        if (InsertUsingProbe_AssumeEnoughCapacity_(probe, table_traits::Key(_values_hashIndices[i]), &dataIndex))
            AssertNotReached();
        Assert(dataIndex == i);
    }

    Assert(n == size());
    Assert(n == probe.Size);
    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::Rehash_() {
    details::HashTableProbe_ probe = MakeProbe_();
    RehashUsingProbe_(probe);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::swap(HashTable& other) {
    typedef typename allocator_traits::propagate_on_container_swap propagate_type;
    if (this != &other)
        swap_(other, propagate_type());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::swap_(HashTable& other, std::true_type ) {
    std::swap(_size_capacityIndex, other._size_capacityIndex);
    std::swap(_values_hashIndices, other._values_hashIndices);
    std::swap(static_cast<allocator_type&>(*this), static_cast<allocator_type&>(other));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hash, typename _Equal, typename _Allocator>
void HashTable<_Key, _Value, _Hash, _Equal, _Allocator>::swap_(HashTable& other, std::false_type ) {
    HashTable* plhs = nullptr;
    HashTable* prhs = nullptr;
    if (size() < other.size()) {
        plhs = this;
        prhs = &other;
    }
    else {
        plhs = &other;
        prhs = this;
    }

    Assert(plhs->size() <= prhs->size());
    Assert(plhs->capacity() <= prhs->capacity());
    std::swap_ranges(
        plhs->_values_hashIndices,
        plhs->_values_hashIndices + plhs->size(),
        MakeCheckedIterator(prhs->_values_hashIndices, prhs->size(), 0) );

    if (plhs->capacity() != prhs->capacity()) {
        Assert(plhs->size() < prhs->size());
        Assert(plhs->capacity() < prhs->capacity());

        pointer const oldData = plhs->_values_hashIndices;
        plhs->_values_hashIndices = allocator_traits::allocate(*this, AllocationCountWIndicesFor_(prhs->_size_capacityIndex&probe_type::MaskCapacityIndex));
        Assert(plhs->_values_hashIndices);

        forrange(i, 0, plhs->size()) {
            allocator_traits::construct(*this, plhs->_values_hashIndices+i, std::move(oldData[i]));
            allocator_traits::destroy(*this, oldData+i);
        }

        if (oldData)
            allocator_traits::deallocate(*this, oldData, AllocationCountWIndicesFor_(plhs->_size_capacityIndex&probe_type::MaskCapacityIndex));

        forrange(i, plhs->size(), prhs->size()) {
            allocator_traits::construct(*this, plhs->_values_hashIndices+i, std::move(prhs->_values_hashIndices[i]));
            allocator_traits::destroy(*this, prhs->_values_hashIndices+i);
        }

        std::swap(plhs->_size_capacityIndex, prhs->_size_capacityIndex);

        plhs->Rehash_();
        prhs->Rehash_();

        AssertNotImplemented(); // TODO: rehash using old hash values
    }
    else {
        std::swap(plhs->_size_capacityIndex, prhs->_size_capacityIndex);

        const details::HashTableProbe_& lhs_probe = plhs->MakeProbe_();
        const details::HashTableProbe_& rhs_probe = prhs->MakeProbe_();
        Assert(lhs_probe.UseHashIndices64 == rhs_probe.UseHashIndices64);

        if (lhs_probe.UseHashIndices64)
            std::swap_ranges(lhs_probe.HashIndices64().begin(), lhs_probe.HashIndices64().end(), rhs_probe.HashIndices64().begin());
        else
            std::swap_ranges(lhs_probe.HashIndices32().begin(), lhs_probe.HashIndices32().end(), rhs_probe.HashIndices32().begin());
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
