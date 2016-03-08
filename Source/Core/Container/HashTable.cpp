#include "stdafx.h"

#include "Core/Container/HashTable.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _HashWIndex>
NO_INLINE static Pair<size_t, bool> InsertBucketUsingProbe_(
    const details::HashTableProbe_ probe,
    const MemoryView<_HashWIndex> hashWIndices,
    size_t hashValue, size_t* pDataIndex,
    size_t hint ) {
    Assert(pDataIndex);

    typedef typename _HashWIndex::size_type size_type;
    Assert(probe.Size == *pDataIndex);

    _HashWIndex idx = _HashWIndex::Make(hashValue, *pDataIndex);

    size_t bucket = (size_t(-1) == hint)
        ? probe.DesiredPos(idx.hash_value)
        : probe.DesiredPos(hint+1);

    const size_t baseBucket = bucket;

    size_t distance = probe.ProbeDistance(size_type(hashValue), bucket);
    size_t it_distance = 0;
    for (;;) {
        Assert(distance <= probe.HashCapacity);

        _HashWIndex& it = hashWIndices[bucket];

        if (it.empty()) {
            hashWIndices[bucket] = idx;
            return MakePair(bucket, false);
        }
        else if (it.hash_value == hashValue) {
            bool existed = (it.hash_value == size_type(hashValue));
            *pDataIndex = it.data_index;
            return MakePair(bucket, existed);
        }
        else if ((it_distance = probe.ProbeDistance(it.hash_value, bucket)) < distance) {
            if (it.deleted()) {
                hashWIndices[bucket] = idx;
                return MakePair(bucket, false);
            }
            std::swap(it, idx);
            distance = it_distance;
        }

        distance++;
        bucket = probe.DesiredPos(bucket+1);

#ifdef WITH_CORE_ASSERT
        const size_t distanceForDebug = probe.ProbeDistance(idx.hash_value, bucket);
        Assert(distanceForDebug == distance);
#endif
    }

    AssertNotReached();
    return Pair<size_t, bool>();
}
//----------------------------------------------------------------------------
template <typename _HashWIndex>
NO_INLINE static Pair<size_t, bool> LookupBucketUsingProbe_(
    const details::HashTableProbe_ probe,
    const MemoryView<_HashWIndex> hashWIndices,
    size_t hashValue, size_t* pDataIndex,
    size_t hint ) noexcept {
    Assert(pDataIndex);

    typedef typename _HashWIndex::size_type size_type;

    size_t bucket = (size_t(-1) == hint)
        ? probe.DesiredPos(hashValue)
        : probe.DesiredPos(hint+1);

    size_t distance = probe.ProbeDistance(size_type(hashValue), bucket);
    for (;;) {
        Assert(distance <= probe.HashCapacity);

        const _HashWIndex& it = hashWIndices[bucket];

        if (it.empty() || distance > probe.ProbeDistance(it.hash_value, bucket) ) {
            break;
        }
        else if (it.hash_value == size_type(hashValue)) {
            *pDataIndex = it.data_index;
            return MakePair(bucket, true);
        }

        distance++;
        bucket = probe.DesiredPos(bucket+1);
    }

    return MakePair(bucket, false);
}
//----------------------------------------------------------------------------
template <typename _HashWIndex>
static void EraseBucketUsingProbe_(
    const details::HashTableProbe_ probe,
    const MemoryView<_HashWIndex> hashWIndices,
    size_t bucket, size_t hashValue
    ) {
    Assert(bucket < probe.HashCapacity);
    typedef typename _HashWIndex::size_type size_type;
    hashValue = size_type(hashValue);

    _HashWIndex& it = hashWIndices[bucket];
    Assert(not it.empty());
    Assert(not it.deleted());

    it.MarkAsDeleted();
}
//----------------------------------------------------------------------------
template <typename _HashWIndex>
static void SwapDataIndexUsingProbe_(
    const details::HashTableProbe_ probe,
    const MemoryView<_HashWIndex> hashWIndices,
    size_t bucket0, size_t bucket1
    ) {
    Assert(bucket0 < probe.HashCapacity);
    Assert(bucket1 < probe.HashCapacity);
    Assert(bucket0 != bucket1);
    Assert(hashWIndices[bucket0].data_index < probe.Size);
    Assert(hashWIndices[bucket1].data_index < probe.Size);

    std::swap(  hashWIndices[bucket0].data_index,
                hashWIndices[bucket1].data_index );
}
//----------------------------------------------------------------------------
template <typename _HashWIndex>
static void ClearBucketsUsingProbe_(
    const details::HashTableProbe_ probe,
    const MemoryView<_HashWIndex> hashWIndices ) {
    const _HashWIndex zero = _HashWIndex::Zero();
    Assert(zero.empty());
    for (_HashWIndex& idx : hashWIndices)
        idx = zero;
}
//----------------------------------------------------------------------------
template <typename _HashWIndex>
static HashTableStats ProbingStatsUsingProbe_(
    const details::HashTableProbe_ probe,
    const MemoryView<_HashWIndex>& hashWIndices ) {

    HashTableStats stats;
    stats.MinProbe = NumericLimits<size_t>::MaxValue();
    stats.MaxProbe = NumericLimits<size_t>::MinValue();
    stats.MeanProbe = 0;
    stats.DevProbe = 0;

    size_t count = 0;

    forrange(i, 0, probe.HashCapacity) {
        const _HashWIndex& idx = hashWIndices[i];
        if (not (idx.empty() || idx.deleted())) {
            const size_t d = probe.ProbeDistance(idx.hash_value, i);
            stats.MinProbe = std::min(d, stats.MinProbe);
            stats.MaxProbe = std::max(d, stats.MaxProbe);
            stats.MeanProbe += d;
            count++;
        }
    }

    Assert(probe.Size == count);

    stats.MeanProbe /= probe.Size;

    forrange(i, 0, probe.HashCapacity) {
        const _HashWIndex& idx = hashWIndices[i];
        if (not (idx.empty() || idx.deleted())) {
            const size_t d = probe.ProbeDistance(idx.hash_value, i);
            const double f = (d - stats.MeanProbe);
            stats.DevProbe = f*f;
        }
    }

    stats.DevProbe /= probe.Size;
    stats.DevProbe = std::sqrt(stats.DevProbe);

    return stats;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(u32) == sizeof(HashValueWIndex32_));
