#pragma once

#include "Core_fwd.h"

#include "Container/Array.h"
#include "Container/IntrusiveWAVL.h"
#include "Container/Vector.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"

#define SLABHEAP(_DOMAIN) ::PPE::TSlabHeap<ALLOCATOR(_DOMAIN)>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator = ALLOCATOR(Unknown)>
class TSlabHeap : _Allocator {
public:
    STATIC_CONST_INTEGRAL(u32, DefaultSlabSize, PAGE_SIZE);

    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<_Allocator>;

    TSlabHeap() NOEXCEPT;

    explicit TSlabHeap(Meta::FForceInit) NOEXCEPT;

    explicit TSlabHeap(allocator_type&& ralloc) NOEXCEPT;
    explicit TSlabHeap(const allocator_type& alloc) NOEXCEPT;

    TSlabHeap(const TSlabHeap& ) = delete;
    TSlabHeap& operator =(const TSlabHeap& ) = delete;

    TSlabHeap(TSlabHeap&& rvalue) NOEXCEPT
    :   TSlabHeap(ForceInit) {
        operator =(std::move(rvalue));
    }
    TSlabHeap& operator =(TSlabHeap&& rvalue) NOEXCEPT {
        Assert_NoAssume(rvalue.CheckCanary_());
        allocator_traits::Move(this, std::move(rvalue.Allocator()));
        _slabs = std::move(rvalue._slabs);
        _slabSize = rvalue._slabSize;
        rvalue._slabSize = DefaultSlabSize;
#if USE_PPE_ASSERT
        _numLiveBlocks = rvalue._numLiveBlocks;
        rvalue._numLiveBlocks = 0;
#endif
#if USE_PPE_MEMORYDOMAINS
        rvalue._trackingData.MoveTo(&_trackingData);
#endif
        return (*this);
    }

    ~TSlabHeap();

    allocator_type& Allocator() { return allocator_traits::Get(*this); }
    const allocator_type& Allocator() const { return allocator_traits::Get(*this); }

#if USE_PPE_MEMORYDOMAINS
    bool HasLiveBlocks_ForDebugOnly() const NOEXCEPT { return (!!_trackingData.User().NumAllocs); }
#endif

    u32 SlabSize() const { return _slabSize; }
    void SetSlabSize(size_t value) NOEXCEPT;

    NODISCARD PPE_DECLSPEC_ALLOCATOR() void* Allocate(size_t size);
    NODISCARD PPE_DECLSPEC_ALLOCATOR() void* Reallocate(void* p, size_t newSize, size_t oldSize);
    void Deallocate(void* p, size_t sz);

    template <typename T>
    NODISCARD TMemoryView<T> AllocateT(size_t count = 1) {
        return { static_cast<T*>(Allocate(count * sizeof(T))), count };
    }
    template <typename T>
    NODISCARD TMemoryView<T> ReallocateT(TMemoryView<T> old, size_t count) {
        return { static_cast<T*>(Reallocate(old.data(), count * sizeof(T), old.SizeInBytes())), count };
    }
    template <typename T>
    void DeallocateT(TMemoryView<T> view) {
        Deallocate(view.data(), view.SizeInBytes());
    }

    size_t RegionSize(void* p) const NOEXCEPT;

    void DiscardAll() NOEXCEPT; // release all blocks, keep slabs allocated
    void ReleaseAll(); // release all blocks and all slabs
    void TrimMemory(); // release unused slabs

    static CONSTEXPR size_t SnapSize(size_t sizeInBytes) NOEXCEPT {
        return (Meta::RoundToNextPow2(sizeInBytes + BlockOverhead, ALLOCATION_BOUNDARY) - BlockOverhead);
    }

    bool AliasesToHeap(void* ptr) const NOEXCEPT;

#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking& TrackingData() NOEXCEPT { return _trackingData; }
    const FMemoryTracking& TrackingData() const NOEXCEPT { return _trackingData; }
#endif
#if USE_PPE_ASSERT
    bool CheckInvariants() const NOEXCEPT;
#endif

private:
    struct FSlab;
    struct FBlockHeader;
    struct FBlockFooter;

