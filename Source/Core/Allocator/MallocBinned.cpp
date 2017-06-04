#include "stdafx.h"

#include "MallocBinned.h"

#include "VirtualMemory.h"
#include "Container/IntrusiveList.h"
#include "Misc/TargetPlatform.h"
#include "Thread/AtomicSpinLock.h"

PRAGMA_INITSEG_LIB

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_ASSERT(64 == CACHELINE_SIZE);
//----------------------------------------------------------------------------
#ifdef WITH_CORE_ASSERT
static void FillBlockDeleted_(void* p, size_t size) { ::memset(p, 0xDD, size); }
static void FillBlockUninitialized_(void* p, size_t size) { ::memset(p, 0xCC, size); }
static void FillBlockPage_(void* p, size_t n, size_t size) {
    u8* block = (u8*)p;
    forrange(i, 0, n) {
        *(u16*)block = u16(i);
        ::memset(block + sizeof(u16), 0xAA, size - sizeof(u16));
        block += size;
    }
}
#endif
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedPage_ {
    STATIC_CONST_INTEGRAL(size_t, PageSize,         64 * 1024);

    TIntrusiveSingleListNode<FBinnedPage_> Node;

    FORCE_INLINE static FBinnedPage_* Allocate() {
        return (FBinnedPage_*)FVirtualMemory::AlignedAlloc(PageSize, PageSize);
    }

    FORCE_INLINE static void Release(FBinnedPage_* p) {
        FVirtualMemory::AlignedFree(p, PageSize);
    }
};
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedChunk_ {
    STATIC_CONST_INTEGRAL(size_t, Alignment,        16);
    STATIC_CONST_INTEGRAL(size_t, MinSize,          16);
    STATIC_CONST_INTEGRAL(size_t, MaxSize,          32736);
    STATIC_CONST_INTEGRAL(size_t, ChunkSize,        FBinnedPage_::PageSize);
    STATIC_CONST_INTEGRAL(size_t, ChunkSizeMask,    ~(ChunkSize - 1));
    STATIC_CONST_INTEGRAL(size_t, ChunkAvailable,   ChunkSize - CACHELINE_SIZE);

    struct FBlock {
        FBlock* Next;
        FBinnedChunk_* Owner() const {
            FBinnedChunk_* owner = (FBinnedChunk_*)((uintptr_t(this) & ChunkSizeMask));
            Assert(owner->CheckCanaries_());
            Assert(owner->CheckThreadSafety_());
            return owner;
        }
    };

    ~FBinnedChunk_() {
        Assert(CheckCanaries_());
        Assert(CheckThreadSafety_());
    }

    size_t Class() const { return _class; }
    size_t BlockSizeInBytes() const { return GClassesSize_[_class]; }
    size_t BlockTotalCount() const { return (ChunkAvailable / GClassesSize_[_class]); }
    size_t BlockUsedCount() const { return (_usedCount); }

    bool empty() const { return (BlockTotalCount() == _freeCount); }

    FORCE_INLINE FBlock* Allocate() {
        Assert(CheckCanaries_());
        Assert(CheckThreadSafety_());

        Likely(_freeCount);
        if (0 == _freeCount)
            return nullptr;

        FBlock* p;
        if (_freeBlock) {
            p = _freeBlock;
            _freeBlock = p->Next;
        }
        else {
            p = BlockAt_(_usedCount++);
        }

        Assert(_freeCount);
        _freeCount--;

        Assert(IS_ALIGNED(Alignment, p));
        ONLY_IF_ASSERT(FillBlockUninitialized_(p, GClassesSize_[_class]));

        return p;
    }

    FORCE_INLINE void Release(FBlock* p) {
        Assert(CheckCanaries_());
        Assert(CheckThreadSafety_());

        ONLY_IF_ASSERT(FillBlockDeleted_(p, GClassesSize_[_class]));

        p->Next = _freeBlock;
        _freeBlock = p;
        _freeCount++;
    }

    static constexpr size_t NumClasses = 45;
    FORCE_INLINE static size_t MakeClass(size_t size) {
        constexpr size_t POW_N = 2;
        constexpr size_t MinClassIndex = 19;
        size = ROUND_TO_NEXT_16(size);
        const size_t index = Meta::FloorLog2((size - 1) | 1);
        return ((index << POW_N) + ((size - 1) >> (index - POW_N)) - MinClassIndex);
    }

private:
    friend struct FBinnedThreadCache_;

#ifdef WITH_CORE_ASSERT
    STATIC_CONST_INTEGRAL(u32, Canary0, 0xA0FACADE);
    const u32 _canary0 = Canary0;
#endif

    const u32 _class;

    u32 _freeCount;
    u32 _usedCount;

    FBlock* _freeBlock;

    TIntrusiveListNode<FBinnedChunk_> _node;
    typedef INTRUSIVELIST_ACCESSOR(&FBinnedChunk_::_node) list_type;

#ifdef WITH_CORE_ASSERT
    const std::thread::id _threadId = std::this_thread::get_id();

    STATIC_CONST_INTEGRAL(u32, Canary1, 0xBAADF00D);
    const u32 _canary1 = Canary1;

    bool CheckCanaries_() const { return (_canary0 == Canary0 && _canary1 == Canary1); }
    bool CheckThreadSafety_() const { return (std::this_thread::get_id() == _threadId); }
#endif

    static constexpr u16 GClassesSize_[NumClasses] = {
       16,     0,     0,     0,    32,     0,
       48,     0,    64,    80,    96,   112,
      128,   160,   192,   224,   256,   320,
      384,   448,   512,   640,   768,   896,
     1024,  1280,  1536,  1792,  2048,  2560,
     3072,  3584,  4096,  5120,  6144,  7168,
     8192, 10240, 12288, 14336, 16384, 20480,
    24576, 28672, 32736,
    /*
       16,     0,     0,     0,    32,     0,
       48,     0,    64,    80,    96,   112,
      128,   160,   176,   224,   256,   288,
      352,   448,   496,   528,   704,   896,
      992,  1056,  1488,  1632,  1984,  2112,
     2976,  3440,  3632,  4672,  5456,  6544,
     8176,  9344, 10912, 13088, 16368, 20480,
    21824, 28672, 32736,
    */
    };

    explicit FBinnedChunk_(size_t class_)
        : _class(checked_cast<u32>(class_))
        , _freeCount(checked_cast<u32>(BlockTotalCount()))
        , _usedCount(0)
        , _freeBlock(nullptr) {
        Assert(IS_ALIGNED(ChunkSize, this));
        ONLY_IF_ASSERT(FillBlockPage_(BlockAt_(0), BlockTotalCount(), BlockSizeInBytes()));
    }

    FBlock* BlockAt_(size_t index) const {
        Assert(index < BlockTotalCount());
#if 1
        // blocks in chunk are located in such way to achieve a maximum possible alignment
        return ((FBlock*)((u8*)this + ChunkSize - (BlockTotalCount() - index) * BlockSizeInBytes()));
#else
        return ((FBlock*)((u8*)(this + 1) + index * BlockSizeInBytes()));
#endif
    }

};
STATIC_ASSERT(sizeof(FBinnedChunk_) == CACHELINE_SIZE);
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedGlobalCache_ {
    STATIC_CONST_INTEGRAL(size_t, FreePagesMax, 128); // <=> 8 mo global cache (128 * 64 * 1024)

    static FBinnedGlobalCache_ GInstance;

    ~FBinnedGlobalCache_() {
        // release pages in cache
        while (FBinnedPage_* p = _freePages.PopHead()) {
            Assert(_freePagesCount);
            FBinnedPage_::Release(p);
            _freePagesCount--;
        }
    }

    FBinnedPage_* AllocPage() {
        Likely(_freePagesCount);
        if (_freePagesCount) {
            const FAtomicSpinLock::FScope scopeLock(_barrier);
            if (FBinnedPage_* p = _freePages.PopHead()) {
                Assert(_freePagesCount);
                _freePagesCount--;
                return p;
            };
        }
        return FBinnedPage_::Allocate();
    }

    void ReleasePage(FBinnedPage_* page) {
        Assert(page);
        Likely(_freePagesCount < FreePagesMax);
        if (_freePagesCount < FreePagesMax) {
            const FAtomicSpinLock::FScope scopeLock(_barrier);
            Assert(_freePagesCount < FreePagesMax);
            _freePages.PushFront(page);
            _freePagesCount++;
        }
        else {
            FBinnedPage_::Release(page);
        }
    }

private:
    size_t _freePagesCount;
    FAtomicSpinLock _barrier;
    INTRUSIVESINGLELIST(&FBinnedPage_::Node) _freePages;

    FBinnedGlobalCache_() : _freePagesCount(0) {}
};
FBinnedGlobalCache_ FBinnedGlobalCache_::GInstance;
//----------------------------------------------------------------------------
struct FBinnedThreadCache_ {
    STATIC_CONST_INTEGRAL(size_t, FreePagesMax, 32); // <=> 2 mo cache per thread (32 * 64 * 1024)

