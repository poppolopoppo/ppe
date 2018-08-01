#include "stdafx.h"

#include "HashTable.h"

#include "HAL/PlatformMemory.h"

namespace PPE {
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
    if (Unlikely(index < GGroupSizeM1)) {
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

#ifdef WITH_PPE_ASSERT
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

    FPlatformMemory::Memset(StatesAndBuckets, u8(kEmpty), (numStates - 1) * sizeof(state_t));

    // used by iterators to stop looking for a new filled bucket
    ((state_t*)StatesAndBuckets)[numStates - 1] = kSentinel;
}
//----------------------------------------------------------------------------
size_t FHashTableData_::FirstFilledBucket_ReturnOffset(const state_t* states) {
    Assert(states);

    __m128i kSentinel_16 = _mm_set1_epi8(kSentinel);

    STATIC_ASSERT(sizeof(__m128i) == GGroupSize);

    static constexpr uintptr_t GGroupMask = uintptr_t(GGroupSize - 1);
    static constexpr uintptr_t GGroupAlign = (~GGroupMask);

    const state_t* aligned = (state_t*)(uintptr_t(states) & GGroupAlign);

    FBitMask visited{ size_t(0xFFFFu) << (size_t(states) & GGroupMask)  }; // don't go back

    for (;;) {
        group_t group = _mm_load_si128((const __m128i*)aligned); // benefits from aligned load in this version

#if 1
        if (Unlikely(Match(group, kSentinel_16))) {
            Assert(Match(group, kSentinel_16).FirstBitSet_AssumeNotEmpty() == GGroupMask); // always ATE
            return (aligned - states);
        }
#else
        // TODO: profile with scalar test instead of SSE for sentinel
        if (Unlikely(kSentinel == aligned[15]))
            return (aligned - states);
#endif

        if (FBitMask filled = (MatchFilledBucket(group) & visited)) {
            const state_t* item = (aligned + filled.PopFront_AssumeNotEmpty());
            return (item - states);
        }

        aligned += GGroupSize;
        visited = { 0xFFFFu };
    }
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