    struct FBlockHeader {
        // avl tree node stored in memory excess (FBlockHeader must be aligned on ALLOCATION_BOUNDARY)
        u64 Available       : 1;
        u64 FirstBlock      : 1;
        u64 Rank            : 20;
        i64 LeftOffset      : 42;
        u64 BlockSize       : 22;
        i64 RightOffset     : 42;

        bool HasPrevBlock() const { return not FirstBlock; }

        FBlockHeader* Left() const {
            return (!!LeftOffset ? bit_cast<FBlockHeader*>(bit_cast<intptr_t>(this) + (static_cast<intptr_t>(LeftOffset) * ALLOCATION_BOUNDARY)) : nullptr);
        }
        FBlockHeader* Right() const {
            return (!!RightOffset ? bit_cast<FBlockHeader*>(bit_cast<intptr_t>(this) + (static_cast<intptr_t>(RightOffset) * ALLOCATION_BOUNDARY)) : nullptr);
        }

        void SetLeft(FBlockHeader* node) {
            LeftOffset = (node ? (bit_cast<intptr_t>(node) - bit_cast<intptr_t>(this)) / ALLOCATION_BOUNDARY : 0);
            Assert_NoAssume(Left() == node);
        }
        void SetRight(FBlockHeader* node) {
            RightOffset = (node ? (bit_cast<intptr_t>(node) - bit_cast<intptr_t>(this)) / ALLOCATION_BOUNDARY : 0);
            Assert_NoAssume(Right() == node);
        }

        void* Data() const { return const_cast<FBlockHeader*>(this+1); }
        FAllocatorBlock Allocation() const { return { Data(), BlockSize }; }
        size_t AvailableSize() const { return (Available ? BlockSize : 0); }

        static FBlockHeader* FromData(void* p) {
            FBlockHeader* const header = (static_cast<FBlockHeader*>(p) - 1);
            Assert_NoAssume(BlockFooter(header)->Header() == header);
            return header;
        }

        // for WAVL tree
        NODISCARD friend bool operator < (const FBlockHeader& lhs, const FBlockHeader& rhs) NOEXCEPT {
#if 0
            return (lhs.BlockSize < rhs.BlockSize);
#else// also compares block address to get an absolute sorting order and avoid lhs == rhs when block sizes match
            return (lhs.BlockSize != rhs.BlockSize ? lhs.BlockSize < rhs.BlockSize : &lhs < &rhs);
#endif
        }
    };
    STATIC_ASSERT(sizeof(FBlockHeader) == ALLOCATION_BOUNDARY);

    struct FBlockFooter {
        Meta::TPointerWFlags<FBlockHeader> HeaderWFlags;
        FBlockHeader* Header() const { return HeaderWFlags.Get(); }
        bool HasNextBlock() const { return HeaderWFlags.Flag0(); }
    };

    // store free blocks in a self-balanced binary tree
    // WAVL should be faster overall than AVL for insertion/deletion
    // (lookup is the same, but AVL are strictly balanced while WAVL relaxes constraints)
    struct FBlockWAVLTraits {
        using pointer = FBlockHeader*;

        NODISCARD static i32 Rank(pointer node) { return node->Rank; }
        static void SetRank(pointer node, i32 height) { node->Rank = height; }

        NODISCARD static pointer Left(pointer node) { return node->Left(); }
        static void SetLeft(pointer node, pointer child) { node->SetLeft(child); }

        NODISCARD static pointer Right(pointer node) { return node->Right(); }
        static void SetRight(pointer node, pointer child) { node->SetRight(child); }
    };
    using FFreeBlockTree = TWAVLTree<FBlockHeader, Meta::TLess<FBlockHeader>, FBlockWAVLTraits>;

    NODISCARD static FBlockFooter* BlockFooter(FBlockHeader* header) {
        return reinterpret_cast<FBlockFooter*>(bit_cast<intptr_t>(header + 1) + header->BlockSize);
    }
    NODISCARD static FBlockHeader* NextBlock(FBlockFooter* footer) {
        if (Likely(footer->HasNextBlock()))
            return reinterpret_cast<FBlockHeader*>(footer + 1);
        return nullptr;
    }
    NODISCARD static FBlockHeader* PrevBlock(FBlockHeader* header) {
        if (Likely(header->HasPrevBlock()))
            return (reinterpret_cast<FBlockFooter*>(header) - 1)->Header();
        return nullptr;
    }
    NODISCARD static FBlockHeader* NextBlock(FBlockHeader* header) {
        return NextBlock(BlockFooter(header));
    }
    NODISCARD static FBlockHeader* PrevBlock(FBlockFooter* footer) {
        return PrevBlock(footer->Header());
    }

