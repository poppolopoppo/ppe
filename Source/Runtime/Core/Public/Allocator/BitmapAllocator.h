#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"
#include "Container/BitMask.h"
#include "HAL/PlatformMemory.h"
#include "Meta/AlignedStorage.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Using a bit mask and an in-situ buffer
//----------------------------------------------------------------------------
template <size_t _BlockSize>
class TBitmapAllocator : private FGenericAllocator {
    STATIC_ASSERT(Meta::IsAligned(sizeof(intptr_t), _BlockSize));
public:
    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap = std::false_type;

    using is_always_equal = std::false_type;

    using has_maxsize = std::true_type;
    using has_owns = std::true_type;
    using has_reallocate = std::true_type;
    using has_acquire = std::false_type;
    using has_steal = std::false_type;

    STATIC_CONST_INTEGRAL(size_t, Alignment, Min(_BlockSize, size_t(ALLOCATION_BOUNDARY)));
    STATIC_CONST_INTEGRAL(size_t, BlockSize, _BlockSize);

    using bitmask_t = TBitMask<uintptr_t>;
    using block_t = ALIGNED_STORAGE(BlockSize, Alignment);

    STATIC_CONST_INTEGRAL(size_t, BlockCount, bitmask_t::BitCount);
    STATIC_CONST_INTEGRAL(size_t, CapacityInBytes, BlockSize * BlockCount);

    block_t Blocks[BlockCount];
    bitmask_t Mask{ bitmask_t::AllMask };

#if USE_PPE_ASSERT
    ~TBitmapAllocator() {
        STATIC_ASSERT(Meta::IsAligned(Alignment, _BlockSize));
        Assert_NoAssume(bitmask_t::AllMask == Mask);
    }
#endif

    TBitmapAllocator() = default;

    // copy checks that both src and dst allocators are empty (can't copy insitu allocations)
    TBitmapAllocator(const TBitmapAllocator& other) { operator =(other); }
    TBitmapAllocator& operator =(const TBitmapAllocator& other) {
        Assert(Mask == bitmask_t::AllMask);
        Assert(other.Mask == bitmask_t::AllMask);
        return (*this);
    }

    // move checks that both src and dst allocators are empty (can't move insitu allocations)
    TBitmapAllocator(TBitmapAllocator&& rvalue) { operator =(std::move(rvalue)); }
    TBitmapAllocator& operator =(TBitmapAllocator&& rvalue) {
        Assert(Mask == bitmask_t::AllMask);
        Assert(rvalue.Mask == bitmask_t::AllMask);
        return (*this);
    }

    static size_t MaxSize() NOEXCEPT {
        return CapacityInBytes;
    }

    static size_t SnapSize(size_t s) NOEXCEPT {
        return Meta::RoundToNext(s, BlockSize);
    }

    bool Owns(FAllocatorBlock b) const NOEXCEPT {
        return (((u8*)b.Data >= (u8*)&Blocks[0]) &
                ((u8*)b.Data + b.SizeInBytes <= (u8*)&Blocks[BlockCount]));
    }

    FAllocatorBlock Allocate(size_t s) NOEXCEPT {
        const size_t n = (s + BlockSize - 1) / BlockSize;
        const size_t i = Mask.Allocate(n);

        return ((0 != i)
            ? FAllocatorBlock{ &Blocks[i - 1],  n * BlockSize }
            : FAllocatorBlock::Null() );
    }

    void Deallocate(FAllocatorBlock b) NOEXCEPT {
        Assert_NoAssume(Owns(b));

        const size_t n = (b.SizeInBytes + BlockSize - 1) / BlockSize;
        const size_t i = ((block_t*)b.Data - Blocks);

        Mask.Deallocate(i, n);

        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(b.Data, b.SizeInBytes));
    }

    NODISCARD bool Reallocate(FAllocatorBlock& b, size_t s) {
        Assert_NoAssume(Owns(b));

        const size_t o = (b.SizeInBytes + BlockSize - 1) / BlockSize;
        const size_t n = (s + BlockSize - 1) / BlockSize;
        const size_t i = ((block_t*)b.Data - Blocks);

        if (Likely(o < n)) { // growth
            if (const size_t j = Mask.Reallocate(i, o, n)) {
                const FAllocatorBlock r{ &Blocks[j - 1], o * BlockSize };
                if (i != j) // move the data if the block was relocated
                    FPlatformMemory::Memmove(r.Data, b.Data, b.SizeInBytes);
                b = r;
            }
            else {
                return false;
            }
        }
        else { // shrink or release
            Mask.Deallocate(i + n, o - n);
        }

        return true;
    }

    friend bool operator ==(const TBitmapAllocator& lhs, TBitmapAllocator& rhs) NOEXCEPT {
        return (&lhs == &rhs);
    }

    friend bool operator !=(const TBitmapAllocator& lhs, TBitmapAllocator& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
