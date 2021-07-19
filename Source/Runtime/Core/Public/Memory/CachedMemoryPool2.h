#pragma once

#include "Core_fwd.h"

#include "Container/HashMap.h"
#include "Memory/MemoryPool4.h"
#include "Memory/PtrRef.h"
#include "Meta/Functor.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
class TCachedMemoryPool2 : Meta::FNonCopyableNorMovable {
    template <typename K, typename V>
    class TCacheItem_ {
        const K _key;
        V _value;
    public:
        STATIC_CONST_INTEGRAL(bool, has_separated_key, true);
        explicit TCacheItem_(K&& rkey) : _key(std::move(rkey)) {}
        const K& key() const { return _key; }
        V& Value() { return _value; }
        const V& Value() const { return _value; }
        V* MutableValue() const { return &_value; }
    };

    template <typename T>
    class TCacheItem_<T, T> { // immutable when _Key == _Value
        const T _item;
    public:
        STATIC_CONST_INTEGRAL(bool, has_separated_key, false);
        explicit TCacheItem_(T&& ritem) : _item(std::move(ritem)) {}
        const T& Key() const { return _item; }
        const T& Value() const { return _item; }
        T* MutableValue() const { return const_cast<T*>(&_item); }
    };

    using FCacheItem_ = TCacheItem_<_Key, _Value>;
    using FCacheKey_ = THashMemoizer<TPtrRef<const _Key>, Meta::THash<_Key>, Meta::TDerefEqualTo<TPtrRef<const _Key>> >; // memoize a ptr reference to the key in the block

    using pool_type = TTypedMemoryPool4<FCacheItem_, _ChunkSize, _MaxChunks, 4, _Allocator>;
    using cache_type = THashMap<
        FCacheKey_,
        typename pool_type::index_type, // index of the block
        Meta::THash<FCacheKey_>,
        Meta::TEqualTo<FCacheKey_>,
        _Allocator
    >;
    STATIC_ASSERT(std::is_default_constructible_v<_Value>); // use ctor to initialize the object

public:
    using key_type = _Key;
    using value_type = _Value;
    using index_type = typename pool_type::index_type;
    using block_type = typename pool_type::value_type;

    STATIC_CONST_INTEGRAL(size_t, MaxSize, pool_type::MaxSize);

    TCachedMemoryPool2() { InitializeInternalCache_(); }

    explicit TCachedMemoryPool2(_Allocator&& ralloc) : _cache(_Allocator(ralloc)), _pool(_Allocator(ralloc)) { InitializeInternalCache_(); }
    explicit TCachedMemoryPool2(const _Allocator& alloc) : _cache(alloc), _pool(alloc) { InitializeInternalCache_(); }

    index_type NumCachedBlocks() const NOEXCEPT { return checked_cast<index_type>(_cache.Value_NotThreadSafe().size()); }
    index_type NumCommittedBlocks() const NOEXCEPT { return checked_cast<index_type>(_pool.NumCommittedBlocks()); }