STATIC_ASSERT(sizeof(u64) == sizeof(HashValueWIndex64_));
//----------------------------------------------------------------------------
STATIC_ASSERT(std::is_pod<HashValueWIndex32_>::value);
STATIC_ASSERT(std::is_pod<HashValueWIndex64_>::value);
//----------------------------------------------------------------------------
Pair<size_t, bool> HashTableProbe_::InsertBucket(size_t hashValue, size_t* pDataIndex, size_t hint /* = size_t(-1) */) const {
    return (UseHashIndices64
        ? InsertBucketUsingProbe_(*this, HashIndices64(), hashValue, pDataIndex, hint)
        : InsertBucketUsingProbe_(*this, HashIndices32(), hashValue, pDataIndex, hint) );
}
//----------------------------------------------------------------------------
Pair<size_t, bool> HashTableProbe_::LookupBucket(size_t hashValue, size_t* pDataIndex, size_t hint /* = size_t(-1) */) const {
    return (UseHashIndices64
        ? LookupBucketUsingProbe_(*this, HashIndices64(), hashValue, pDataIndex, hint)
        : LookupBucketUsingProbe_(*this, HashIndices32(), hashValue, pDataIndex, hint) );
}
//----------------------------------------------------------------------------
void HashTableProbe_::EraseBucket(size_t bucket, size_t hashValue) const {
    (UseHashIndices64
        ? EraseBucketUsingProbe_(*this, HashIndices64(), bucket, hashValue)
        : EraseBucketUsingProbe_(*this, HashIndices32(), bucket, hashValue) );
}
//----------------------------------------------------------------------------
void HashTableProbe_::SwapDataIndex(size_t bucket0, size_t bucket1) const {
    (UseHashIndices64
        ? SwapDataIndexUsingProbe_(*this, HashIndices64(), bucket0, bucket1)
        : SwapDataIndexUsingProbe_(*this, HashIndices32(), bucket0, bucket1) );
}
//----------------------------------------------------------------------------
void HashTableProbe_::ClearBuckets() const {
    (UseHashIndices64
        ? ClearBucketsUsingProbe_(*this, HashIndices64())
        : ClearBucketsUsingProbe_(*this, HashIndices32()) );
}
//----------------------------------------------------------------------------
HashTableStats HashTableProbe_::ProbingStats() const {
    return (UseHashIndices64
        ? ProbingStatsUsingProbe_(*this, HashIndices64())
        : ProbingStatsUsingProbe_(*this, HashIndices32()) );
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
// checks template class compilation :
template class HashTable<int, float>;
template class HashTable<double, void>;
#endif
//----------------------------------------------------------------------------
template <typename T, size_t _Eq, size_t _Size = sizeof(T)>
struct CheckSize
{
    static constexpr bool value = (_Size == _Eq);
    STATIC_ASSERT(value);
};

STATIC_ASSERT(CheckSize<HashTableBase<int, float>, 2*sizeof(size_t)>::value);
STATIC_ASSERT(CheckSize<HashTable<int, float>, 2*sizeof(size_t)>::value);
STATIC_ASSERT(CheckSize<HashTable<double, void>, 2*sizeof(size_t)>::value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core