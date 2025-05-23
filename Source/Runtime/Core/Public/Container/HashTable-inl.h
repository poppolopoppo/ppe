#pragma once

#include "Container/HashTable.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
inline bool FHashTableData_::SetElement(size_t index, size_t hash) {
    return (H2(hash) != SetState(index, H2(hash)));
}
//----------------------------------------------------------------------------
inline void FHashTableData_::Swap(FHashTableData_& other) {
    std::swap(Size, other.Size);
    std::swap(CapacityM1, other.CapacityM1);
    std::swap(StatesAndBuckets, other.StatesAndBuckets);
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::operator=(const TBasicHashTable& other) -> TBasicHashTable& {
    if (this == &other)
        return *this;

    typedef typename allocator_traits::propagate_on_container_copy_assignment propagate_type;
    allocator_copy_(other, propagate_type());

    assign(other);
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
    const size_type n = capacity();
    Assert(Meta::IsAlignedPow2(FHTD::GGroupSize, n));

    size_type emptyBlocks = 0;
    for (size_type i = 0; i < n; i += FHTD::GGroupSize) {
        // include kDeleted for load_factor() since it's directly affecting probing performance
        emptyBlocks += FHTD::MatchEmpty(_data.GroupAt(i)).Count();
    }

    return (1.0f - static_cast<float>(emptyBlocks) / n);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::max_probe_dist() const -> size_type {
    if (0 == _data.Size)
        return 0;

    const const_pointer buckets = BucketAt_(0);

    size_type max_dist = 0;
    ONLY_IF_ASSERT(size_type sizeCheck = 0);

    forrange(it, begin(), end()) {
        const const_pointer p = it.data();
        const size_type hash = HashValue_(*p);
        Assert_NoAssume(FHTD::H2(hash) == *it.state());

        const size_type wanted = (_data.H1(hash, Meta::has_trivial_copy<value_type>{}) & _data.CapacityM1);
        const size_type inserted = (p - buckets);

        size_type probe_dist = 0;
        for (size_type offset = wanted, index = 0; offset != inserted; ++probe_dist) {
            index += FHTD::GGroupSize;
            offset = ((offset + index) & _data.CapacityM1);
        }

        max_dist = Max(max_dist, probe_dist);

        ONLY_IF_ASSERT(++sizeCheck);
    }

    Assert_NoAssume(sizeCheck == _data.Size);
    return max_dist;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::assign(const TBasicHashTable& other) {
    Assert(&other != this);

    IF_CONSTEXPR(Meta::has_trivial_copy<value_type>::value) {
        if (_data.StatesAndBuckets)
            clear_ReleaseMemory();

        const size_t blockSizeInBytes = other.allocator_block_().SizeInBytes;

        if (other._data.Size) {
            Assert_NoAssume(nullptr == _data.StatesAndBuckets);
            Assert_NoAssume(blockSizeInBytes == (other.OffsetOfBuckets_() + other._data.NumBuckets()) * sizeof(value_type));

            const FAllocatorBlock alloc = allocator_traits::Allocate(*this, blockSizeInBytes);
            Assert_NoAssume(alloc.SizeInBytes >= blockSizeInBytes);

            _data.Size = other._data.Size;
            _data.CapacityM1 = other._data.CapacityM1;
            _data.StatesAndBuckets = alloc.Data;

            Assert_NoAssume(blockSizeInBytes == (OffsetOfBuckets_() + _data.NumBuckets()) * sizeof(value_type));

            FPlatformMemory::Memcpy(_data.StatesAndBuckets, other._data.StatesAndBuckets, blockSizeInBytes);
        }
        else {
            Assert_NoAssume(0 == _data.Size);
            Assert_NoAssume(0 == _data.NumBuckets());
            Assert_NoAssume(nullptr == _data.StatesAndBuckets);
        }
    }
    else {
        assign(other.begin(), other.end());
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::assign(TBasicHashTable&& rvalue) {
    Assert(&rvalue != this);

    const FAllocatorBlock alloc = rvalue.allocator_block_();

    if (MoveAllocatorBlock(&allocator_traits::Get(*this), allocator_traits::Get(rvalue), alloc)) {
        clear_ReleaseMemory();

        std::swap(_data, rvalue._data);
    }
    else {
        assign(
            MakeMoveIterator(rvalue.begin()),
            MakeMoveIterator(rvalue.end()));

        rvalue.clear_ReleaseMemory();
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::find(const key_type& key) NOEXCEPT -> iterator {
    return find_like(key, HashKey_(key));
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
FORCE_INLINE auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::find(const key_type& key) const NOEXCEPT -> const_iterator {
    return find_like(key, HashKey_(key));
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename _KeyLike>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::find_like(const _KeyLike& keyLike, hash_t hash) NOEXCEPT -> iterator {
    using public_pointer = typename iterator::pointer;

    pointer const buckets = (reinterpret_cast<pointer>(_data.StatesAndBuckets) + OffsetOfBuckets_());

    if (Likely(_data.Size)) {
        const ::__m128i h2_16 = ::_mm_set1_epi8(FHTD::H2(hash));

        for (size_type offset = (_data.H1(hash, Meta::has_trivial_copy<value_type>{}) & _data.CapacityM1), index = 0;;) {
            const auto group = _data.GroupAt(offset);

            for (bitmask_t match_16 = FHTD::Match(group, h2_16); match_16; ) {
                const size_type bucket = ((offset + match_16.PopFront_AssumeNotEmpty()) & _data.CapacityM1/* can overflow due to wrapping */);
                if (Likely(static_cast<const _EqualTo&>(*this)(table_traits::Key(buckets[bucket]), keyLike)))
                    return iterator(
                        static_cast<const state_t*>(_data.StatesAndBuckets),
                        reinterpret_cast<public_pointer>(buckets),
                        _data.NumBuckets(), bucket);
            }

            if (FHTD::MatchEmpty(group))
                break;

            index += FHTD::GGroupSize;
            offset = ((offset + index) & _data.CapacityM1);
        }
    }

    return iterator(
        static_cast<const state_t*>(_data.StatesAndBuckets),
        reinterpret_cast<public_pointer>(buckets),
        _data.NumBuckets(), _data.NumBuckets());
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename _KeyLike>
FORCE_INLINE auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::find_like(const _KeyLike& keyLike, hash_t hash) const NOEXCEPT -> const_iterator {
    return const_cast<TBasicHashTable*>(this)->find_like(keyLike, hash);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::Add(const key_type& key) -> mapped_reference {
    auto r = try_emplace(key);
    Assert(r.second);
    return table_traits::Value(*r.first);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::Add(key_type&& rkey) -> mapped_reference {
    auto r = try_emplace(std::move(rkey));
    Assert(r.second);
    return table_traits::Value(*r.first);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::FindOrAdd(const key_type& key) -> mapped_reference {
    auto r = try_emplace(key);
    return table_traits::Value(*r.first);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::FindOrAdd(key_type&& rkey) -> mapped_reference {
    auto r = try_emplace(std::move(rkey));
    return table_traits::Value(*r.first);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::insert(const value_type& value) -> TPair<iterator, bool> {
    return InsertIFN_(table_traits::Key(value), HashKey_(table_traits::Key(value)), value);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::insert(value_type&& rvalue) -> TPair<iterator, bool> {
    return InsertIFN_(table_traits::Key(rvalue), HashKey_(table_traits::Key(rvalue)), std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::insert_AssertUnique(const value_type& value) {
    value_type cpy(value);
    insert_AssertUnique(std::move(cpy));
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::insert_AssertUnique(value_type&& rvalue) {
    Assert_NoAssume(not AliasesToContainer(&rvalue));

    reserve_Additional(1); // TODO: problem here -> we reserve a slot (and potential alloc) even if nothing is inserted

    const size_type hash = HashValue_(rvalue);
#if USE_PPE_ASSERT
    const size_type index = FindEmptyBucket_(table_traits::Key(rvalue), hash);
#else
    const size_type index = FindEmptyBucket_(hash);
#endif

    if (not _data.SetElement(index, hash))
        AssertNotReached();

    _data.Size++;
    Meta::Construct(BucketAt_(index), std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::insert_or_assign(const value_type& value) -> TPair<iterator, bool> {
    const auto result = InsertIFN_(table_traits::Key(value), HashKey_(table_traits::Key(value)), value);
    if (not result.second)
        *reinterpret_cast<pointer>(result.first.data()) = value;

    return result;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::insert_or_assign(value_type&& rvalue) -> TPair<iterator, bool> {
    const auto result = InsertIFN_(table_traits::Key(rvalue), HashKey_(table_traits::Key(rvalue)), std::move(rvalue));
    if (not result.second)
        *reinterpret_cast<pointer>(result.first.data()) = std::move(rvalue);

    return result;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename... _Args>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::try_emplace(const key_type& key, _Args&&... args) -> TPair<iterator, bool> {
    return InsertIFN_(key, HashKey_(key), key, std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename... _Args>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::try_emplace(key_type&& rkey, _Args&&... args) -> TPair<iterator, bool> {
    return InsertIFN_(rkey, HashKey_(rkey), std::move(rkey), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename _KeyLike, typename... _Args>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::try_emplace_like(const _KeyLike& key, hash_t hash, _Args&&... args) -> TPair<iterator, bool> {
    return InsertIFN_(key, hash, std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::erase(const const_iterator& it) {
    Assert_NoAssume(AliasesToContainer(it));
    Assert(_data.Size > 0);

    pointer const buckets = ((pointer)_data.StatesAndBuckets + OffsetOfBuckets_());;
    const size_type index = checked_cast<size_type>((pointer)it.data() - buckets);

    --_data.Size;
    _data.SetDeleted(index);
    Meta::Destroy(buckets + index);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::erase_ReturnNext(const iterator& it) -> iterator {
    erase(it);
    return ++iterator(it);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::erase(const key_type& key, value_type* pValueIFP) {
    if (Unlikely(0 == _data.Size))
        return false;

    const size_type hash = HashKey_(key);
    const size_type index = FindFilledBucket_(key, hash);

    if (INDEX_NONE == index)
        return false;

    Assert(_data.Size > 0);
    --_data.Size;
    _data.SetDeleted(index);

    pointer pItem = BucketAt_(index);

    if (pValueIFP)
        *pValueIFP = std::move(*pItem);

    Meta::Destroy(pItem);

    return true;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::clear() {
    Assert(CheckInvariants());

    if (_data.Size)
        clear_keepSound_(Meta::has_trivial_destructor<value_type>{});
    else if (_data.StatesAndBuckets)
        _data.ResetStates(); // reset all tombstones

    Assert_NoAssume(0 == _data.Size);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::clear_ReleaseMemory() {
    Assert(CheckInvariants());

    if (_data.NumBuckets() != 0)
        clear_ReleaseMemory_(Meta::has_trivial_destructor<value_type>{});
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::rehash(size_type count) {
    const size_type newCapacity = FPlatformMaths::NextPow2(count);
    if (newCapacity > capacity())
        RelocateRehash_(newCapacity);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::reserve(size_type count) {
    if (size_type newCapacity = GrowIFN_ReturnNewCapacity_(count))
        RelocateRehash_(newCapacity);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::shrink_to_fit() {
    if (size_type newCapacity = ShrinkIFN_ReturnNewCapacity_(_data.Size))
        RelocateRehash_(newCapacity);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::swap(TBasicHashTable& other) NOEXCEPT {
    typedef typename allocator_traits::propagate_on_container_swap propagate_type;
    if (this != &other)
        swap_(other, propagate_type());
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::CheckInvariants() const {
    STATIC_ASSERT(MaxLoadFactor < 100);
    STATIC_ASSERT(sizeof(value_type) == sizeof(public_type));
    STATIC_ASSERT(sizeof(value_type) >= sizeof(state_t));
#if USE_PPE_ASSERT
    const size_type n = capacity();
    if (0 != n && false == Meta::IsPow2(n))
        return false;
    if (nullptr == _data.StatesAndBuckets && (_data.Size || n))
        return false;
    if (_data.Size > n)
        return false;
    if (_data.StatesAndBuckets && static_cast<const state_t*>(_data.StatesAndBuckets)[_data.CapacityM1 + FHTD::GGroupSize] != FHTD::kSentinel)
        return false;
#endif
    return true;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::AliasesToContainer(const void* p) const {
    return ((u8*)BucketAtUnsafe_(0) <= (u8*)p && (u8*)BucketAtUnsafe_(_data.NumBuckets()/* end */) > (u8*)p);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::AliasesToContainer(const_reference v) const {
    return ((u8*)BucketAtUnsafe_(0) <= (u8*)&v && (u8*)BucketAtUnsafe_(_data.NumBuckets()/* end */) > (u8*)&v);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::AliasesToContainer(const_iterator it) const {
    return it.AliasesToContainer(TMemoryView(static_cast<const state_t*>(_data.StatesAndBuckets), capacity()));
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::operator ==(const TBasicHashTable& other) const {
    if (this == &other)
        return true;
    if (size() != other.size())
        return false;
    if (0 == _data.Size)
        return true;

    const size_type c = capacity();
    const pointer buckets = BucketAt_(0);

    ONLY_IF_ASSERT(size_t n = 0);
    for (size_type i = 0; i < c; i += FHTD::GGroupSize) {
        for (bitmask_t filled = FHTD::MatchFilledBucket(_data.GroupAt(i)); filled; ) {
            const size_type lhs = (i + filled.PopFront_AssumeNotEmpty())/* can't overflow here due to loop */;
            const size_type hsh = HashValue_(buckets[lhs]);
            const size_type rhs = other.FindFilledBucket_(table_traits::Key(buckets[lhs]), hsh);

            if (INDEX_NONE == rhs)
                return false;

            ONLY_IF_ASSERT(n++);
        }
    }

    Assert_NoAssume(n == size());
    return true;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::allocator_copy_(const allocator_type& other, std::true_type ) {
    if (not allocator_traits::Equals(*this, other)) {
        clear_ReleaseMemory();
        Assert(0 == _data.Size);
        Assert(nullptr == _data.StatesAndBuckets);
        allocator_traits::Copy(this, other);
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::allocator_move_(allocator_type&& rvalue, std::true_type ) {
    if (not allocator_traits::Equals(*this, rvalue)) {
        clear_ReleaseMemory();
        Assert(0 == _data.Size);
        Assert(nullptr == _data.StatesAndBuckets);
        allocator_traits::Move(this, std::move(rvalue));
    }
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
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::clear_keepSound_(std::true_type) {
    _data.Size = 0;
    _data.ResetStates();
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::clear_keepSound_(std::false_type) {
    const size_type n = capacity();
    const pointer buckets = BucketAt_(0);

    ONLY_IF_ASSERT(size_t sizeCheck = 0);

    for (size_type b = 0; b < n; b += FHTD::GGroupSize) {
        for(bitmask_t nonEmpty = FHTD::MatchEmpty(_data.GroupAt(b)).Invert(); nonEmpty; ) {
            const size_type index = (b + nonEmpty.PopFront_AssumeNotEmpty())/* can't overflow here due to loop */;

            if (not FHTD::IsEmptyOrDeleted(_data.SetState(index, FHTD::kEmpty))) {
                ONLY_IF_ASSERT(++sizeCheck);
                Meta::Destroy(buckets + index);
            }
        }
    }

    Assert_NoAssume(sizeCheck == _data.Size);
    _data.Size = 0;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::clear_leaveDirty_(std::true_type) {
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::clear_leaveDirty_(std::false_type) {
    if (0 == _data.Size)
        return;

    const size_type n = capacity();
    const pointer buckets = BucketAt_(0);

    ONLY_IF_ASSERT(size_t sizeCheck = 0);

    for (size_type b = 0; b < n; b += FHTD::GGroupSize) {
        for (bitmask_t filled = FHTD::MatchFilledBucket(_data.GroupAt(b)); filled; ) {
            const size_type index = (b + filled.PopFront_AssumeNotEmpty())/* can't overflow here due to loop */;
            Meta::Destroy(buckets + index);
            ONLY_IF_ASSERT(++sizeCheck);
        }
    }

    Assert_NoAssume(sizeCheck == _data.Size);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::clear_ReleaseMemory_(std::true_type) {
    Assert(_data.StatesAndBuckets);

    allocator_traits::Deallocate(*this, allocator_block_());

    _data.Size = 0;
    _data.CapacityM1 = UINT32_MAX; STATIC_ASSERT(std::is_same_v<u32, decltype(_data.CapacityM1)>);
    _data.StatesAndBuckets = nullptr;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::clear_ReleaseMemory_(std::false_type) {
    clear_leaveDirty_(std::false_type{});
    clear_ReleaseMemory_(std::true_type{});
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
FORCE_INLINE auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::GrowIFN_ReturnNewCapacity_(size_type atleast) const -> size_type {
    atleast += (atleast * SlackFactor) >> 7;// (atleast * (100 - MaxLoadFactor))/100;
    // if not empty can't be smaller than GGroupSize
    return ((atleast > 0 && atleast > _data.NumBuckets()) ? Max(FHTD::GGroupSize, FPlatformMaths::NextPow2(atleast)) : 0);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
FORCE_INLINE auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::ShrinkIFN_ReturnNewCapacity_(size_type atleast) const -> size_type {
    if (atleast == 0) return 0;
    atleast += (atleast * SlackFactor) >> 7;// (atleast * (100 - MaxLoadFactor))/100;
    // if not empty can't be smaller than GGroupSize
    atleast = Max(FHTD::GGroupSize, FPlatformMaths::NextPow2(atleast));
    return ((atleast < _data.NumBuckets()) ? atleast : 0);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename _KeyLike, typename... _Args>
FORCE_INLINE auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::InsertIFN_(const _KeyLike& key, hash_t hash, _Args&&... args) -> TPair<iterator, bool> {
    Assert_NoAssume(not AliasesToContainer(&key));
    Assert_NoAssume(not (0 | ... | AliasesToContainer(&args)));

    reserve_Additional(1); // TODO: problem here -> we reserve a slot (and potential alloc) even if nothing is inserted

    const size_type bucket = FindOrAllocateBucket_(key, hash);

    const iterator it = MakeIterator_(bucket);
    if (not _data.SetElement(bucket, hash))
        return MakePair(it, false);

    _data.Size++;
    Meta::Construct(
        reinterpret_cast<pointer>(remove_const(it.data())),
        table_traits::MakeValue(std::forward<_Args>(args)...) );

    return MakePair(it, true);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename _KeyLike>
auto FORCE_INLINE TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::FindFilledBucket_(const _KeyLike& keyLike, size_t hash) const NOEXCEPT -> size_type {
    Assert(0 != _data.Size);

    const __m128i h2_16 = _mm_set1_epi8(FHTD::H2(hash));

    for (size_type offset = (_data.H1(hash, Meta::has_trivial_copy<value_type>{}) & _data.CapacityM1), index = 0;;) {
        const auto group = _data.GroupAt(offset);

        for (bitmask_t match = FHTD::Match(group, h2_16); match; ) {
            const size_type bucket = ((offset + match.PopFront_AssumeNotEmpty()) & _data.CapacityM1/* can overflow due to wrapping */);
            const key_type& storedKey = table_traits::Key(*BucketAt_(bucket));
            if (Likely(static_cast<const _EqualTo&>(*this)(storedKey, keyLike)))
                return bucket;
        }

        if (FHTD::MatchEmpty(group))
            break;

        index += FHTD::GGroupSize;
        offset = ((offset + index) & _data.CapacityM1);
    }

    return INDEX_NONE;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
#if USE_PPE_ASSERT
template <typename _KeyLike>
auto FORCE_INLINE TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::FindEmptyBucket_(const _KeyLike& keyLikeForAssert, size_t hash) const NOEXCEPT -> size_type {
#else
auto FORCE_INLINE TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::FindEmptyBucket_(size_t hash) const NOEXCEPT -> size_type {
#endif
    Assert(_data.CapacityM1 > 0);
    Assert(_data.Size < (_data.CapacityM1 + 1));

    size_type bucket = (_data.H1(hash, Meta::has_trivial_copy<value_type>{}) & _data.CapacityM1);
    ONLY_IF_ASSERT(const __m128i h2_16 = _mm_set1_epi8(FHTD::H2(hash)));

    for (size_type index = 0;;) {
        const auto group = _data.GroupAt(bucket);

        if (const bitmask_t freeMask = FHTD::MatchEmptyOrDeleted(group))
            return ((bucket + freeMask.FirstBitSet_AssumeNotEmpty()) & _data.CapacityM1/* can overflow due to wrapping */);

#if USE_PPE_ASSERT
        bitmask_t match = FHTD::Match(group, h2_16);
        while (const size_type offsetP1 = match.PopFront()) {
            const size_type it = (bucket + offsetP1 - 1);
            if (Unlikely(static_cast<const _EqualTo&>(*this)(table_traits::Key(*BucketAt_(it)), keyLikeForAssert)))
                AssertNotReached(); // key already exists !
        }
#endif

        index += FHTD::GGroupSize;
        bucket = (bucket + index) & _data.CapacityM1;
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename _KeyLike>
auto FORCE_INLINE TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::FindOrAllocateBucket_(const _KeyLike& keyLike, size_t hash) const NOEXCEPT -> size_type {
    Assert(_data.CapacityM1 > 0);
    Assert(_data.Size < (_data.CapacityM1 + 1));

    pointer const buckets = BucketAt_(0);
    const __m128i h2_16 = _mm_set1_epi8(FHTD::H2(hash));

    for (size_type offset = (_data.H1(hash, Meta::has_trivial_copy<value_type>{}) & _data.CapacityM1), index = 0;;) {
        const auto group = _data.GroupAt(offset);

        for (bitmask_t match = FHTD::Match(group, h2_16); match; ) {
            const size_type bucket = ((offset + match.PopFront_AssumeNotEmpty()) & _data.CapacityM1/* can overflow due to wrapping */);
            const key_type& storedKey = table_traits::Key(buckets[bucket]);
            if (Likely(static_cast<const _EqualTo&>(*this)(storedKey, keyLike)))
                return bucket;
        }

        if (const bitmask_t freeMask = FHTD::MatchEmptyOrDeleted(group))
            return ((offset + freeMask.FirstBitSet_AssumeNotEmpty()) & _data.CapacityM1/* can overflow due to wrapping */);

        index += FHTD::GGroupSize;
        offset = ((offset + index) & _data.CapacityM1);
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
NO_INLINE void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::RelocateRehash_(size_type newCapacity) {
    Assert(newCapacity > 0 && Meta::IsPow2(newCapacity));
    Assert(_data.Size <= newCapacity);

    const FHTD oldData = _data;
    const size_t oldCapacity = capacity();
    const size_t oldOffsetOfBuckets = OffsetOfBuckets_();
    Assert(oldCapacity != newCapacity);

    _data.CapacityM1 = checked_cast<u32>(newCapacity) - 1;
    _data.StatesAndBuckets = allocator_traits::Allocate(*this, (OffsetOfBuckets_() + newCapacity) * sizeof(value_type)).Data;
    _data.ResetStates();

    Assert(Meta::IsAlignedPow2(FHTD::GGroupSize, capacity()));

    if (0 == _data.Size) {
        Assert(nullptr == oldData.StatesAndBuckets);
        return;
    }

    Assert(oldData.StatesAndBuckets);
    Assert(oldCapacity > 0 && Meta::IsPow2(oldCapacity));

    const pointer newBuckets = BucketAt_(0);
    const pointer oldBuckets = (reinterpret_cast<pointer>(oldData.StatesAndBuckets) + oldOffsetOfBuckets);

    ONLY_IF_ASSERT(size_t sizeCheck = 0);

    for (size_type i = 0; i < oldCapacity; i += FHTD::GGroupSize) {
        for (bitmask_t filled = FHTD::MatchFilledBucket(oldData.GroupAt(i)); filled; ) {
            const size_type src = (i + filled.PopFront_AssumeNotEmpty())/* can't overflow here due to loop */;
            const size_t hash = HashValue_(oldBuckets[src]); // still needs rehashing here, but THashMemoizer<> can amortize this cost for heavy hash functions

#if USE_PPE_ASSERT
            const size_type dst = FindEmptyBucket_(table_traits::Key(oldBuckets[src]), hash);
#else
            const size_type dst = FindEmptyBucket_(hash);
#endif
            Assert(INDEX_NONE != dst);

            Assert(FHTD::H2(hash) == *oldData.State(src)); // checks that the hash function is stable, could also be caused by a corrupted item
            if (not _data.SetElement(dst, hash))
                AssertNotReached();

            Meta::Construct(&newBuckets[dst], std::move(oldBuckets[src]));
            Meta::Destroy(&oldBuckets[src]);

            ONLY_IF_ASSERT(++sizeCheck);
        }
    }

    Assert_NoAssume(sizeCheck == _data.Size);

    allocator_traits::Deallocate(*this, FAllocatorBlock{
        oldData.StatesAndBuckets,
        (oldOffsetOfBuckets + oldCapacity) * sizeof(value_type) });

    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
