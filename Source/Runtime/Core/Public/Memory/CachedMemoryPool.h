#pragma once

#include "Core_fwd.h"

#include "Container/HashMap.h"
#include "Memory/MemoryPool.h"
#include "Meta/Functor.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
class TCachedMemoryPool : Meta::FNonCopyableNorMovable {
    using pool_index_type =
        std::conditional_t<(_ChunkSize * _MaxChunks > UINT32_MAX), u64,
        std::conditional_t<(_ChunkSize * _MaxChunks > UINT16_MAX), u32, u16> >;
    template <typename K, typename V>
    class TCacheItem_ {
        const K _key;
        V _value;
        const hash_t _hashValue;
        const pool_index_type _poolIndex;
    public:
        STATIC_CONST_INTEGRAL(bool, has_separated_key, true);
        TCacheItem_* NextItem{ nullptr };
        TCacheItem_(K&& rkey, hash_t hashValue, pool_index_type poolIndex) NOEXCEPT
        :   _key(std::move(rkey))
        ,   _hashValue(hashValue)
        ,   _poolIndex(poolIndex)
        {}
        ~TCacheItem_() {
            const_cast<pool_index_type&>(_poolIndex) = UMax;
        }
        const K& key() const { return _key; }
        V& Value() { return _value; }
        const V& Value() const { return _value; }
        hash_t HashValue() const { return _hashValue; }
        pool_index_type PoolIndex() const { return _poolIndex; }
        V* MutableValue() const { return &_value; }
    };

    template <typename T>
    class TCacheItem_<T, T> { // immutable when _Key == _Value
        const T _item;
        const hash_t _hashValue;
        const pool_index_type _poolIndex;
    public:
        STATIC_CONST_INTEGRAL(bool, has_separated_key, false);
        TCacheItem_* NextItem{ nullptr };
        TCacheItem_(T&& ritem, hash_t hashValue, pool_index_type poolIndex) NOEXCEPT
        :   _item(std::move(ritem))
        ,   _hashValue(hashValue)
        ,   _poolIndex(poolIndex)
        {}
        ~TCacheItem_() {
            const_cast<pool_index_type&>(_poolIndex) = UMax;
        }
        const T& Key() const { return _item; }
        const T& Value() const { return _item; }
        hash_t HashValue() const { return _hashValue; }
        pool_index_type PoolIndex() const { return _poolIndex; }
        T* MutableValue() const { return const_cast<T*>(&_item); }
    };

    using FCacheItem_ = TCacheItem_<_Key, _Value>;
    using pool_type = TTypedMemoryPool<FCacheItem_, _ChunkSize, _MaxChunks, _Allocator>;

    using FHashSegment_ = FAtomicOrderedLock;
    using FHashBucket_ = FCacheItem_*;
    struct FHashTable_ {
        STATIC_CONST_INTEGRAL(u32, SegmentMask, FGenericPlatformMaths::NextPow2(static_cast<u32>(pool_type::ChunkSize >> 3)) - 1);
        STATIC_CONST_INTEGRAL(u32, BucketMask, FGenericPlatformMaths::NextPow2(static_cast<u32>(pool_type::ChunkSize) + (pool_type::ChunkSize >> 1)) - 1);

        std::atomic<u32> Count{ 0 };
        FHashSegment_* pSegments;
        FHashBucket_* pBuckets;

        FHashSegment_& Segment(hash_t h) const { return pSegments[h & SegmentMask]; }
        FHashBucket_& Bucket(hash_t h) const { return pBuckets[h & BucketMask]; }
    };

    STATIC_ASSERT(std::is_default_constructible_v<_Value>); // use ctor to initialize the object

public:
    using allocator_type = typename pool_type::allocator_type;
    using allocator_traits = typename pool_type::allocator_traits;

    using key_type = _Key;
    using value_type = _Value;
    using index_type = typename pool_type::index_type;
    using block_type = typename pool_type::value_type;
    STATIC_ASSERT(std::is_same_v<pool_index_type, index_type>);

    STATIC_CONST_INTEGRAL(size_t, MaxSize, pool_type::MaxSize);

    TCachedMemoryPool() { InitializeInternalCache_(); }

