#pragma once

#include "Core_fwd.h"

#include "Allocator/Allocation.h"
#include "Container/Pair.h"
#include "Thread/AtomicSpinLock.h"

#define CONCURRENT_HASHMAP(_DOMAIN, _KEY, _VALUE) \
    ::PPE::TConcurrentQueue<_KEY, _VALUE, ::PPE::Meta::THash<_KEY>, ::PPE::Meta::TEqualTo<_KEY>, ALLOCATOR(_DOMAIN)>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Simple concurrent hash-map with a linked-list per bucket.
// - Support deletion of keys, which is why it is not atomic
// - Support iteration with both shared or exclusive locking, but with a functor instead of iterators (which would be messy)
// - This implementation is not lock-free, but relies on a fine grained lock : we use an atomic bit set for
//   either shared or exclusive locking on a bucket level thanks to FAtomicMaskLock.
// - Each node uses its own allocation, but the allocator can be override (could use a slab or a pool easily).
// - The number of buckets is fixed by constructor parameter, and buckets should be allocated outside of this container.
//----------------------------------------------------------------------------
template <
    typename _Key, typename _Value,
    typename _Hash = Meta::THash<_Key>,
    typename _EqualTo = Meta::TEqualTo<_Key>,
    typename _Allocator = ALLOCATOR(Container) >
class TConcurrentHashMap : Meta::FNonCopyableNorMovable, _Hash, _EqualTo, _Allocator {
public:
    using key_type = _Key;
    using mapped_type = _Key;
    using hash_type = _Hash;
    using equalto_type = _EqualTo;
    using allocator_type = _Allocator;

    using value_type = TPair<_Key, _Value>;
    using public_type = TPair<const _Key, _Value>;
    using allocator_traits = TAllocatorTraits<_Allocator>;

    struct FEntry {
        value_type Value;
        FEntry* NextEntry{ nullptr };
    };

    explicit TConcurrentHashMap(TMemoryView<FEntry*> buckets) { InitBuckets_(buckets); }

    TConcurrentHashMap(TMemoryView<FEntry*> buckets, allocator_type&& ralloc) NOEXCEPT : _Allocator(std::move(ralloc)) { InitBuckets_(buckets); }
    TConcurrentHashMap(TMemoryView<FEntry*> buckets, const allocator_type& alloc) : _Allocator(alloc) { InitBuckets_(buckets); }

    TConcurrentHashMap(TMemoryView<FEntry*> buckets, hash_type&& rhash, equalto_type&& requalto) NOEXCEPT : _Hash(std::move(rhash)), _EqualTo(std::move(requalto)) { InitBuckets_(buckets); }
    TConcurrentHashMap(TMemoryView<FEntry> buckets, const hash_type& hash, const equalto_type& equalto) : _Hash(hash), _EqualTo(equalto) { InitBuckets_(buckets); }

    TConcurrentHashMap(TMemoryView<FEntry*> buckets, hash_type&& rhash, equalto_type&& requalto, allocator_type&& ralloc) NOEXCEPT : _Hash(std::move(rhash)), _EqualTo(std::move(requalto)), _Allocator(std::move(ralloc)) { InitBuckets_(buckets); }
    TConcurrentHashMap(TMemoryView<FEntry*> buckets, const hash_type& hash, const equalto_type& equalto, const allocator_type& alloc) : _Hash(hash), _EqualTo(equalto), _Allocator(alloc) { InitBuckets_(buckets); }

    ~TConcurrentHashMap() {
        Clear_ReleaseMemory();
    }

    bool empty() const { return (_size == 0); }
    size_t size() const { return _size.load(std::memory_order_relaxed); }

    TMemoryView<FEntry* const> Buckets() { return _buckets; }

    template <typename _KeyLike>
    const public_type* Lookup(const _KeyLike& key) const {
        return const_cast<TConcurrentHashMap*>(this)->Lookup(key);
    }
    template <typename _KeyLike>
    public_type* Lookup(const _KeyLike& key) {
        const size_t bucket = BucketFromKey_(key);
        const FAtomicMaskLock::FScopeLock scopeLock(_barrier, LockSubset_(bucket));

        for (FEntry* pEntry = _buckets[bucket]; pEntry; pEntry = pEntry->NextEntry) {
            if (pEntry->Value.first == key)
                return reinterpret_cast<public_type*>(&pEntry->Value);
        }

        return nullptr;
    }

