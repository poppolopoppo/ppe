#include "stdafx.h"

#include "HashTable.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(FHashTableData_) == sizeof(void*)+sizeof(u64));
STATIC_ASSERT((FHashTableData_::kEmpty & FHashTableData_::kDeleted) == FHashTableData_::kDeleted);
STATIC_ASSERT((FHashTableData_::kSentinel & FHashTableData_::kDeleted) == FHashTableData_::kDeleted);
//----------------------------------------------------------------------------
auto FHashTableData_::SetState(size_t index, state_t state) -> state_t {
    Assert(index < Capacity);

    state_t* const states = (state_t*)StatesAndBuckets;

    const state_t prev = states[index];
    states[index] = state;

    // mirror state ate for first group
    constexpr size_t GGroupSizeM1 = (GGroupSize - 1);
    if (index < GGroupSizeM1) {
        Assert(prev == states[Capacity + index]);
        states[Capacity + index] = state;
    }

    return prev;
}
//----------------------------------------------------------------------------
void FHashTableData_::SetDeleted(size_t index) {
    // insert a kEmpty instead of kDeleted if there is no element after
    // => tries to keep the load factor as small as possible
    const EState state = (((state_t*)StatesAndBuckets)[index + 1] == kEmpty ? kEmpty : kDeleted);

#ifdef WITH_CORE_ASSERT
    if (SetState(index, state) & kDeleted)
        AssertNotReached(); // double delete
#else
    SetState(index, state);
#endif
}
//----------------------------------------------------------------------------
void FHashTableData_::ResetStates() {
    Assert(StatesAndBuckets);
    Assert(Meta::IsAligned(16, StatesAndBuckets));
    STATIC_ASSERT(GGroupSize == 16);

    const size_t numStates = NumStates();
    Assert(numStates);
    Assert(NumBuckets() >= GGroupSize);

    ::memset(StatesAndBuckets, kEmpty, (numStates - 1) * sizeof(state_t));

    // used by iterators to stop looking for a new filled bucket
    ((state_t*)StatesAndBuckets)[numStates - 1] = kSentinel;
}
//----------------------------------------------------------------------------
size_t FHashTableData_::FirstFilledBucket_ReturnOffset(const state_t* states) {
    Assert(states);

    __m128i kSentinel_16 = _mm_set1_epi8(kSentinel);

    constexpr size_t GTwoGroupsSize = (GGroupSize * 2);
    for (size_t bucket = 0; ; bucket += GTwoGroupsSize) {
        group_t group0 = _mm_lddqu_si128((const __m128i*)(states + bucket));
        group_t group1 = _mm_lddqu_si128((const __m128i*)(states + bucket + GGroupSize));

        const FBitMask filled = (MatchFilledBucket(group0) | (MatchFilledBucket(group1) << GGroupSize));
        const FBitMask sentinel = (Match(group0, kSentinel_16) | (Match(group1, kSentinel_16) << GGroupSize));

        if (filled) {
            const size_t itm = (bucket + filled.FirstBitSet_AssumeNotEmpty());
            if (Unlikely(sentinel)) {
                const size_t snl = bucket + sentinel.FirstBitSet_AssumeNotEmpty();
                Assert(snl >= GGroupSize - 1);
                const size_t end = (snl - (GGroupSize - 1));
                return (itm < end ? itm : end);
            }
            return itm;
        }

        if (Unlikely(sentinel)) {
            const size_t snl = bucket + sentinel.FirstBitSet_AssumeNotEmpty();
            Assert(snl >= GGroupSize - 1);
            const size_t end = (snl - (GGroupSize - 1));
            return end;
        }
    }

    AssertNotReached();
    return INDEX_NONE;
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