    explicit TCachedMemoryPool(_Allocator&& ralloc) : _pool(_Allocator(ralloc)) { InitializeInternalCache_(); }
    explicit TCachedMemoryPool(const _Allocator& alloc) : _pool(alloc) { InitializeInternalCache_(); }

    ~TCachedMemoryPool();

    index_type NumCachedBlocks() const NOEXCEPT {
        return checked_cast<index_type>(_cache.Count.load(std::memory_order_relaxed));
    }

    const value_type* At(index_type id) const NOEXCEPT {
        block_type* const pBlock = _pool[id];
        Assert(pBlock);
        Assert_NoAssume(pBlock->PoolIndex() == id);
        return std::addressof(pBlock->Value());
    }
    value_type* AtIFP(index_type id) const NOEXCEPT {
        block_type* const pBlock = _pool[id];
        if (pBlock && pBlock->PoolIndex() == id) {
            return pBlock->MutableValue();
        }
        return nullptr;
    }
    value_type* operator [](index_type id) const NOEXCEPT {
        return AtIFP(id);
    }

    const key_type& Key(index_type id) const NOEXCEPT { return _pool.At(id)->Key(); }
    value_type& Value(index_type id) NOEXCEPT { return *_pool.At(id)->MutableValue(); }
    const value_type& Value(index_type id) const NOEXCEPT { return _pool.At(id)->Value(); }

    template <typename _Ctor>
    TPair<index_type, bool> FindOrAdd(key_type&& rkey, _Ctor&& ctor);
    template <typename _Dtor>
    bool RemoveIf(index_type id, _Dtor&& dtor);
    bool Remove(index_type id);

    void Clear_AssertCompletelyEmpty();
    template <typename _Dtor>
    void Clear_IgnoreLeaks(_Dtor&& dtor);
    void Clear_IgnoreLeaks();

    using FGCHandle = std::atomic<size_t>;
    template <typename _Dtor>
    size_t GarbageCollect(FGCHandle& gc, size_t maxIterations, _Dtor&& dtor);

    bool CheckInvariants() const;

private:
    FHashTable_ _cache;
    pool_type _pool;

