#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBinning.h"
#include "Container/Array.h"
#include "Container/Vector.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"

#define SLABHEAP(_DOMAIN) ::PPE::TSlabHeap<ALLOCATOR(_DOMAIN)>
#define SLABHEAP_POOLED(_DOMAIN) ::PPE::TPoolingSlabHeap<ALLOCATOR(_DOMAIN)>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator = ALLOCATOR(Unknown)>
class TSlabHeap : _Allocator, Meta::FNonCopyable {
public:
    STATIC_CONST_INTEGRAL(u32, DefaultSlabSize, PAGE_SIZE);

    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<_Allocator>;

#if USE_PPE_MEMORYDOMAINS
    TSlabHeap() NOEXCEPT;
    explicit TSlabHeap(allocator_type&& ralloc) NOEXCEPT;
    explicit TSlabHeap(const allocator_type& alloc) NOEXCEPT;
#else
    TSlabHeap() = default;
    explicit TSlabHeap(allocator_type&& ralloc) NOEXCEPT
        : allocator_type(allocator_traits::SelectOnMove(std::move(ralloc)))
        , _slabs(static_cast<allocator_type&>(*this))
    {}
    explicit TSlabHeap(const allocator_type& alloc) NOEXCEPT
        : allocator_type(allocator_traits::SelectOnCopy(alloc))
        , _slabs(static_cast<allocator_type&>(*this))
    {}
#endif

    explicit TSlabHeap(Meta::FForceInit) NOEXCEPT;

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
    bool HasLiveBlocks() const NOEXCEPT { return (!!_trackingData.User().NumAllocs); }
#endif

    u32 SlabSize() const { return _slabSize; }
    void SetSlabSize(size_t value) NOEXCEPT;

    NODISCARD PPE_DECLSPEC_ALLOCATOR() void* Allocate(size_t size) {
        Assert(size);
        Assert_NoAssume(CheckCanary_());
        size = Meta::RoundToNextPow2(size, ALLOCATION_BOUNDARY);

        for (FSlabPtr& slab : _slabs) {
            if (slab.Offset + size <= slab.Size) {
                void* const p = (static_cast<u8*>(slab.Ptr) + slab.Offset);

                slab.Offset += checked_cast<u32>(size);

                ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(p, size));
                ONLY_IF_MEMORYDOMAINS(_trackingData.AllocateUser(size));
                ONLY_IF_ASSERT(++_numLiveBlocks);
                return p;
            }
        }

        return Allocate_FromNewSlab_(size);
    }

    NODISCARD void* Reallocate_AssumeLast(void* ptr, size_t newSize, size_t oldSize);
    void Deallocate_AssumeLast(void* ptr, size_t size);
    NODISCARD bool Deallocate_ReturnIfLast(void* ptr, size_t size);

    template <typename T>
    NODISCARD TMemoryView<T> AllocateT(size_t count = 1) {
        return { static_cast<T*>(Allocate(count * sizeof(T))), count };
    }

    template <typename T>
    NODISCARD TMemoryView<T> ReallocateT_AssumeLast(TMemoryView<T> old, size_t count) {
        return { static_cast<T*>(Reallocate_AssumeLast(old.data(), count * sizeof(T), old.SizeInBytes())), count };
    }

    void DiscardAll() NOEXCEPT; // release all blocks, keep slabs allocated
    void ReleaseAll(); // release all blocks and all slabs
    void TrimMemory(); // release unused slabs

    static size_t SnapSize(size_t sizeInBytes) NOEXCEPT {
        return Meta::RoundToNextPow2(sizeInBytes, ALLOCATION_BOUNDARY);
    }

#if !USE_PPE_FINAL_RELEASE
    bool AliasesToHeap(void* ptr) const NOEXCEPT;
#endif
#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking& TrackingData() NOEXCEPT { return _trackingData; }
    const FMemoryTracking& TrackingData() const NOEXCEPT { return _trackingData; }
#endif

private:
    struct FSlabPtr {
        void* Ptr;
        u32 Offset;
        u32 Size : 31;
        u32 Standalone : 1;
    };

    NO_INLINE void* Allocate_FromNewSlab_(size_t size);

    template <typename _OtherAllocator>
    friend class TPoolingSlabHeap;
    void ReclaimUserBlock_AssumeTracked_(void* ptr, size_t size);

    TVector<FSlabPtr, _Allocator> _slabs;
    u32 _slabSize{ DefaultSlabSize };

#if USE_PPE_ASSERT
    u32 _numLiveBlocks{ 0 };
    u64 _canaryForDbg = PPE_HASH_VALUE_SEED_64;
    bool CheckCanary_() const { return (PPE_HASH_VALUE_SEED_64 == _canaryForDbg); }
#endif
#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking _trackingData;
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
template <typename _Allocator = ALLOCATOR(Unknown)>
class TPoolingSlabHeap {
public:
    using heap_type = TSlabHeap<_Allocator>;
    using allocator_type = typename heap_type::allocator_type;
    using allocator_traits = typename heap_type::allocator_traits;

    TPoolingSlabHeap() = default;

    TPoolingSlabHeap(TPoolingSlabHeap&& rvalue) NOEXCEPT
    :   _heap(std::move(rvalue._heap)) {
        forrange(i, 0, NumBins) {
            _pools[i] = rvalue._pools[i];
            rvalue._pools[i] = nullptr;
        }
    }

    TPoolingSlabHeap& operator =(TPoolingSlabHeap&& rvalue) NOEXCEPT {
        _heap = std::move(rvalue._heap);
        forrange(i, 0, NumBins) {
            _pools[i] = rvalue._pools[i];
            rvalue._pools[i] = nullptr;
        }
        return (*this);
    }

    TPoolingSlabHeap(allocator_type&& ralloc) NOEXCEPT : _heap(std::move(ralloc)) {}
    TPoolingSlabHeap(const allocator_type& alloc) NOEXCEPT : _heap(alloc) {}

    ~TPoolingSlabHeap() {
#if USE_PPE_ASSERT
        TrimMemory(); // will trigger an error if a block is still allocated
#else
        ReleaseAll(); // discard all allocations
#endif
    }

    allocator_type& Allocator() { return _heap.Allocator(); }
    const allocator_type& Allocator() const { return _heap.Allocator(); }

#if USE_PPE_ASSERT
    bool HasLiveBlocks() const NOEXCEPT { return _heap.HasLiveBlocks(); }
#endif

    size_t SlabSize() const { return _heap.SlabSize(); }
    void SetSlabSize(size_t value) NOEXCEPT {
        _heap.SetSlabSize(value);
        Assert_Lightweight(_heap.SlabSize() >= MaxBinSize);
    }

#if !USE_PPE_FINAL_RELEASE
    bool AliasesToHeap(void* ptr) const NOEXCEPT { return _heap.AliasesToHeap(ptr); }
#endif
#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking& TrackingData() NOEXCEPT { return _heap.TrackingData(); }
    const FMemoryTracking& TrackingData() const NOEXCEPT { return _heap.TrackingData(); }
#endif

    NODISCARD PPE_DECLSPEC_ALLOCATOR() void* Allocate(size_t size) {
        Assert_NoAssume(_heap.CheckCanary_());
        if (size <= MaxBinSize) {
            const u32 pool = FAllocatorBinning::IndexFromSize(size);
            Assert_NoAssume(FAllocatorBinning::BinSizes[pool] >= size);
            return PoolAlloc(pool);
        }
        return _heap.Allocate(size);
    }

    void Deallocate(void* ptr, size_t size) NOEXCEPT {
        Assert_NoAssume(_heap.CheckCanary_());
        // always try to resize the heap first
        if (not _heap.Deallocate_ReturnIfLast(ptr, size)) {
            if (size <= MaxBinSize) {
                // put small block back in pools
                const u32 pool = FAllocatorBinning::IndexFromSize(size);
                Assert_NoAssume(FAllocatorBinning::BinSizes[pool] >= size);
                PoolFree(pool, ptr);
            }
            else {
                _heap.ReclaimUserBlock_AssumeTracked_(ptr, size);
            }
        }
    }

    NODISCARD PPE_DECLSPEC_ALLOCATOR() void* PoolAlloc(u32 pool) {
        Assert_NoAssume(_heap.CheckCanary_());
        Assert(pool < NumBins);
        if (_pools[pool]) {
            void* const p = _pools[pool];
            _pools[pool] = *static_cast<void**>(p);
            ONLY_IF_MEMORYDOMAINS(TrackingData().AllocateUser(FAllocatorBinning::BinSizes[pool]));
            ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(p, FAllocatorBinning::BinSizes[pool]));
            return p;
        }
        return _heap.Allocate(FAllocatorBinning::BinSizes[pool]);
    }

    void PoolFree(u32 pool, void* ptr) NOEXCEPT {
        Assert_NoAssume(_heap.CheckCanary_());
        Assert(pool < NumBins);
        ONLY_IF_MEMORYDOMAINS(TrackingData().DeallocateUser(FAllocatorBinning::BinSizes[pool]));
        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(ptr, FAllocatorBinning::BinSizes[pool]));
        *static_cast<void**>(ptr) = _pools[pool];
        _pools[pool] = ptr;
    }

    template <typename T>
    NODISCARD PPE_DECLSPEC_ALLOCATOR() T* AllocateT() {
        CONSTEXPR size_t pool = FAllocatorBinning::IndexFromSizeConst(sizeof(T));
        STATIC_ASSERT(pool < NumBins);
        return static_cast<T*>(PoolAlloc(pool));
    }

    template <typename T>
    NODISCARD PPE_DECLSPEC_ALLOCATOR() TMemoryView<T> AllocateT(size_t count) {
        return { static_cast<T*>(Allocate(sizeof(T) * count)), count };
    }

    template <typename T>
    NODISCARD PPE_DECLSPEC_ALLOCATOR() TMemoryView<T> AllocateCopyT(TMemoryView<T> src) {
        TMemoryView<Meta::TRemoveConst<T>> dst;
        if (not src.empty()) {
            dst = AllocateT<Meta::TRemoveConst<T>>(checked_cast<u32>(src.size()));
            src.CopyTo(dst);
        }
        return dst;
    }

    template <typename T, typename _Map>
    NODISCARD PPE_DECLSPEC_ALLOCATOR() auto AllocateCopyT(TMemoryView<T> src, _Map&& map) {
        using map_type = decltype(std::declval<_Map>()(std::declval<T&>()));
        TMemoryView<map_type> dst;
        if (not src.empty()) {
            dst = AllocateT<map_type>(src.size());
            src.Map(std::forward<_Map>(map)).UninitializedCopyTo(dst.begin());
        }
        return dst;
    }

    template <typename T>
    void DeallocateT(T* ptr) {
        CONSTEXPR size_t pool = FAllocatorBinning::IndexFromSizeConst(sizeof(T));
        STATIC_ASSERT(pool < NumBins);
        return PoolFree(pool, ptr);
    }

    NODISCARD void* Reallocate(void* ptr, size_t newSize, size_t oldSize);

    void DiscardAll() NOEXCEPT;
    void ReleaseAll();
    void TrimMemory();

    static size_t SnapSize(size_t sizeInBytes) NOEXCEPT {
        return (sizeInBytes <= MaxBinSize
            ? FAllocatorBinning::BoundSizeToBins(checked_cast<u32>(sizeInBytes))
            : heap_type::SnapSize(sizeInBytes) );
    }

private:
    STATIC_CONST_INTEGRAL(size_t, NumBins, FAllocatorBinning::NumBins_4kPages);
    STATIC_CONST_INTEGRAL(size_t, MaxBinSize, FAllocatorBinning::MaxBinSize_4kPages);

    TSlabHeap<allocator_type> _heap;
    TStaticArray<void*, NumBins> _pools{ 0 };
};
//----------------------------------------------------------------------------
#if PPE_HAS_CXX17
template <typename _Allocator>
TPoolingSlabHeap(_Allocator&&) -> TPoolingSlabHeap< Meta::TDecay<_Allocator> >;
template <typename _Allocator>
TPoolingSlabHeap(const _Allocator&) -> TPoolingSlabHeap< Meta::TDecay<_Allocator> >;
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
    using namespace PPE;
    Unused(ptr);
    Unused(heap);
    AssertNotImplemented(); // not supported
}
//----------------------------------------------------------------------------
// Use TPoolingSlabHeap<> with allocation operators
//----------------------------------------------------------------------------
template <typename _Allocator>
inline void* operator new(size_t sizeInBytes, PPE::TPoolingSlabHeap<_Allocator>& heap) NOEXCEPT {
    return heap.Allocate(sizeInBytes);
}
template <typename _Allocator>
inline void operator delete(void* ptr, PPE::TPoolingSlabHeap<_Allocator>& heap) NOEXCEPT {
    using namespace PPE;
    Unused(ptr);
    Unused(heap);
    AssertNotImplemented(); // could be supported if block size was known
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