    STATIC_CONST_INTEGRAL(size_t, BlockOverhead, sizeof(FBlockHeader)+sizeof(FBlockFooter));
    STATIC_CONST_INTEGRAL(size_t, MinBlockSize, SnapSize(BlockOverhead+ALLOCATION_BOUNDARY));

    friend struct FSlab;
    struct FSlab {
        FBlockHeader* FirstBlock;
        u32 Capacity;
        //u32 SizeInUse;

        void Reset(FAllocatorBlock alloc) {
            Capacity = checked_cast<u32>(alloc.SizeInBytes);
            FirstBlock = static_cast<FBlockHeader*>(alloc.Data);

            FirstBlock->Available = true;
            FirstBlock->FirstBlock = true;
            FirstBlock->BlockSize = (Capacity - BlockOverhead);

            BlockFooter(FirstBlock)->HeaderWFlags.Reset(FirstBlock, false, false);
        }

        FAllocatorBlock Allocation() const {
            return { FirstBlock, Capacity };
        }
        bool HasLiveAllocations() const {
            return (not FirstBlock->Available || NextBlock(FirstBlock)); // there should only big available node if the slab is empty
        }

        NODISCARD static void* Allocate(TSlabHeap& heap, FBlockHeader* block, size_t userSize);
        NODISCARD static void* Reallocate(TSlabHeap& heap, FBlockHeader* block, size_t newSize);
        static void Deallocate(TSlabHeap& heap, FBlockHeader* block);

        NODISCARD static FBlockHeader* CoalesceLeft(TSlabHeap& heap, FBlockHeader* block);
        NODISCARD static FBlockHeader* CoalesceRight(TSlabHeap& heap, FBlockHeader* block);

        static FBlockHeader* SplitBlockIFP(TSlabHeap& heap, FBlockHeader* block, size_t userSize);

#if USE_PPE_ASSERT
        bool CheckPredicates_() const;
#endif
    };

    NODISCARD NO_INLINE void* Allocate_FromNewSlab_(size_t size);

    TVectorInSitu<FSlab, 3, _Allocator> _slabs;
    FFreeBlockTree _freeBlocks;
    u32 _slabSize{ DefaultSlabSize };

#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking _trackingData;
#endif

#if USE_PPE_ASSERT
    u64 _canaryForDbg = PPE_HASH_VALUE_SEED_64;
    u32 _numLiveBlocks{ 0 };
    bool CheckCanary_() const { return (PPE_HASH_VALUE_SEED_64 == _canaryForDbg); }
#endif
};
//----------------------------------------------------------------------------
#if PPE_HAS_CXX17
template <typename _Allocator>
TSlabHeap(_Allocator&&) -> TSlabHeap< Meta::TDecay<_Allocator> >;
template <typename _Allocator>
TSlabHeap(const _Allocator&) -> TSlabHeap< Meta::TDecay<_Allocator> >;
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Allocator/SlabHeap-inl.h"

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Use TSlabHeap<> with allocation operators
//----------------------------------------------------------------------------
template <typename _Allocator>
inline void* operator new(size_t sizeInBytes, PPE::TSlabHeap<_Allocator>& heap) NOEXCEPT {
    return heap.Allocate(sizeInBytes);
}
template <typename _Allocator>
inline void operator delete(void* ptr, PPE::TSlabHeap<_Allocator>& heap) NOEXCEPT {
    heap.Deallocate(ptr, heap.RegionSize(ptr));
}
template <typename _Allocator>
inline void operator delete(void* ptr, size_t sizeInBytes, PPE::TSlabHeap<_Allocator>& heap) NOEXCEPT {
    heap.Deallocate(ptr, sizeInBytes);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