    void InitializeInternalCache_();
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
void TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::InitializeInternalCache_() {
    Assert_NoAssume(_cache.Count.load(std::memory_order_relaxed) == 0);

    const u32 segmentCapacity = (FHashTable_::SegmentMask + 1);
    const u32 bucketsCapacity = (FHashTable_::BucketMask + 1);
    Assert_NoAssume(segmentCapacity <= bucketsCapacity);
    Assert_NoAssume(Meta::IsPow2(segmentCapacity));
    Assert_NoAssume(Meta::IsPow2(bucketsCapacity));

    const size_t segmentSizeInBytes = ROUND_TO_NEXT_CACHELINE(segmentCapacity * sizeof(FHashSegment_));
    const size_t bucketSizeInBytes = (bucketsCapacity * sizeof(FHashBucket_));

    const FAllocatorBlock blk{ allocator_traits::Allocate(_pool.Allocator(), segmentSizeInBytes + bucketSizeInBytes) };

    _cache.pSegments = static_cast<FHashSegment_*>(blk.Data);
    _cache.pBuckets = reinterpret_cast<FHashBucket_*>(static_cast<u8*>(blk.Data) + segmentSizeInBytes);

    Meta::Construct(TMemoryView{ _cache.pSegments, FHashTable_::SegmentMask + 1 });
    Broadcast(TMemoryView{ _cache.pBuckets, FHashTable_::BucketMask + 1 }, nullptr);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::~TCachedMemoryPool() {
    Assert_NoAssume(_cache.Count.load(std::memory_order_relaxed) == 0);

    const size_t segmentSizeInBytes = ROUND_TO_NEXT_CACHELINE((FHashTable_::SegmentMask + 1) * sizeof(FHashSegment_));
    const size_t bucketSizeInBytes = ((FHashTable_::BucketMask + 1) * sizeof(FHashBucket_));

    const FAllocatorBlock blk{ _cache.pSegments, segmentSizeInBytes + bucketSizeInBytes };
    allocator_traits::Deallocate(_pool.Allocator(), blk);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
template <typename _Ctor>
auto TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::FindOrAdd(key_type&& rkey, _Ctor&& ctor) -> TPair<index_type, bool> {

    const hash_t h = hash_value(rkey);
    const FHashSegment_::FScope scopeLock( _cache.Segment(h) );

    auto*& pBucket = _cache.Bucket(h);

    for (FCacheItem_* p = pBucket; p; p = p->NextItem) {
        if (p->HashValue() == h && p->Key() == rkey) {
            Verify( Meta::VariadicFunctor(ctor, p->MutableValue(), p->PoolIndex(), true) );
            return MakePair(p->PoolIndex(), true);
        }
    }

    const index_type id = _pool.Allocate();
    FCacheItem_* const pBlock = _pool.At(id);
    INPLACE_NEW(pBlock, FCacheItem_){ std::move(rkey), h, id };

    if (not Meta::VariadicFunctor(ctor, pBlock->MutableValue(), id, false)) {
        Meta::Destroy(pBlock);
        _pool.Deallocate(id);

        return MakePair(UMax, false);
    }

    pBlock->NextItem = pBucket;
    pBucket = pBlock;
    _cache.Count.fetch_add(1, std::memory_order_relaxed);

    Assert(id < pool_type::MaxSize);
    return MakePair(id, false);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
bool TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::Remove(index_type id) {
    return RemoveIf(id, []() constexpr -> bool { return true; });
}
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
template <typename _Dtor>
bool TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::RemoveIf(index_type id, _Dtor&& dtor) {
    FCacheItem_* const pBlock = _pool.At(id);
    Assert_NoAssume(pBlock->PoolIndex() == id);

    const FHashSegment_::FScope scopeLock( _cache.Segment(pBlock->HashValue()) );

    bool shouldRemove;
    IF_CONSTEXPR(FCacheItem_::has_separated_key)
        shouldRemove = Meta::VariadicFunctor(dtor, pBlock->MutableValue(), pBlock->Key(), id);
    else
        shouldRemove = Meta::VariadicFunctor(dtor, pBlock->MutableValue(), id);

    if (Unlikely(shouldRemove)) {
        FHashBucket_& pBucket = _cache.Bucket(pBlock->HashValue());

        ONLY_IF_ASSERT(bool found = false);
        for (FCacheItem_* p = pBucket, *prev = nullptr; p; prev = p, p = p->NextItem) {
            Assert_NoAssume(_cache.Bucket(p->HashValue()) == pBucket);

            if (p == pBlock) {
                ONLY_IF_ASSERT(found = true);
                if (prev)
                    prev->NextItem = pBlock->NextItem;
                else
                    pBucket = pBlock->NextItem;
                break;
            }
        }

        Assert_NoAssume( found );
        Verify( _cache.Count.fetch_sub(1, std::memory_order_relaxed) > 0 );

        Meta::Destroy(pBlock);
        _pool.Deallocate(id);

        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
void TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::Clear_AssertCompletelyEmpty() {
    Assert_NoAssume(_cache.Count.load(std::memory_order_relaxed) == 0);

    u32 segment = UINT32_MAX;
    forrange(b, 0, FHashTable_::BucketMask + 1) {
        const u32 s = (b & FHashTable_::SegmentMask);
        if (segment != s) {
            if (segment <= FHashTable_::SegmentMask)
                _cache.pSegments[segment].Unlock();
            _cache.pSegments[s].Lock();
            segment = s;
        }

        _cache.pBuckets[b] = nullptr;
    }

    if (segment <= FHashTable_::SegmentMask)
        _cache.pSegments[segment].Unlock();

    _cache.Count.store(0, std::memory_order_release);
    _pool.Clear_AssertCompletelyEmpty();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
void TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::Clear_IgnoreLeaks() {
    Clear_IgnoreLeaks([]() constexpr -> bool { return true; });
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
template <typename _Dtor>
void TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::Clear_IgnoreLeaks(_Dtor&& dtor) {
    u32 segment = UINT32_MAX;
    forrange(b, 0, FHashTable_::BucketMask + 1) {
        const u32 s = (b & FHashTable_::SegmentMask);
        if (segment != s) {
            if (segment <= FHashTable_::SegmentMask)
                _cache.pSegments[segment].Unlock();
            _cache.pSegments[s].Lock();
            segment = s;
        }

        for (FCacheItem_* p = _cache.pBuckets[b]; p; ) {
            FCacheItem_* const pNext = p->NextItem;

            const index_type id = p->PoolIndex();
            Assert_NoAssume((p->HashValue() & FHashTable_::BucketMask) == b);

            IF_CONSTEXPR(FCacheItem_::has_separated_key)
                Verify( Meta::VariadicFunctor(dtor, p->MutableValue(), p->Key(), id) );
            else
                Verify( Meta::VariadicFunctor(dtor, p->MutableValue(), id) );

            Meta::Destroy(p);
            _pool.Deallocate(id);

            Verify( _cache.Count.fetch_sub(1, std::memory_order_release) > 0 );
            p = pNext;
        }

        _cache.pBuckets[b] = nullptr;
    }

    if (segment <= FHashTable_::SegmentMask)
        _cache.pSegments[segment].Unlock();

    _pool.Clear_AssertCompletelyEmpty();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
template <typename _Dtor>
size_t TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::GarbageCollect(FGCHandle& gc, size_t maxIterations, _Dtor&& dtor) {
    const size_t offset = gc.fetch_add(maxIterations, std::memory_order_relaxed);

    size_t numObjectsReleased = 0;

    u32 segment = UINT32_MAX;
    forrange(i, 0, maxIterations) {
        const size_t b = (i + offset);
        const u32 s = (b & FHashTable_::SegmentMask);

        FHashBucket_& pBucket = _cache.Bucket(b);
        if (not pBucket)
            continue;

        if (segment != s) {
            if (segment <= FHashTable_::SegmentMask)
                _cache.pSegments[segment].Unlock();
            _cache.pSegments[s].Lock();
            segment = s;
        }

        for (FCacheItem_* p = pBucket, *prev = nullptr; p; ) {
            const index_type id = p->PoolIndex();

            bool shouldRemove;
            IF_CONSTEXPR(FCacheItem_::has_separated_key)
                shouldRemove = Meta::VariadicFunctor(dtor, p->MutableValue(), p->Key(), id);
            else
                shouldRemove = Meta::VariadicFunctor(dtor, p->MutableValue(), id);

            if (Unlikely(shouldRemove)) {
                FCacheItem_* const nextItem = p->NextItem;

                if (prev)
                    prev->NextItem = nextItem;
                else
                    pBucket = nextItem;

                Meta::Destroy(p);
                _pool.Deallocate(id);

                ++numObjectsReleased;
                Verify( _cache.Count.fetch_sub(1, std::memory_order_release) > 0 );

                p = nextItem;
            }
            else {
                prev = p;
                p = p->NextItem;
            }
        }
    }

    if (segment <= FHashTable_::SegmentMask)
        _cache.pSegments[segment].Unlock();

    return numObjectsReleased;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
bool TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::CheckInvariants() const {
#if USE_PPE_DEBUG

    auto numElements = _cache.Count.load(std::memory_order_acquire);

    u32 segment = UINT32_MAX;
    forrange(b, 0, FHashTable_::BucketMask + 1) {
        const u32 s = (b & FHashTable_::SegmentMask);
        if (segment != s) {
            if (segment <= FHashTable_::SegmentMask)
                _cache.pSegments[segment].Unlock();
            _cache.pSegments[s].Lock();
            segment = s;
        }

        for (FCacheItem_* p = _cache.pBuckets[b]; p; p = p->NextItem) {
            const index_type id = p->PoolIndex();
            Assert_NoAssume((p->HashValue() & FHashTable_::SegmentMask) == s);
            Assert_NoAssume((p->HashValue() & FHashTable_::BucketMask) == b);
            Assert_NoAssume(_pool.At(id) == p);

            Verify( numElements-- > 0 );
        }
    }

    Assert_NoAssume(0 == numElements);

    if (segment <= FHashTable_::SegmentMask)
        _cache.pSegments[segment].Unlock();

    return _pool.CheckInvariants();

#else
    return true;
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
