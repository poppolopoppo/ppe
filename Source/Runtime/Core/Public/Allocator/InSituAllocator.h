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

    STATIC_CONST_INTEGRAL(size_t, Alignment, alignof(void*));
    STATIC_CONST_INTEGRAL(size_t, SizeInBytes, _SizeInBytes);

    using insitu_t = ALIGNED_STORAGE(SizeInBytes, Alignment);

    insitu_t InSitu;

#if USE_PPE_ASSERT
    enum class EState : size_t { // state + canary
        Allocated   = CODE3264(0x0CC491EDul, 0x0CC491ED0CC491EDull),
        Freed       = CODE3264(0xFA11BAC4ul, 0xFA11BAC4FA11BAC4ull),
    };

    EState State = EState::Freed;

    FORCE_INLINE ~TInSituAllocator() {
        Assert_NoAssume(EState::Freed == State);
    }
#endif

    TInSituAllocator() NOEXCEPT = default;

    // copy checks that both allocators are empty (can't copy in situ allocations)
    TInSituAllocator(const TInSituAllocator& other) NOEXCEPT { operator =(other); }
    TInSituAllocator& operator =(const TInSituAllocator& other) NOEXCEPT {
        Assert_NoAssume(EState::Freed == State);
        Assert_NoAssume(EState::Freed == other.State);
        UNUSED(other);
        return (*this);
    }

    // move checks that both allocators are empty (can't move in situ allocations)
    TInSituAllocator(TInSituAllocator&& rvalue) NOEXCEPT { operator =(std::move(rvalue)); }
    TInSituAllocator& operator =(TInSituAllocator&& rvalue) NOEXCEPT {
        Assert_NoAssume(EState::Freed == State);
        Assert_NoAssume(EState::Freed == rvalue.State);
        UNUSED(rvalue);
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
        if (b.Data == std::addressof(InSitu)) {
            Assert(EState::Allocated == State);
            Assert(b.SizeInBytes == SizeInBytes);
            return true;
        }
        else {
            Assert(not FPlatformMemory::Memoverlap(std::addressof(InSitu), SizeInBytes, b.Data, b.SizeInBytes));
            return false;
        }
#else
        return (b.Data == std::addressof(InSitu));
#endif
    }

    FORCE_INLINE FAllocatorBlock Allocate(size_t s) NOEXCEPT {
#if USE_PPE_ASSERT
        Assert(s);
        Assert(s == SizeInBytes);
        Assert(EState::Freed == State);
        Assert(Meta::IsAlignedPow2(Alignment, std::addressof(InSitu)));

        State = EState::Allocated;
#else
        UNUSED(s);
#endif
        return FAllocatorBlock{ std::addressof(InSitu), SizeInBytes };
    }

    FORCE_INLINE void Deallocate(FAllocatorBlock b) NOEXCEPT {
#if USE_PPE_ASSERT
        Assert(EState::Allocated == State);
        Assert(b.Data == std::addressof(InSitu));
        Assert(b.SizeInBytes == SizeInBytes);

        State = EState::Freed;
        FPlatformMemory::Memdeadbeef(b.Data, b.SizeInBytes);
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
// Uses an in-situ storage like a stack, can't reclaim memory if not FIFO
//----------------------------------------------------------------------------
template <size_t _SizeInBytes>
class TInSituStackAllocator  : private FGenericAllocator {
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
    size_t Offset{ 0 };

    TInSituStackAllocator() NOEXCEPT = default;

    // copy checks that both allocators are empty (can't copy in situ allocations)
    TInSituStackAllocator(const TInSituStackAllocator& other) NOEXCEPT { operator =(other); }
    TInSituStackAllocator& operator =(const TInSituStackAllocator& other) NOEXCEPT {
        Assert_NoAssume(0 == Offset);
        Assert_NoAssume(0 == other.Offset);
        UNUSED(other);
        return (*this);
    }

    // move checks that both allocators are empty (can't move in situ allocations)
    TInSituStackAllocator(TInSituStackAllocator&& rvalue) NOEXCEPT { operator =(std::move(rvalue)); }
    TInSituStackAllocator& operator =(TInSituStackAllocator&& rvalue) NOEXCEPT {
        Assert_NoAssume(0 == Offset);
        Assert_NoAssume(0 == rvalue.Offset);
        UNUSED(rvalue);
        return (*this);
    }

    size_t MaxSize() const NOEXCEPT {
        return SizeInBytes;
    }

    size_t SnapSize(size_t s) const NOEXCEPT {
        Assert(s <= SizeInBytes);
        return Meta::RoundToNextPow2(s, Alignment);
    }

    bool Owns(FAllocatorBlock b) const NOEXCEPT {
#if USE_PPE_ASSERT
        if (FPlatformMemory::Memoverlap(b.Data, b.SizeInBytes, std::addressof(InSitu), _SizeInBytes)) {
            Assert_NoAssume((u8*)b.Data - (u8*)std::addressof(InSitu) < ptrdiff_t(Offset));
            return true;
        }
        else {
            return false;
        }
#else
        return ((u8*)b.Data >= (u8*)std::addressof(InSitu) && (u8*)std::addressof(InSitu) + Offset >= (u8*)b.Data);
#endif
    }

    FAllocatorBlock Allocate(size_t s) NOEXCEPT {
        FAllocatorBlock b;
        b.SizeInBytes = Meta::RoundToNextPow2(s, Alignment);

        if (Offset + b.SizeInBytes <= _SizeInBytes) {
            b.Data = (u8*)std::addressof(InSitu) + Offset;
            Offset += b.SizeInBytes;
        }
        else {
            b.Data = nullptr;
            b.SizeInBytes = 0; // not enough space, fail allocation
        }

        return b;
    }

    void Deallocate(FAllocatorBlock b) NOEXCEPT {
        Assert_NoAssume(Owns(b));

        // can reclaim only the last block allocated !
        const size_t off = checked_cast<size_t>((u8*)b.Data - (u8*)std::addressof(InSitu));
        if (off + b.SizeInBytes == Offset)
            Offset -= b.SizeInBytes;

        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(b.Data, b.SizeInBytes));
    }

    friend bool operator ==(const TInSituStackAllocator& lhs, const TInSituStackAllocator& rhs) NOEXCEPT {
        return (static_cast<const void*>(&lhs.InSitu) == static_cast<const void*>(&rhs.InSitu));
    }

    friend bool operator !=(const TInSituStackAllocator& lhs, const TInSituStackAllocator& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) PPE::TInSituAllocator<CODE3264(48,96)>;
