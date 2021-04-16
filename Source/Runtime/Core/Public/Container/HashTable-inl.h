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
    std::swap(Capacity, other.Capacity);
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
    const size_type n = capacity();
    Assert(Meta::IsAligned(FHTD::GGroupSize, n));

    size_type occupied = 0;
    for (size_type i = 0; i < n; i += FHTD::GGroupSize) {
        // include kDeleted for load_factor() since it's directly affecting probing performance
        occupied += FHTD::MatchNonEmpty(_data.GroupAt_StreamLoad(i)).Count();
    }

    return (float(occupied) / n);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::max_probe_dist() const -> size_type {
    if (0 == _data.Size)
        return 0;

    const size_type capacityM1 = size_type(_data.Capacity - 1);
    const size_type n = (_data.Capacity);
    const const_pointer buckets = BucketAt_(0);

    size_type max_dist = 0;
    ONLY_IF_ASSERT(size_type sizeCheck = 0);

    forrange(it, begin(), end()) {
        const const_pointer p = (const_pointer)it.data();
        const size_type hash = HashValue_(*p);

        const size_type wanted = (FHTD::H1(hash) & capacityM1);
        const size_type inserted = (p - buckets);

        const size_type probe_dist = (((inserted + n) - wanted) & capacityM1);

        max_dist = Max(max_dist, probe_dist);

        ONLY_IF_ASSERT(++sizeCheck);
    }

    Assert_NoAssume(sizeCheck == _data.Size);
    return max_dist;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::assign(TBasicHashTable&& rvalue) {
    Assert(&rvalue != this);

    FAllocatorBlock b{
        rvalue._data.StatesAndBuckets,
        (rvalue.OffsetOfBuckets_() + rvalue.capacity()) * sizeof(value_type) };

    const bool moved = MoveAllocatorBlock(&allocator_traits::Get(*this), allocator_traits::Get(rvalue), b);

    if (moved) {
        clear_ReleaseMemory();

        std::swap(_data, rvalue._data);
    }
    else {
        assign(
            MakeMoveIterator(rvalue.begin()),
            MakeMoveIterator(rvalue.end()) );

        rvalue.clear();
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::find(const key_type& key) NOEXCEPT -> iterator {
    return find_like(key, HashKeyNoSeed_(key));
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
FORCE_INLINE auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::find(const key_type& key) const NOEXCEPT -> const_iterator {
    return find_like(key, HashKeyNoSeed_(key));
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename _KeyLike>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::find_like(const _KeyLike& keyLike, hash_t hash) NOEXCEPT -> iterator {
    using public_pointer = typename iterator::pointer;

    hash = SeedHash_(hash);

    pointer const buckets = ((pointer)_data.StatesAndBuckets + OffsetOfBuckets_());

    if (Likely(_data.Size)) {
        const size_type capacityM1 = size_type(_data.Capacity - 1);
        const ::__m128i h2_16 = ::_mm_set1_epi8(FHTD::H2(hash));

        size_type start = (FHTD::H1(hash) & capacityM1);

        for (;;) {
            auto group = _data.GroupAt(start);
            FBitMask match_16 = FHTD::Match(group, h2_16);

            while (match_16) {
                const size_type index = ((start + match_16.PopFront_AssumeNotEmpty()) & capacityM1/* can overflow due to wrapping */);
                if (Likely(static_cast<const _EqualTo&>(*this)(table_traits::Key(buckets[index]), keyLike)))
                    return iterator((const state_t*)_data.StatesAndBuckets, (public_pointer)buckets, _data.Capacity, index);
            }

            if (FHTD::MatchEmpty(group))
                break;

            start = ((start + FHTD::GGroupSize) & capacityM1);
        }
    }

    return iterator((const state_t*)_data.StatesAndBuckets, (public_pointer)buckets, _data.Capacity, _data.Capacity);
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
    return InsertIFN_(table_traits::Key(value), value);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::insert(value_type&& rvalue) -> TPair<iterator, bool> {
    return InsertIFN_(table_traits::Key(rvalue), std::move(rvalue));
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
    const auto result = InsertIFN_(table_traits::Key(value), value);
    if (not result.second)
        *(pointer)result.first.data() = value;

    return result;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::insert_or_assign(value_type&& rvalue) -> TPair<iterator, bool> {
    const auto result = InsertIFN_(table_traits::Key(rvalue), std::move(rvalue));
    if (not result.second)
        *(pointer)result.first.data() = std::move(rvalue);

    return result;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename... _Args>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::try_emplace(const key_type& key, _Args&&... args) -> TPair<iterator, bool> {
    return InsertIFN_(key, key, std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename... _Args>
auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::try_emplace(key_type&& rkey, _Args&&... args) -> TPair<iterator, bool> {
    return InsertIFN_(rkey, std::move(rkey), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::erase(const const_iterator& it) {
    Assert_NoAssume(AliasesToContainer(it));
    Assert(_data.Size > 0);

    pointer const buckets = ((pointer)_data.StatesAndBuckets + OffsetOfBuckets_());;
    const size_type index = checked_cast<size_type>((pointer)it.data() - buckets);

    _data.Size--;
    _data.SetDeleted(index);
    Meta::Destroy(buckets + index);
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
    _data.Size--;
    _data.SetDeleted(index);

    pointer pitem = BucketAt_(index);

    if (pValueIFP)
        *pValueIFP = std::move(*pitem);

    Meta::Destroy(pitem);

    return true;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::clear() {
    Assert(CheckInvariants());

    if (_data.Size)
        clear_keepSound_(std::bool_constant<Meta::is_pod_v<value_type>>{});
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
void TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::clear_ReleaseMemory() {
    Assert(CheckInvariants());

    if (_data.Capacity)
        clear_ReleaseMemory_(std::bool_constant<Meta::is_pod_v<value_type>>{});
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
    if (_data.StatesAndBuckets && ((const state_t*)_data.StatesAndBuckets)[_data.Capacity + FHTD::GGroupSize - 1] != FHTD::kSentinel)
        return false;
#endif
    return true;
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::AliasesToContainer(const void* p) const {
    return ((u8*)BucketAtUnsafe_(0) <= (u8*)p && (u8*)BucketAtUnsafe_(_data.Capacity/* end */) > (u8*)p);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::AliasesToContainer(const_reference v) const {
    return ((u8*)BucketAtUnsafe_(0) <= (u8*)&v && (u8*)BucketAtUnsafe_(_data.Capacity/* end */) > (u8*)&v);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::AliasesToContainer(const_iterator it) const {
    return it.AliasesToContainer(TMemoryView<const state_t>((const state_t*)_data.StatesAndBuckets, capacity()));
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::operator ==(const TBasicHashTable& other) const {
    if (this == &other)
        return true;
    else if (size() != other.size())
        return false;
    else if (0 == _data.Size)
        return true;

    const size_type c = capacity();
    const pointer buckets = BucketAt_(0);

    ONLY_IF_ASSERT(size_t n = 0);
    for (size_type i = 0; i < c; i += FHTD::GGroupSize) {
        FBitMask filled = FHTD::MatchFilledBucket(_data.GroupAt_StreamLoad(i));

        while (filled) {
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
        FBitMask nonEmpty = FHTD::MatchNonEmpty(_data.GroupAt_StreamLoad(b));

        while (nonEmpty) {
            const size_type index = (b + nonEmpty.PopFront_AssumeNotEmpty())/* can't overflow here due to loop */;

            if (not (_data.SetState(index, FHTD::kEmpty) & FHTD::kDeleted)) {
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
        FBitMask filled = FHTD::MatchFilledBucket(_data.GroupAt_StreamLoad(b));

        while (filled) {
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

    allocator_traits::Deallocate(*this, FAllocatorBlock{
        _data.StatesAndBuckets,
        (OffsetOfBuckets_() + capacity()) * sizeof(value_type)
        });

    _data.Size = 0;
    _data.Capacity = 0;
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
    return ((atleast > 0 && atleast > (_data.Capacity)) ? Max(FHTD::GGroupSize, FPlatformMaths::NextPow2(atleast)) : 0);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
FORCE_INLINE auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::ShrinkIFN_ReturnNewCapacity_(size_type atleast) const -> size_type {
    if (atleast == 0) return 0;
    atleast += (atleast * SlackFactor) >> 7;// (atleast * (100 - MaxLoadFactor))/100;
    // if not empty can't be smaller than GGroupSize
    atleast = Max(FHTD::GGroupSize, FPlatformMaths::NextPow2(atleast));
    return ((atleast < (_data.Capacity)) ? atleast : 0);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename... _Args>
FORCE_INLINE auto TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::InsertIFN_(const key_type& key, _Args&&... args) -> TPair<iterator, bool> {
    Assert_NoAssume(not AliasesToContainer(&key));
    Assert_NoAssume(not (0 | ... | AliasesToContainer(&args)));

    reserve_Additional(1); // TODO: problem here -> we reserve a slot (and potential alloc) even if nothing is inserted

    const size_type hash = HashKey_(key);
    const size_type bucket = FindOrAllocateBucket_(key, hash);

    const iterator it = MakeIterator_(bucket);
    if (not _data.SetElement(bucket, hash))
        return MakePair(it, false);

    _data.Size++;
    Meta::Construct(
        (pointer)it.data(),
        table_traits::MakeValue(std::forward<_Args>(args)...) );

    return MakePair(it, true);
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename _KeyLike>
auto FORCE_INLINE TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::FindFilledBucket_(const _KeyLike& keyLike, size_t hash) const NOEXCEPT -> size_type {
    Assert(0 != _data.Size);

    const size_type capacityM1 = size_type(_data.Capacity - 1);
    __m128i h2_16 = _mm_set1_epi8(FHTD::H2(hash));
    size_type start = (FHTD::H1(hash) & capacityM1);

    for (;;) {
        auto group = _data.GroupAt(start);
        FBitMask match = FHTD::Match(group, h2_16);

        while (match) {
            const size_type index = ((start + match.PopFront_AssumeNotEmpty()) & capacityM1/* can overflow due to wrapping */);
            const key_type& storedKey = table_traits::Key(*BucketAt_(index));
            if (Likely(static_cast<const _EqualTo&>(*this)(storedKey, keyLike)))
                return index;
        }

        if (FHTD::MatchEmpty(group))
            break;

        start = ((start + FHTD::GGroupSize) & capacityM1);
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
    Assert(_data.Capacity > 0);
    Assert(_data.Size < _data.Capacity);

    const size_type capacityM1 = size_type(_data.Capacity - 1);
    size_type bucket = (FHTD::H1(hash) & capacityM1);
    ONLY_IF_ASSERT(__m128i h2_16 = _mm_set1_epi8(FHTD::H2(hash)));

    for (;;) {
        auto group = _data.GroupAt(bucket);
        FBitMask freeMask = FHTD::MatchFreeBucket(group);

        if (freeMask)
            return ((bucket + freeMask.FirstBitSet_AssumeNotEmpty()) & capacityM1/* can overflow due to wrapping */);

#if USE_PPE_ASSERT
        FBitMask match = FHTD::Match(group, h2_16);
        while (size_type offsetP1 = match.PopFront()) {
            const size_type index = (bucket + offsetP1 - 1);
            if (Unlikely(static_cast<const _EqualTo&>(*this)(table_traits::Key(*BucketAt_(index)), keyLikeForAssert)))
                AssertNotReached(); // key already exists !
        }
#endif

        bucket = (bucket + FHTD::GGroupSize) & capacityM1;
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
template <typename _KeyLike>
auto FORCE_INLINE TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>::FindOrAllocateBucket_(const _KeyLike& keyLike, size_t hash) const NOEXCEPT -> size_type {
    Assert(_data.Capacity > 0);
    Assert(_data.Size < _data.Capacity);

    const size_type capacityM1 = size_type(_data.Capacity - 1);
    pointer const buckets = BucketAt_(0);
    __m128i h2_16 = _mm_set1_epi8(FHTD::H2(hash));
    size_type start = (FHTD::H1(hash) & capacityM1);

    for (;;) {
        auto group = _data.GroupAt(start);
        FBitMask match = FHTD::Match(group, h2_16);

        while (match) {
            const size_type index = ((start + match.PopFront_AssumeNotEmpty()) & capacityM1/* can overflow due to wrapping */);
            const key_type& storedKey = table_traits::Key(buckets[index]);
            if (Likely(static_cast<const _EqualTo&>(*this)(storedKey, keyLike)))
                return index;
        }

        if (const FBitMask freeMask = FHTD::MatchFreeBucket(group))
            return ((start + freeMask.FirstBitSet_AssumeNotEmpty()) & capacityM1/* can overflow due to wrapping */);

        start = ((start + FHTD::GGroupSize) & capacityM1);
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

    _data.Capacity = checked_cast<u32>(newCapacity);
    _data.StatesAndBuckets = allocator_traits::Allocate(*this, (OffsetOfBuckets_() + newCapacity) * sizeof(value_type)).Data;
    _data.ResetStates();

    Assert(Meta::IsAligned(FHTD::GGroupSize, capacity()));

    if (0 == _data.Size) {
        Assert(nullptr == oldData.StatesAndBuckets);
        return;
    }

    Assert(oldData.StatesAndBuckets);
    Assert(oldCapacity > 0 && Meta::IsPow2(oldCapacity));

    const pointer newBuckets = BucketAt_(0);
    const pointer oldBuckets = ((pointer)oldData.StatesAndBuckets + oldOffsetOfBuckets);

    ONLY_IF_ASSERT(size_t sizeCheck = 0);

    for (size_type i = 0; i < oldCapacity; i += FHTD::GGroupSize) {
        FBitMask filled = FHTD::MatchFilledBucket(oldData.GroupAt_StreamLoad(i));

        while (filled) {
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
