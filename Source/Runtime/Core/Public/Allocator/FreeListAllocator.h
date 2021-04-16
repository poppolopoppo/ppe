#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"
#include "Container/IntrusiveList.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Holds a free blocks cache to avoid call _Allocator for each allocation
//----------------------------------------------------------------------------
template <typename _Allocator, size_t _MinSize, size_t _MaxSize, size_t _MaxBlocks>
class TFreeListAllocator : private _Allocator {
public:
    using allocator_traits = TAllocatorTraits<_Allocator>;

    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = typename allocator_traits::propagate_on_container_move_assignment;
    using propagate_on_container_swap = typename allocator_traits::propagate_on_container_swap;

    using is_always_equal = typename allocator_traits::is_always_equal;

    using has_maxsize = typename allocator_traits::has_maxsize;
    using has_owns = typename allocator_traits::has_owns;
    using has_reallocate = typename allocator_traits::has_reallocate;
    using has_acquire = typename allocator_traits::has_acquire;
    using has_steal = typename allocator_traits::has_steal;

    STATIC_CONST_INTEGRAL(size_t, Alignment, allocator_traits::Alignment);

    STATIC_CONST_INTEGRAL(size_t, MinSize, _MinSize);
    STATIC_CONST_INTEGRAL(size_t, MaxSize, _MaxSize);
    STATIC_CONST_INTEGRAL(size_t, MaxBlocks, _MaxBlocks);

    STATIC_ASSERT(MinSize < MaxSize);

    TFreeListAllocator() = default;
    ~TFreeListAllocator() {
        ReleaseFreeList();
    }

    explicit TFreeListAllocator(const _Allocator& alloc)
    :   _Allocator(alloc)
    {}
    explicit TFreeListAllocator(_Allocator&& ralloc)
    :   _Allocator(std::move(alloc))
    {}

    // copy
    TFreeListAllocator(const TFreeListAllocator& other)
    :   _Allocator(allocator_traits::SelectOnCopy(other))
    {}
    TFreeListAllocator& operator =(const TFreeListAllocator& other) {
        IF_CONSTEXPR(propagate_on_container_copy_assignment::value) {
            ReleaseFreeList();
            allocator_traits::Copy(this, other);
        }
        return (*this);
    }

    // move
    TFreeListAllocator(TFreeListAllocator&& rvalue) NOEXCEPT
    :   _Allocator(allocator_traits::SelectOnMove(std::move(rvalue)))
    {}
    TFreeListAllocator& operator =(TFreeListAllocator&& rvalue) NOEXCEPT {
        IF_CONSTEXPR(propagate_on_container_move_assignment::value) {
            ReleaseFreeList();
            allocator_traits::Copy(this, std::move(rvalue));
        }
        return (*this);
    }

    static size_t MaxSize() NOEXCEPT {
        return allocator_traits::MaxSize();
    }

    static size_t SnapSize(size_t s) NOEXCEPT {
        return allocator_traits::SnapSize(s);
    }

    bool Owns(FAllocatorBlock b) const NOEXCEPT {
        return allocator_traits::Owns(*this, b);
    }

    FAllocatorBlock Allocate(size_t s) NOEXCEPT {
        if (FitInFreeList(s) & NumBlocks) {
            FFreeBlock* p = nullptr;
            for (FFreeBlock* f = FreeList.Head(); f; f = p = f, f->Next)
                if (f->SizeInBytes > s) {
                    NumBlocks--;
                    FreeList.Erase(f, p);
                    return f->Block();
                }
        }
        return allocator_traits::Allocate(*this, s);
    }

    void Deallocate(FAllocatorBlock b) NOEXCEPT {
        if (FitInFreeList(b.SizeInBytes) & (NumBlocks < MaxBlocks)) {
            auto* f = reinterpret_cast<FFreeBlock*>(b.Data);
            f->SizeInBytes = b.SizeInBytes;
            NumBlocks++;
            FreeList.PushHead(f);
        }
        else {
            allocator_traits::Deallocate(*this, b);
        }
    }

    auto Reallocate(FAllocatorBlock& b, size_t s) {
        if (b) {
            return allocator_traits::Reallocate(*this, b, s);
        }
        else {
            b = Allocate(s);
        }
    }

    bool Acquire(FAllocatorBlock b) NOEXCEPT {
        return allocator_traits::Acquire(*this, b);
    }

    bool Steal(FAllocatorBlock b) NOEXCEPT {
        return allocator_traits::Steal(*this, b);
    }

    friend bool operator ==(const TFreeListAllocator& lhs, TFreeListAllocator& rhs) NOEXCEPT {
        return allocator_traits::Equals(lhs, rhs);
    }

    friend bool operator !=(const TFreeListAllocator& lhs, TFreeListAllocator& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    friend void swap(TFreeListAllocator& lhs, TFreeListAllocator& rhs) NOEXCEPT {
        IF_CONSTEXPR(propagate_on_container_swap::value) {
            std::swap(lhs.FreeList, rhs.FreeList);
            allocator_traits::Swap(lhs, rhs);
        }
    }

protected: // free list

    struct FFreeBlock {
        FFreeBlock* Next;
        size_t SizeInBytes;

        FAllocatorBlock Block() {
            return FAllocatorBlock{ this, SizeInBytes };
        }
    };
    STATIC_ASSERT(sizeof(FFreeBlock) <= MinSize);

    size_t NumBlocks = 0;
    INTRUSIVESINGLELIST(&FFreeBlock::Next) FreeList;

    static bool FitInFreeList(size_t s) const NOEXCEPT {
        return ((s >= MinSize) & (s <= MaxSize));
    }

    void ReleaseFreeBlock(FFreeBlock* f) {
        Assert(NumBlocks);

        NumBlocks--;
        allocator_traits::Deallocate(f->Block());
    }

    void ReleaseFreeList() {
        while (FFreeBlock * f = FreeList.PopHead())
            ReleaseFreeBlock(f);

        Assert_NoAssume(0 == NumBlocks);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