    static THREAD_LOCAL FBinnedThreadCache_ GInstanceTLS;

    ~FBinnedThreadCache_() {
#ifdef WITH_CORE_ASSERT
        // check for leaks :
        for (FBinnedChunk_* p : _buckets)
            Assert(nullptr == p);
#endif
        // release pages in cache
        while (FBinnedPage_* p = _freePages.PopHead()) {
            Assert(_freePagesCount);
            FBinnedPage_::Release(p);
            _freePagesCount--;
        }
    }

    void* Allocate(size_t sizeInBytes) {
        typedef FBinnedChunk_::list_type list_type;

        const size_t sizeClass = FBinnedChunk_::MakeClass(sizeInBytes);
        Assert(sizeClass < lengthof(_buckets));

        FBinnedChunk_* chunk = _buckets[sizeClass];
        for (; chunk; chunk = chunk->_node.Next) {
            if (void* p = (void*)chunk->Allocate()) {
                // poke the chunk with free blocks first if it's not already
                if (chunk != _buckets[sizeClass] && chunk->_freeCount)
                    list_type::Poke(&_buckets[sizeClass], nullptr, chunk);
                return p;
            }
            Assert(0 == chunk->_freeCount);
        }

        FBinnedPage_* const page = AllocPage_();
        ONLY_IF_ASSERT(FillBlockUninitialized_(page, FBinnedPage_::PageSize));

        chunk = new ((void*)page) FBinnedChunk_(sizeClass);
        list_type::PushFront(&_buckets[sizeClass], nullptr, chunk);
        void* p = (void*)chunk->Allocate();
        Assert(p);

        return p;
    }

