#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"
#include "HAL/PlatformMemory.h"
#include "Meta/AlignedStorage.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// In-situ storage for only 1 allocation, always using full capacity
//----------------------------------------------------------------------------
template <size_t _SizeInBytes>
class TInSituAllocator : private FGenericAllocator {
public:
    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap = std::false_type;

    using is_always_equal = std::false_type;

    using has_maxsize = std::true_type;
    using has_owns = std::true_type;
    using has_reallocate = std::false_type;
    using has_acquire = std::false_type;
    using has_steal = std::false_type;

    STATIC_CONST_INTEGRAL(size_t, Alignment, ALLOCATION_BOUNDARY);
    STATIC_CONST_INTEGRAL(size_t, SizeInBytes, _SizeInBytes);

    using insitu_t = ALIGNED_STORAGE(SizeInBytes, Alignment);

    insitu_t InSitu;

#if USE_PPE_ASSERT
    enum class EState : size_t { // state + canary
        Allocated   = CODE3264(0x0CC491EDul, 0x0CC491ED0CC491EDull),
        Freed       = CODE3264(0xFA11BAC4ul, 0xFA11BAC4FA11BAC4ull),
    };

    EState State = EState::Freed;

    ~TInSituAllocator() {
        Assert_NoAssume(EState::Freed == State);
    }
#endif

    TInSituAllocator() = default;

    // copy checks that both allocators are empty (can't copy in situ allocations)
    TInSituAllocator(const TInSituAllocator& other) { operator =(other); }
    TInSituAllocator& operator =(const TInSituAllocator& other) {
        Assert_NoAssume(EState::Freed == State);
        Assert_NoAssume(EState::Freed == other.State);
        return (*this);
    }

    // move checks that both allocators are empty (can't move in situ allocations)
    TInSituAllocator(TInSituAllocator&& rvalue) { operator =(std::move(rvalue)); }
    TInSituAllocator& operator =(TInSituAllocator&& rvalue) {
        Assert_NoAssume(EState::Freed == State);
        Assert_NoAssume(EState::Freed == rvalue.State);
        return (*this);
    }

    static size_t MaxSize() NOEXCEPT {
        return SizeInBytes;
    }

    static size_t SnapSize(size_t s) NOEXCEPT {
        Assert(s <= SizeInBytes);
        return SizeInBytes;
    }

    bool Owns(FAllocatorBlock b) const NOEXCEPT {
#if USE_PPE_ASSERT
        if (b.Data == &InSitu) {
            Assert(EState::Allocated == State);
            Assert(b.SizeInBytes == SizeInBytes);
            return true;
        }
        else {
            Assert(not FPlatformMemory::Memoverlap(&InSitu, SizeInBytes, b.Data, b.SizeInBytes));
            return false;
        }
#else
        return (b.Data == &InSitu);
#endif
    }

    FAllocatorBlock Allocate(size_t s) NOEXCEPT {
#if USE_PPE_ASSERT
        Assert(s);
        Assert(s == SizeInBytes);
        Assert(EState::Freed == State);

        State = EState::Allocated;
#else
        UNUSED(s);
#endif
        return FAllocatorBlock{ &InSitu, SizeInBytes };
    }

    void Deallocate(FAllocatorBlock b) NOEXCEPT {
#if USE_PPE_ASSERT
        Assert(EState::Allocated == State);
        Assert(b.Data == &InSitu);
        Assert(b.SizeInBytes == SizeInBytes);

        State = EState::Freed;
#else
        UNUSED(b);
#endif
    }

    friend bool operator ==(const TInSituAllocator& lhs, const TInSituAllocator& rhs) NOEXCEPT {
        return (static_cast<const void*>(&lhs.InSitu) == static_cast<const void*>(&rhs.InSitu));
    }

    friend bool operator !=(const TInSituAllocator& lhs, const TInSituAllocator& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
