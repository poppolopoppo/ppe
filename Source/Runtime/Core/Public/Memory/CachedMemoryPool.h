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
        explicit TCacheItem_(const K& key) : _key(key) {}
        const K& key() const { return _key; }
        V& Value() { return _value; }
        const V& Value() const { return _value; }
        V* MutableValue() const { return &_value; }
    };

    template <typename T>
    class TCacheItem_<T, T> { // immutable when _Key == _Value
        const T _item;
    public:
        explicit TCacheItem_(const T& item) : _item(item) {}
        const T& Key() const { return _item; }
        const T& Value() const { return _item; }
        T* MutableValue() const { return const_cast<T*>(&_item); }
    };

    using FCacheItem_ = TCacheItem_<_Key, _Value>;
    using FCacheKey_ = THashMemoizer<TPtrRef<const _Key>, Meta::THash<_Key>>; // memoize a ptr reference to the key in the block

    using pool_type = TTypedMemoryPool<FCacheItem_, _ChunkSize, _MaxChunks, false/* got a global CS already */, _Allocator>;
    using cache_type = TFixedSizeHashMap<
        FCacheKey_,
        typename pool_type::index_type, // index of the block
        pool_type::MaxSize
    >;
    STATIC_ASSERT(std::is_default_constructible_v<_Value>); // use ctor to initialize the object

public:
    using key_type = _Key;
    using value_type = _Value;
    using index_type = typename pool_type::index_type;
    using block_type = typename pool_type::value_type;

    STATIC_CONST_INTEGRAL(size_t, MaxSize, pool_type::MaxSize);

    TCachedMemoryPool() = default;

    TCachedMemoryPool(_Allocator&& ralloc) : _pool(std::move(ralloc)) {}
    TCachedMemoryPool(const _Allocator& alloc) : _pool(alloc) {}

    const value_type* At(index_type id) const NOEXCEPT {
        return (&_pool->At(id)->Value());
    }
    auto* operator [](index_type id) const NOEXCEPT {
        block_type* const pblock = _pool[id];
        return (pblock ? &pblock->Value() : nullptr);
    }

    const key_type& Key(index_type id) const NOEXCEPT { return _pool.At(id)->Key(); }
    value_type& Value(index_type id) const NOEXCEPT { return _pool.At(id)->Value(); }

    template <typename _Ctor>
    TPair<index_type, bool> FindOrAdd(const key_type& rkey, _Ctor&& ctor);
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
auto TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::FindOrAdd(const key_type& key, _Ctor&& ctor) -> TPair<index_type, bool> {
    const FCriticalScope scopeLock(&_cacheCS);

    const auto it = _cache.find(MakePtrRef(key));

    bool exist = true;
    index_type id;
    if (Likely(_cache.end() != it)) {
        id = it->second;
    }
    else {
        exist = false;
        id = _pool.Allocate();

        block_type* const pblock = _pool.At(id);
        INPLACE_NEW(pblock, block_type){ key };

        _cache.Add_AssertUnique(MakePtrRef(pblock->Key()), id);
    }

    Assert(id < pool_type::MaxSize);
    block_type* const pblock = _pool.At(id);
    Meta::VariadicFunctor(ctor, &pblock->Value(), id, exist);

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

    block_type* const pblock = _pool.At(id);

    if (Unlikely(Meta::VariadicFunctor(dtor, pblock->MutableValue(), pblock->Key(), id))) {
        _cache.erase(MakePtrRef(pblock->Key()));

        Meta::Destroy(pblock);

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
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
template <typename _Dtor>
void TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::Clear_IgnoreLeaks(_Dtor&& dtor) {
    FCriticalScope scopeLock(&_cacheCS);

    _cache.clear();

    _pool.Each([&dtor](block_type* pblock, index_type id) {
        Meta::VariadicFunctor(dtor, &pblock->value_for_detor(), pblock->Key(), id);
        Meta::Destroy(pblock);
    });

    _pool.Clear_IgnoreLeaks_AssumePOD();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
bool TCachedMemoryPool<_Key, _Value, _ChunkSize, _MaxChunks, _Allocator>::CheckInvariants() const {
#if USE_PPE_DEBUG
    FCriticalScope scopeLock(&_cacheCS);

    size_t cnt = 0;
    const_cast<pool_type&>(_pool).Each([this, &cnt](const block_type* pblock, index_type id) {
        auto it = _cache.find(MakePtrRef(pblock->Key()));
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