    void Release(void* p) {
        typedef FBinnedChunk_::list_type list_type;

        Assert(p);
        FBinnedChunk_::FBlock* block = (FBinnedChunk_::FBlock*)p;
        FBinnedChunk_* chunk = block->Owner();

        // Check that the block if freed from the same thread
        Assert(list_type::Contains(_buckets[chunk->_class], chunk));

        chunk->Release(block);

        Assert(chunk->_freeCount);
        if (chunk->empty()) {
            list_type::Erase(&_buckets[chunk->_class], nullptr, chunk);
            ONLY_IF_ASSERT(chunk->~FBinnedChunk_());
            ONLY_IF_ASSERT(FillBlockDeleted_(chunk, FBinnedChunk_::ChunkSize));
            ReleasePage_((FBinnedPage_*)chunk);
        }
        else if (chunk != _buckets[chunk->_class]) {
            // poke the chunk with free blocks first if it's not already
            list_type::Poke(&_buckets[chunk->_class], nullptr, chunk);
        }
    }

private:
    FBinnedChunk_* _buckets[FBinnedChunk_::NumClasses] = { 0 };
    INTRUSIVESINGLELIST(&FBinnedPage_::Node) _freePages;
    size_t _freePagesCount;

    FBinnedThreadCache_() : _freePagesCount(0) {}

    FBinnedPage_* AllocPage_() {
        if (FBinnedPage_* p = _freePages.PopHead()) {
            Assert(_freePagesCount);
            _freePagesCount--;
            return p;
        }
        else {
            return FBinnedGlobalCache_::GInstance.AllocPage();
        }
    }