    const value_type* At(index_type id) const NOEXCEPT {
        return std::addressof(_pool.At(id)->Value());
    }
    auto* operator [](index_type id) const NOEXCEPT {
        block_type* const pBlock = _pool[id];
        return (pBlock ? pBlock->MutableValue() : nullptr);
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
    TThreadSafe<cache_type, EThreadBarrier::RWLock> _cache;
    pool_type _pool;

    void InitializeInternalCache_() {
        _cache.LockExclusive()->reserve(MaxSize);
    }
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
template <typename _Ctor>
auto TCachedMemoryPool2<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::FindOrAdd(key_type&& rkey, _Ctor&& ctor) -> TPair<index_type, bool> {
    STATIC_ASSERT(Meta::is_pod_v<FCacheKey_>);

    const FCacheKey_ cacheKey{ MakePtrRef(static_cast<const _Key&>(rkey)) };
    Assert_NoAssume(hash_value(cacheKey) == cacheKey.Hash());
    index_type id = _cache.LockShared()->GetCopy(cacheKey, UMax);

    bool exist = true;
    if (Likely(id != UMax)) {
        Meta::VariadicFunctor(ctor, _pool.At(id)->MutableValue(), id, true);
    }
    else {
        const auto exclusiveCache = _cache.LockExclusive();

        // checks if another thread already allocated a block for this key
        id = exclusiveCache->GetCopy(cacheKey, UMax);

        if (Likely(UMax == id)) {
            // allocate a new block from the pool

            exist = false;
            id = _pool.Allocate();

            block_type* const pBlock = _pool.At(id);
            INPLACE_NEW(pBlock, block_type){ std::move(rkey) };

            // run ctor and fail if returned false

            if (not Meta::VariadicFunctor(ctor, pBlock->MutableValue(), id, false)) {
                Assert_NoAssume(not exist); // #TODO: too restrictive ?
                _pool.Deallocate(id);
                return MakePair(UMax, false);
            }

            // finally register the new block in the cache
            Assert_NoAssume(hash_value(pBlock->Key()) == cacheKey.Hash());

            Insert_AssertUnique(*exclusiveCache,
                FCacheKey_{ MakePtrRef(pBlock->Key()), cacheKey.Hash() },
                id );

            Assert_NoAssume(exclusiveCache->find(MakePtrRef(pBlock->Key())) != exclusiveCache->end());
        }
        else {
            Meta::VariadicFunctor(ctor, _pool.At(id)->MutableValue(), id, true);
        }
    }

    Assert(id < pool_type::MaxSize);
    return MakePair(id, exist);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
bool TCachedMemoryPool2<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::Remove(index_type id) {
    return RemoveIf(id, []() constexpr -> bool { return true; });
}
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
template <typename _Dtor>
bool TCachedMemoryPool2<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::RemoveIf(index_type id, _Dtor&& dtor) {
    block_type* const pBlock = _pool.At(id);

    bool shouldRemove;
    IF_CONSTEXPR(FCacheItem_::has_separated_key)
        shouldRemove = Meta::VariadicFunctor(dtor, pBlock->MutableValue(), pBlock->Key(), id);
    else
        shouldRemove = Meta::VariadicFunctor(dtor, pBlock->MutableValue(), id);

    if (Unlikely(shouldRemove)) {
        const auto exclusiveCache = _cache.LockExclusive();

        exclusiveCache->erase(MakePtrRef(pBlock->Key()));

        Meta::Destroy(pBlock);

        _pool.Deallocate(id);

        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
void TCachedMemoryPool2<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::Clear_AssertCompletelyEmpty() {
    const auto exclusiveCache = _cache.LockExclusive();

    AssertRelease_NoAssume(exclusiveCache->empty());

    exclusiveCache->clear();
    _pool.Clear_AssertCompletelyEmpty();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
void TCachedMemoryPool2<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::Clear_IgnoreLeaks() {
    Clear_IgnoreLeaks([]() constexpr -> bool { return true; });
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
template <typename _Dtor>
void TCachedMemoryPool2<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::Clear_IgnoreLeaks(_Dtor&& dtor) {
    const auto exclusiveCache = _cache.LockExclusive();

    for (const auto& it : *exclusiveCache) {
        block_type* const pBlock = _pool.At(it.second);

        IF_CONSTEXPR(FCacheItem_::has_separated_key)
            Meta::VariadicFunctor(dtor, pBlock->MutableValue(), pBlock->Key(), it.second);
        else
            Meta::VariadicFunctor(dtor, pBlock->MutableValue(), it.second);

        Meta::Destroy(pBlock);
    }

    exclusiveCache->clear();

    _pool.Clear_IgnoreLeaks_AssumePOD();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
template <typename _Dtor>
size_t TCachedMemoryPool2<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::GarbageCollect(FGCHandle& gc, size_t maxIterations, _Dtor&& dtor) {
    const auto exclusiveCache = _cache.LockExclusive();

    const size_t offset = gc.fetch_add(maxIterations, std::memory_order_relaxed);

    size_t numObjectsReleased = 0;
    forrange(slot, offset, offset + maxIterations) {
        const auto it = exclusiveCache->At(slot);
        if (exclusiveCache->end() == it)
            continue;

        const index_type id = it->second;
        block_type* const pBlock = _pool.At(it->second);

        bool shouldRemove;
        IF_CONSTEXPR(FCacheItem_::has_separated_key)
            shouldRemove = Meta::VariadicFunctor(dtor, pBlock->MutableValue(), pBlock->Key(), id);
        else
            shouldRemove = Meta::VariadicFunctor(dtor, pBlock->MutableValue(), id);

        if (Unlikely(shouldRemove)) {
            Meta::Destroy(pBlock);

            exclusiveCache->erase(it);
            _pool.Deallocate(id);

            ++numObjectsReleased;
        }
    }

    return numObjectsReleased;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
bool TCachedMemoryPool2<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::CheckInvariants() const {
#if USE_PPE_DEBUG
    const auto exclusiveCache = const_cast<TCachedMemoryPool2*>(this)->_cache.LockExclusive();

    u32 cnt = 0;
    for (const auto& it : *exclusiveCache) {
        const block_type* const pBlock = _pool.At(it.second);
        Assert(pBlock);

        const FCacheKey_ test{ MakePtrRef(pBlock->Key()) };
        Assert_NoAssume(test.Hash() == it.first.Hash());
        Assert_NoAssume(std::addressof(*exclusiveCache->find(test)) == std::addressof(it));

        ++cnt;
    }

    Assert_NoAssume(_pool.NumLiveBlocks_ForDebug() == cnt);
    Assert_NoAssume(_pool.NumLiveBlocks_ForDebug() == exclusiveCache->size());

    return _pool.CheckInvariants();

#else
    return true;
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