    template <typename _KeyLike>
    public_type& FindOrAdd(const _KeyLike& key, bool *pAdded = nullptr) {
        const size_t bucket = BucketFromKey_(key);
        const FAtomicMaskLock::FScopeLock scopeLock(_barrier, LockSubset_(bucket));

        for (FEntry* pEntry = _buckets[bucket]; pEntry; pEntry = pEntry->NextEntry) {
            if (pEntry->Value.first == key) {
                if (pAdded) *pAdded = false;
                return *reinterpret_cast<public_type*>(&pEntry->Value);
            }
        }

        FEntry* const pNewEntry = INPLACE_NEW(allocator_traits::AllocateOneT<FEntry>(*this), FEntry);
        pNewEntry->Value.first = key_type{key};
        pNewEntry->NextEntry = _buckets[bucket];

        _buckets[bucket] = pNewEntry;
        _size.fetch_add(1);

        if (pAdded) *pAdded = true;
        return *reinterpret_cast<public_type*>(&pNewEntry->Value);
    }

    template <typename _KeyLike>
    bool Erase(const _KeyLike& key) {
        const size_t bucket = BucketFromKey_(key);
        const FAtomicMaskLock::FScopeLock scopeLock(_barrier, LockSubset_(bucket));

        FEntry** ppPrev = &_buckets[bucket];
        for (FEntry* pEntry = _buckets[bucket]; pEntry; pEntry = pEntry->NextEntry) {
            if (pEntry->Value.first == key) {
                (*ppPrev) = pEntry->NextEntry;
                _size.fetch_sub(1);

                Meta::Destroy(pEntry);
                allocator_traits::DeallocateOneT(*this, pEntry);
                return true;
            }
            ppPrev = &pEntry->NextEntry;
        }

        return false;
    }

    template <typename TFunctor>
    void Foreach(TFunctor each) const {
        forrange(bucket, 0, _buckets.size()) {
            const FAtomicMaskLock::FScopeLock scopeLock(_barrier, LockSubset_(bucket));
            for (const FEntry* pEntry = _buckets[bucket]; pEntry; pEntry = pEntry->NextEntry) {
                each(*reinterpret_cast<const public_type*>(&pEntry->Value));
            }
        }
    }

    void Clear() {
        const FAtomicMaskLock::FScopeLock scopeLock(_barrier, FAtomicMaskLock::AllMask);

        Clear_AssumeLocked_();
    }

    TMemoryView<FEntry*> Clear_ReleaseMemory() {
        const FAtomicMaskLock::FScopeLock scopeLock(_barrier, FAtomicMaskLock::AllMask);

        Clear_AssumeLocked_();

        TMemoryView<FEntry*> result = _buckets;
        _buckets = {}; // reset reference bucket's memory view
        return result;
    }

private:
    void InitBuckets_(TMemoryView<FEntry*> buckets) NOEXCEPT {
        _buckets = buckets;
        Broadcast(_buckets, nullptr);
    }
    size_t BucketFromKey_(const key_type& key) const NOEXCEPT {
        return (static_cast<const hash_type&>(*this)(key) % _buckets.size());
    }
    static FAtomicMaskLock::size_type LockSubset_(size_t bucket) NOEXCEPT {
        // 64 locks available on 64 bits, lock slot is determined by hashing the bucket index
        return (FAtomicMaskLock::size_type(1) << (hash_size_t_constexpr(bucket) % FAtomicMaskLock::NumBuckets));
    }

    void Clear_AssumeLocked_() {
        _size.store(0, std::memory_order_relaxed);

        for (FEntry*& pHead : _buckets) {
            for (FEntry* pEntry = pHead; pEntry; ) {
                FEntry* const pNext = pEntry->NextEntry;

                Meta::Destroy(pEntry);
                allocator_traits::DeallocateOneT(*this, pEntry);

                pEntry = pNext;
            }
            pHead = nullptr;
        }
    }

    mutable FAtomicMaskLock _barrier;
    TMemoryView<FEntry*> _buckets;
    std::atomic_size_t _size;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE