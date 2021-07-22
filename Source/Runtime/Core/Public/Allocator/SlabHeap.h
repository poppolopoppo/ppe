#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBinning.h"
#include "Container/Array.h"
#include "Container/Vector.h"
#include "HAL/PlatformDebug.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"

#if USE_PPE_MEMORYDOMAINS
#   define SLABHEAP(_DOMAIN) ::PPE::TSlabHeap<MEMORYDOMAIN_TAG(_DOMAIN)>
#   define SLABHEAP_POOLED(_DOMAIN) ::PPE::TPoolingSlabHeap<MEMORYDOMAIN_TAG(_DOMAIN)>
#else
#   define SLABHEAP(_DOMAIN) ::PPE::FSlabHeap
#   define SLABHEAP_POOLED(_DOMAIN) ::PPE::FPoolingSlabHeap
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FSlabMarker {
    std::streamoff Origin{ 0 };
#if USE_PPE_MEMORYDOMAINS
    struct {
        size_t NumAllocs{ 0 };
        size_t TotalSize{ 0 };
    }   Snapshot;
#endif
};
//----------------------------------------------------------------------------
class PPE_CORE_API FSlabHeap : Meta::FNonCopyableNorMovable {
public:
    static const u32 DefaultSlabSize;

    class FStackMarker : Meta::FThreadResource {
        TPtrRef<FSlabHeap> _heap;
        FSlabMarker _origin;
    public:
        FStackMarker() = default;
        explicit FStackMarker(FSlabHeap& heap) NOEXCEPT
        :   _heap(heap), _origin(_heap->Tell())
        {}
        ~FStackMarker() NOEXCEPT {
            if (_heap)
                _heap->Rewind(_origin);
        }
        const FSlabMarker& Tell() const { return _origin; }
        FSlabHeap* operator ->() const {
            THIS_THREADRESOURCE_CHECKACCESS();
            return _heap.get();
        }
    };

    explicit FSlabHeap(ARG0_IF_MEMORYDOMAINS(FMemoryTracking* pParent)) NOEXCEPT;
    ~FSlabHeap();

    u32 SlabSize() const { return _slabSize; }
    void SetSlabSize(size_t value) NOEXCEPT;

    FSlabMarker Tell() const NOEXCEPT;
    void Rewind(const FSlabMarker& marker) NOEXCEPT;

    FStackMarker StackMarker() NOEXCEPT {
        return FStackMarker( *this );
    }

    NODISCARD PPE_DECLSPEC_ALLOCATOR() void* Allocate(size_t size) {
        Assert(size);
        size = Meta::RoundToNext(size, ALLOCATION_BOUNDARY);

        FSlabPtr* const pSlab = SeekSlab_(_tell);
        if (Likely(pSlab)) {
            Assert_NoAssume(pSlab->Origin + pSlab->Offset == _tell);
            if (pSlab->Offset + size <= pSlab->Size) {
                void* const p = (static_cast<u8*>(pSlab->Ptr) + pSlab->Offset);

                _tell += size;
                pSlab->Offset += checked_cast<u32>(size);

                ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(p, size));
                ONLY_IF_MEMORYDOMAINS(_trackingData.AllocateUser(size));
                return p;
            }
        }

        return Allocate_FromNewSlab_(pSlab, size);
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
        return Meta::RoundToNext(sizeInBytes, ALLOCATION_BOUNDARY);
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
        void* Ptr{nullptr};
        u32 Offset{0};
        u32 Size{0};
        std::streamoff Origin{ 0 };
    };

    NO_INLINE void* Allocate_FromNewSlab_(FSlabPtr* pCurr, size_t size);

    FSlabPtr* SeekSlab_(std::streamsize off) NOEXCEPT {
        reverseforeachitem(it, _slabs) {
            if (it->Origin <= off)
                return std::addressof(*it);
        }
        return nullptr;
    }

    VECTORMINSIZE(Internal, FSlabPtr, 16) _slabs;
    u32 _slabSize{ DefaultSlabSize };
    std::streamoff _tell{ 0 };

#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking _trackingData;
#endif
};
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Domain = MEMORYDOMAIN_TAG(Container)>
class TSlabHeap : public FSlabHeap {
public:
    TSlabHeap() NOEXCEPT : FSlabHeap(std::addressof(_Domain::TrackingData())) {}
};
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FPoolingSlabHeap {
public:
    explicit FPoolingSlabHeap(ARG0_IF_MEMORYDOMAINS(FMemoryTracking* pParent)) NOEXCEPT;

    size_t SlabSize() const { return _heap.SlabSize(); }
    void SetSlabSize(size_t value) NOEXCEPT {
        AssertRelease_NoAssume(value >= FAllocatorBinning::MaxBinSize);
        _heap.SetSlabSize(value);
    }

#if !USE_PPE_FINAL_RELEASE
    bool AliasesToHeap(void* ptr) const NOEXCEPT { return _heap.AliasesToHeap(ptr); }
#endif
#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking& TrackingData() NOEXCEPT { return _heap.TrackingData(); }
    const FMemoryTracking& TrackingData() const NOEXCEPT { return _heap.TrackingData(); }
#endif

    NODISCARD PPE_DECLSPEC_ALLOCATOR() void* Allocate(size_t size) {
        if (size <= FAllocatorBinning::MaxBinSize) {
            const u32 pool = FAllocatorBinning::IndexFromSize(size);
            Assert_NoAssume(FAllocatorBinning::BinSizes[pool] >= size);
            return PoolAlloc(pool);
        }
        return Allocate_SpareBlock_(size);
    }

    void Deallocate(void* ptr, size_t size) NOEXCEPT {
        // always try to resize the heap first
        if (not _heap.Deallocate_ReturnIfLast(ptr, size)) {
            if (size <= FAllocatorBinning::MaxBinSize) {
                // put small block back in pools
                const u32 pool = FAllocatorBinning::IndexFromSize(size);
                Assert_NoAssume(FAllocatorBinning::BinSizes[pool] >= size);
                PoolFree(pool, ptr);
            }
            else {
                // larger blocks are split in pools
                Deallocate_SpareBlock_(ptr, size);
            }
        }
    }

    NODISCARD PPE_DECLSPEC_ALLOCATOR() void* PoolAlloc(u32 pool) {
        Assert(pool < FAllocatorBinning::NumBins);
        if (_pools[pool]) {
            void* const p = _pools[pool];
            _pools[pool] = *static_cast<void**>(p);
            ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(p, FAllocatorBinning::BinSizes[pool]));
            return p;
        }
        return Allocate_SpareBlock_(FAllocatorBinning::BinSizes[pool]);
    }

    void PoolFree(u32 pool, void* ptr) NOEXCEPT {
        Assert(pool < FAllocatorBinning::NumBins);
        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(ptr, FAllocatorBinning::BinSizes[pool]));
        *static_cast<void**>(ptr) = _pools[pool];
        _pools[pool] = ptr;
    }

    template <typename T>
    NODISCARD PPE_DECLSPEC_ALLOCATOR() T* AllocateT() {
        CONSTEXPR size_t pool = FAllocatorBinning::IndexFromSizeConst(sizeof(T));
        STATIC_ASSERT(pool < FAllocatorBinning::NumBins);
        return static_cast<T*>(PoolAlloc(pool));
    }

    template <typename T>
    NODISCARD PPE_DECLSPEC_ALLOCATOR() TMemoryView<T> AllocateT(u32 count) {
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
            dst = AllocateT<map_type>(checked_cast<u32>(src.size()));
            src.Map(std::forward<_Map>(map)).UnitializedCopyTo(dst.begin());
        }
        return dst;
    }

    template <typename T>
    void DeallocateT(T* ptr) {
        CONSTEXPR size_t pool = FAllocatorBinning::IndexFromSizeConst(sizeof(T));
        STATIC_ASSERT(pool < FAllocatorBinning::NumBins);
        return PoolFree(pool, ptr);
    }

    NODISCARD void* Reallocate(void* ptr, size_t newSize, size_t oldSize) {
        if (!!oldSize && !!newSize) {
            newSize = SnapSize(newSize);
            oldSize = SnapSize(oldSize);

            if (newSize != oldSize) {
                // don't use PoolXXX() functions here: not guaranteed to fit in pool
                void* const newp = Allocate(newSize);
                FPlatformMemory::Memcpy(newp, ptr, Min(newSize, oldSize));
                Deallocate(ptr, oldSize);
                ptr = newp;
            }

            return ptr;
        }
        else if (oldSize) {
            Assert_NoAssume(0 == newSize);
            Deallocate(ptr, oldSize);
            return nullptr;
        }
        else {
            Assert(newSize);
            Assert(nullptr == ptr);
            return Allocate(newSize);
        }
    }

    void DiscardAll() NOEXCEPT;
    void ReleaseAll();
    void TrimMemory();

    static size_t SnapSize(size_t sizeInBytes) NOEXCEPT {
        return (sizeInBytes <= FAllocatorBinning::MaxBinSize
            ? FAllocatorBinning::BoundSizeToBins(checked_cast<u32>(sizeInBytes))
            : FSlabHeap::SnapSize(sizeInBytes) );
    }

private:
    struct FSpareBlock_ {
        size_t BlockSize;
        FSpareBlock_* pNext;
        void* EndPtr() const { return ((u8*)this + BlockSize); }
    };

    NO_INLINE void* Allocate_SpareBlock_(size_t size);
    NO_INLINE void Deallocate_SpareBlock_(void* ptr, size_t size) NOEXCEPT;

    FSlabHeap _heap;
    FSpareBlock_* _spares{ nullptr };
    TStaticArray<void*, FAllocatorBinning::NumBins> _pools;
};
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Domain = MEMORYDOMAIN_TAG(Container)>
class TPoolingSlabHeap : public FPoolingSlabHeap {
public:
    TPoolingSlabHeap() NOEXCEPT : FPoolingSlabHeap(std::addressof(_Domain::TrackingData())) {}
};
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Use FSlabHeap with allocation operators
//----------------------------------------------------------------------------
inline void* operator new(size_t sizeInBytes, PPE::FSlabHeap& heap) NOEXCEPT {
    return heap.Allocate(sizeInBytes);
}
inline void operator delete(void* ptr, PPE::FSlabHeap& heap) NOEXCEPT {
    UNUSED(ptr);
    UNUSED(heap);
    AssertNotImplemented(); // not supported
}
//----------------------------------------------------------------------------
// Use FPoolingSlabHeap with allocation operators
//----------------------------------------------------------------------------
inline void* operator new(size_t sizeInBytes, PPE::FPoolingSlabHeap& heap) NOEXCEPT {
    return heap.Allocate(sizeInBytes);
}
inline void operator delete(void* ptr, PPE::FPoolingSlabHeap& heap) NOEXCEPT {
    UNUSED(ptr);
    UNUSED(heap);
    AssertNotImplemented(); // could be supported if block size was known
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