    void ReleasePage_(FBinnedPage_* page) {
        Likely(_freePagesCount != FreePagesMax);
        if (_freePagesCount == FreePagesMax) {
            FBinnedGlobalCache_::GInstance.ReleasePage(page);
        }
        else {
            Assert(_freePagesCount < FreePagesMax);
            _freePages.PushFront(page);
            _freePagesCount++;
        }
    }
};
THREAD_LOCAL FBinnedThreadCache_ FBinnedThreadCache_::GInstanceTLS;
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedAllocator_ {
    STATIC_CONST_INTEGRAL(size_t, VMCacheBlocks,        32);
    STATIC_CONST_INTEGRAL(size_t, VMCacheSizeInBytes,   16*1024*1024); // <=> 16 mo global cache for large blocks

    static FBinnedAllocator_ GInstance;

    FORCE_INLINE static void* Allocate(size_t sizeInBytes) {
        Likely(sizeInBytes <= FBinnedChunk_::MaxSize);
        if (0 == sizeInBytes)
            return nullptr;
        else if (sizeInBytes <= FBinnedChunk_::MaxSize)
            return FBinnedThreadCache_::GInstanceTLS.Allocate(sizeInBytes);
        else
            return GInstance.AllocLargeBlock_(sizeInBytes);
    }

    FORCE_INLINE static void Release(void* p) {
        Likely(not IS_ALIGNED(FBinnedChunk_::ChunkSize, p));
        if (nullptr == p)
            return;
        else if (not IS_ALIGNED(FBinnedChunk_::ChunkSize, p))
            FBinnedThreadCache_::GInstanceTLS.Release(p);
        else
            GInstance.ReleaseLargeBlock_(p);
    }

    FORCE_INLINE static size_t RegionSize(void* p) {
        Likely(not IS_ALIGNED(FBinnedChunk_::ChunkSize, p));
        if (nullptr == p)
            return 0;
        else if (not IS_ALIGNED(FBinnedChunk_::ChunkSize, p))
            return ((FBinnedChunk_::FBlock*)p)->Owner()->BlockSizeInBytes();
        else
            return FVirtualMemory::AllocSizeInBytes(p);
    }

private:
    FAtomicSpinLock _barrier;
    TVirtualMemoryCache<VMCacheBlocks, VMCacheSizeInBytes> _vm;

    FBinnedAllocator_() {
        STATIC_ASSERT(FBinnedPage_::PageSize == FBinnedChunk_::ChunkSize);
        STATIC_ASSERT(FBinnedPage_::PageSize == 64 * 1024);
    }

    void* AllocLargeBlock_(size_t sizeInBytes) {
        sizeInBytes = ROUND_TO_NEXT_64K(sizeInBytes);
        const FAtomicSpinLock::FScope scopeLock(_barrier);
        return _vm.Allocate(sizeInBytes);
    }

    void ReleaseLargeBlock_(void* p) {
        const FAtomicSpinLock::FScope scopeLock(_barrier);
        _vm.Free(p);
    }
};
FBinnedAllocator_ FBinnedAllocator_::GInstance;
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* FMallocBinned::Malloc(size_t size) {
    return FBinnedAllocator_::Allocate(size);
}
//----------------------------------------------------------------------------
void FMallocBinned::Free(void* ptr) {
    return FBinnedAllocator_::Release(ptr);
}
//----------------------------------------------------------------------------
void* FMallocBinned::Realloc(void* ptr, size_t size) {
    void* const result = FBinnedAllocator_::Allocate(size);

    if (ptr) {
        const size_t old = FBinnedAllocator_::RegionSize(ptr);

        // Skip if the same block can be reused :
        if (FBinnedChunk_::MakeClass(old) == FBinnedChunk_::MakeClass(size))
            return ptr;

        if (const size_t cpy = Min(old, size)) {
            Assert(result);
            ::memcpy(result, ptr, cpy);
        }

        FBinnedAllocator_::Release(ptr);
    }

    return result;
}
//----------------------------------------------------------------------------
void* FMallocBinned::AlignedMalloc(size_t size, size_t alignment) {
    Assert(IS_POW2(alignment));
    return alignment--, FBinnedAllocator_::Allocate((size + alignment) & ~alignment);
}
//----------------------------------------------------------------------------
void FMallocBinned::AlignedFree(void* ptr) {
    return FBinnedAllocator_::Release(ptr);
}
//----------------------------------------------------------------------------
void* FMallocBinned::AlignedRealloc(void* ptr, size_t size, size_t alignment) {
    Assert(IS_POW2(alignment));
    return alignment--, FMallocBinned::Realloc(ptr, (size + alignment) & ~alignment);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
