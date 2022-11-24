// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Container/HashTable.h"

#include "HAL/PlatformMemory.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(FHashTableData_) == sizeof(void*)+sizeof(u64));
//----------------------------------------------------------------------------
auto FHashTableData_::SetState(size_t index, state_t state) -> state_t {
    Assert(index < CapacityM1 + 1);

    static_assert(
        kEmpty & kDeleted & kSentinel & 0x80,
        "Special markers need to have the MSB to make checking for them efficient");
    static_assert(kEmpty < kSentinel&& kDeleted < kSentinel,
        "kEmpty and kDeleted must be smaller than kSentinel to make the "
        "SIMD test of IsEmptyOrDeleted() efficient");
    static_assert(kSentinel == -1,
        "kSentinel must be -1 to elide loading it from memory into SIMD "
        "registers (pcmpeqd xmm, xmm)");
    static_assert(kEmpty == -128,
        "kEmpty must be -128 to make the SIMD check for its "
        "existence efficient (psignb xmm, xmm)");
    static_assert(~kEmpty & ~kDeleted & kSentinel & 0x7F,
        "kEmpty and kDeleted must share an unset bit that is not shared "
        "by kSentinel to make the scalar test for MatchEmptyOrDeleted() "
        "efficient");
    static_assert(kDeleted == -2,
        "kDeleted must be -2 to make the implementation of "
        "ConvertSpecialToEmptyAndFullToDeleted efficient");

    state_t* const states = static_cast<state_t*>(StatesAndBuckets);

    const state_t prev = states[index];
    states[index] = state;

    // the number of cloned control bytes that we copy from the beginning to the
    // end of the control bytes array.
    constexpr auto GClonedBytes = (GGroupSize - 1);
#if 0
    // mirror state ate for first group
    if (Unlikely(index < GClonedBytes)) {
        Assert_NoAssume(prev == states[Capacity + index]);
        states[Capacity + index] = state;
    }
#else
    // mirror without branching
    const auto mirror = ((index - GClonedBytes) & CapacityM1) + (GClonedBytes & CapacityM1);
    Assert_NoAssume(mirror == index || prev == states[mirror]);
    states[mirror] = state;
#endif

    return prev;
}
//----------------------------------------------------------------------------
void FHashTableData_::SetDeleted(size_t index) {
    // insert a kEmpty instead of kDeleted if there is no element after
    // => tries to keep the load factor as small as possible
    const size_t indexBefore = ((index - GGroupSize) & CapacityM1);
    const bitmask_t emptyBefore = MatchEmpty(GroupAt(indexBefore));
    const bitmask_t emptyAfter = MatchEmpty(GroupAt(index));

    // We count how many consecutive non empties we have to the right and to the
    // left of `it`. If the sum is >= kWidth then there is at least one probe
    // window that might have seen a full group.
    const bool wasNeverFull = (
        emptyBefore && emptyAfter && (
            emptyAfter.CountTrailingZeros() +
            emptyBefore.CountLeadingZeros() ) < GGroupSize );

    const EState state = (wasNeverFull ? kEmpty : kDeleted);

#if USE_PPE_ASSERT
    if (Unlikely(IsEmptyOrDeleted(SetState(index, state))))
        AssertNotReached(); // double delete
#else
    SetState(index, state);
#endif
}
//----------------------------------------------------------------------------
void FHashTableData_::ResetStates() {
    Assert(StatesAndBuckets);
    Assert(Meta::IsAlignedPow2(16, StatesAndBuckets));
    STATIC_ASSERT(GGroupSize == 16);

    const size_t numStates = NumStates();
    Assert(numStates);
    Assert(NumBuckets() >= GGroupSize);

    FPlatformMemory::Memset(StatesAndBuckets, static_cast<u8>(kEmpty), (numStates - 1) * sizeof(state_t));

    // used by iterators to stop looking for a new filled bucket
    static_cast<state_t*>(StatesAndBuckets)[numStates - 1] = kSentinel;
}
//----------------------------------------------------------------------------
size_t FHashTableData_::FirstFilledBucket_ReturnOffset(const state_t* states) {
    Assert(states);

    ::__m128i kSentinel_16 = ::_mm_set1_epi8(kSentinel);

    STATIC_ASSERT(sizeof(::__m128i) == GGroupSize);

    static constexpr uintptr_t GGroupMask = static_cast<uintptr_t>(GGroupSize - 1);
    static constexpr uintptr_t GGroupAlign = (~GGroupMask);

    const state_t* aligned = reinterpret_cast<state_t*>(reinterpret_cast<uintptr_t>(states) & GGroupAlign);

    bitmask_t visited{ bitmask_t::AllMask << (reinterpret_cast<size_t>(states) & GGroupMask)  }; // don't go back

    for (;;) {
        group_t group = ::_mm_load_si128(reinterpret_cast<const ::__m128i*>(aligned)); // benefits from aligned load in this version

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

        if (bitmask_t filled = (MatchFilledBucket(group) & visited)) {
            const state_t* item = (aligned + filled.PopFront_AssumeNotEmpty());
            return (item - states);
        }

        aligned += GGroupSize;
        visited = { bitmask_t::AllMask };
    }
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
