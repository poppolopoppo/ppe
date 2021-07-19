#pragma once

#include "Core_fwd.h"

#include "Container/FixedSizeHashTable.h"
#include "Memory/MemoryPool.h"
#include "Memory/PtrRef.h"
#include "Meta/Functor.h"
#include "Thread/CriticalSection.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
class TCachedMemoryPool : Meta::FNonCopyableNorMovable {
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
    using FCacheKey_ = THashMemoizer < TPtrRef<const _Key>, Meta::THash<_Key>, Meta::TDerefEqualTo<TPtrRef<const _Key>> > ; // memoize a ptr reference to the key in the block

    using pool_type = TTypedMemoryPool<FCacheItem_, _ChunkSize, _MaxChunks, EThreadBarrier::None/* got a global CS already */, _Allocator>;
    using cache_type = TFixedSizeHashMap<
        FCacheKey_,
        typename pool_type::index_type, // index of the block
        pool_type::MaxSize * 2
    >;
    STATIC_ASSERT(std::is_default_constructible_v<_Value>); // use ctor to initialize the object

public:
    using key_type = _Key;
    using value_type = _Value;
    using index_type = typename pool_type::index_type;
    using block_type = typename pool_type::value_type;

    STATIC_CONST_INTEGRAL(size_t, MaxSize, pool_type::MaxSize);

    TCachedMemoryPool() = default;

    explicit TCachedMemoryPool(_Allocator&& ralloc) : _pool(std::move(ralloc)) {}
    explicit TCachedMemoryPool(const _Allocator& alloc) : _pool(alloc) {}

    index_type NumCachedBlocks() const NOEXCEPT { return checked_cast<index_type>(_cache.size()); }
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

    bool CheckInvariants() const;

private:
    FCriticalSection _cacheCS;
    cache_type _cache;
    pool_type _pool;
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
template <typename _Ctor>
auto TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::FindOrAdd(key_type&& rkey, _Ctor&& ctor) -> TPair<index_type, bool> {
    const FCriticalScope scopeLock(&_cacheCS);

    const FCacheKey_ cacheKey{ MakePtrRef(static_cast<const _Key&>(rkey)) };
    const auto it = _cache.find(cacheKey);

    bool exist;
    index_type id;
    if (Likely(_cache.end() != it)) {
        exist = true;
        id = it->second;

        Meta::VariadicFunctor(ctor, _pool.At(id)->MutableValue(), id, true);
    }
    else {
        exist = false;
        id = _pool.Allocate();

        block_type* const pBlock = _pool.At(id);
        INPLACE_NEW(pBlock, block_type){ std::move(rkey) };

        if (not Meta::VariadicFunctor(ctor, pBlock->MutableValue(), id, false)) {
            Assert_NoAssume(not exist); // #TODO: too restrictive ?
            _pool.Deallocate(id);
            return MakePair(UMax, false);
        }

        _cache.Add_AssertUnique(
            FCacheKey_{ MakePtrRef(pBlock->Key()), cacheKey.Hash() },
            id );
    }

    Assert(id < pool_type::MaxSize);
    return MakePair(id, exist);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
bool TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::Remove(index_type id) {
    return RemoveIf(id, []() constexpr -> bool { return true; });
}
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
template <typename _Dtor>
bool TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::RemoveIf(index_type id, _Dtor&& dtor) {
    const FCriticalScope scopeLock(&_cacheCS);

    block_type* const pBlock = _pool.At(id);

    bool shouldRemove;
    IF_CONSTEXPR(FCacheItem_::has_separated_key)
        shouldRemove = Meta::VariadicFunctor(dtor, pBlock->MutableValue(), pBlock->Key(), id);
    else
        shouldRemove = Meta::VariadicFunctor(dtor, pBlock->MutableValue(), id);

    if (Unlikely(shouldRemove)) {
        _cache.erase(MakePtrRef(pBlock->Key()));

        Meta::Destroy(pBlock);

        _pool.Deallocate(id);

        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
void TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::Clear_AssertCompletelyEmpty() {
    FCriticalScope scopeLock(&_cacheCS);

    AssertRelease(_cache.empty());

    _cache.clear();
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
    FCriticalScope scopeLock(&_cacheCS);

    _cache.clear();

    _pool.Each([&dtor](block_type* pBlock, index_type id) {
        IF_CONSTEXPR(FCacheItem_::has_separated_key)
            Meta::VariadicFunctor(dtor, pBlock->MutableValue(), pBlock->Key(), id);
        else
            Meta::VariadicFunctor(dtor, pBlock->MutableValue(), id);
        Meta::Destroy(pBlock);
    });

    _pool.Clear_IgnoreLeaks_AssumePOD();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
bool TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::CheckInvariants() const {
#if USE_PPE_DEBUG
    FCriticalScope scopeLock(&_cacheCS);

    size_t cnt = 0;
    const_cast<pool_type&>(_pool).Each([this, &cnt](const block_type* pBlock, index_type id) {
        auto it = _cache.find(MakePtrRef(pBlock->Key()));
        AssertRelease(_cache.end() != it);
        AssertRelease(it->second == id);
        ++cnt;
    });

    Assert(_cache.size() == cnt);

    return _pool.CheckInvariants();
#else
    return true;
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
