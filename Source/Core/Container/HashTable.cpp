#include "stdafx.h"

#include "Core/Container/HashTable.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _HashWIndex>
static void EraseBucketUsingProbe_(
    const details::FHashTableProbe_ probe,
    const TMemoryView<_HashWIndex> hashWIndices,
    size_t bucket, size_t hashValue
    ) {
    UNUSED(probe);
    Assert(bucket < probe.HashCapacity);
    typedef typename _HashWIndex::size_type size_type;
    hashValue = size_type(hashValue);

    _HashWIndex& it = hashWIndices[bucket];
    Assert(not it.empty());
    Assert(not it.deleted());

    it.MarkAsDeleted();

    AssertNotImplemented(); // TODO: bubble down instead of tombstones !
}
//----------------------------------------------------------------------------
template <typename _HashWIndex>
static void SwapDataIndexUsingProbe_(
    const details::FHashTableProbe_ probe,
    const TMemoryView<_HashWIndex> hashWIndices,
    size_t bucket0, size_t bucket1
    ) {
    UNUSED(probe);
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
    const details::FHashTableProbe_ probe,
    const TMemoryView<_HashWIndex> hashWIndices ) {
    UNUSED(probe);
    Assert(hashWIndices.size() == probe.HashCapacity);
    const _HashWIndex zero = _HashWIndex::Zero();
    Assert(zero.empty());
    for (_HashWIndex& idx : hashWIndices)
        idx = zero;
}
//----------------------------------------------------------------------------
template <typename _HashWIndex>
static FHashTableStats ProbingStatsUsingProbe_(
    const details::FHashTableProbe_ probe,
    const TMemoryView<_HashWIndex>& hashWIndices ) {

    FHashTableStats stats;
    stats.MinProbe = TNumericLimits<size_t>::MaxValue();
    stats.MaxProbe = TNumericLimits<size_t>::MinValue();
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
STATIC_ASSERT(sizeof(u32) == sizeof(FHashValueWIndex32_));
STATIC_ASSERT(sizeof(u64) == sizeof(FHashValueWIndex64_));
//----------------------------------------------------------------------------
STATIC_ASSERT(std::is_pod<FHashValueWIndex32_>::value);
STATIC_ASSERT(std::is_pod<FHashValueWIndex64_>::value);
//----------------------------------------------------------------------------
void FHashTableProbe_::EraseBucket(size_t bucket, size_t hashValue) const {
    (UseHashIndices64
        ? EraseBucketUsingProbe_(*this, HashIndices64(), bucket, hashValue)
        : EraseBucketUsingProbe_(*this, HashIndices32(), bucket, hashValue) );
}
//----------------------------------------------------------------------------
void FHashTableProbe_::SwapDataIndex(size_t bucket0, size_t bucket1) const {
    (UseHashIndices64
        ? SwapDataIndexUsingProbe_(*this, HashIndices64(), bucket0, bucket1)
        : SwapDataIndexUsingProbe_(*this, HashIndices32(), bucket0, bucket1) );
}
//----------------------------------------------------------------------------
void FHashTableProbe_::ClearBuckets() const {
    (UseHashIndices64
        ? ClearBucketsUsingProbe_(*this, HashIndices64())
        : ClearBucketsUsingProbe_(*this, HashIndices32()) );
}
//----------------------------------------------------------------------------
FHashTableStats FHashTableProbe_::ProbingStats() const {
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
template class THashTable<int, float>;
template class THashTable<double, void>;
#endif
//----------------------------------------------------------------------------
template <typename T, size_t _Eq, size_t _Size = sizeof(T)>
struct TCheckSize
{
    static constexpr bool value = (_Size == _Eq);
    STATIC_ASSERT(value);
};

STATIC_ASSERT(TCheckSize<THashTableBase<int, float>, 2*sizeof(size_t)>::value);
STATIC_ASSERT(TCheckSize<THashTable<int, float>, 2*sizeof(size_t)>::value);
STATIC_ASSERT(TCheckSize<THashTable<double, void>, 2*sizeof(size_t)>::value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
