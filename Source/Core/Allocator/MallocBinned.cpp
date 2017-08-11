#include "stdafx.h"

#include "MallocBinned.h"

#include "VirtualMemory.h"
#include "Container/IntrusiveList.h"
#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "Misc/TargetPlatform.h"
#include "Thread/AtomicSpinLock.h"

#ifdef WITH_CORE_ASSERT
#   include "Diagnostic/DecodedCallstack.h"
#endif

PRAGMA_INITSEG_COMPILER

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_ASSERT(64 == CACHELINE_SIZE);
//----------------------------------------------------------------------------
#ifdef WITH_CORE_ASSERT
#   define WITH_MALLOCBINNED_FILLBLOCK //%_NOCOMMIT%
#   ifdef WITH_MALLOCBINNED_FILLBLOCK
static void FillBlockDeleted_(void* p, size_t size) { ::memset(p, 0xDD, size); }
static void FillBlockPending_(void* p, size_t size) { ::memset(p, 0xBB, size); }
static void FillBlockUninitialized_(void* p, size_t size) { ::memset(p, 0xCC, size); }
static void FillBlockPage_(void* p, size_t n, size_t size) {
    u8* block = (u8*)p;
    forrange(i, 0, n) {
        *(u16*)block = u16(i);
        ::memset(block + sizeof(u16), 0xAA, size - sizeof(u16));
        block += size;
    }
}
#   else
static void FillBlockDeleted_(void* , size_t ) {}
static void FillBlockPending_(void* p, size_t size) { ::memset(p, 0xAA, size); }
static void FillBlockUninitialized_(void* , size_t ) {}
static void FillBlockPage_(void* , size_t , size_t ) {}
#   endif
#endif
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedPage_ {
    STATIC_CONST_INTEGRAL(size_t, PageSize, 64 * 1024);

    FBinnedPage_(const FBinnedPage_&) = delete;
    FBinnedPage_& operator =(const FBinnedPage_&) = delete;

    TIntrusiveSingleListNode<FBinnedPage_> Node;

    FORCE_INLINE static FBinnedPage_* Allocate() {
        return (FBinnedPage_*)FVirtualMemory::AlignedAlloc(PageSize, PageSize);
    }

    FORCE_INLINE static void Release(FBinnedPage_* p) {
        FVirtualMemory::AlignedFree(p, PageSize);
    }
};
//----------------------------------------------------------------------------
struct FBinnedThreadCache_;
struct CACHELINE_ALIGNED FBinnedChunk_ {
    STATIC_CONST_INTEGRAL(size_t, Alignment,        16);
    STATIC_CONST_INTEGRAL(size_t, MinSizeInBytes,   16);
    STATIC_CONST_INTEGRAL(size_t, MaxSizeInBytes,   32736);
    STATIC_CONST_INTEGRAL(size_t, ChunkSizeInBytes, FBinnedPage_::PageSize);
    STATIC_CONST_INTEGRAL(size_t, ChunkSizeMask,    ~(ChunkSizeInBytes - 1));
    STATIC_CONST_INTEGRAL(size_t, ChunkAvailable,   ChunkSizeInBytes - CACHELINE_SIZE);

    struct FBlock {
        FBlock* Next;

#ifdef WITH_CORE_ASSERT
        STATIC_CONST_INTEGRAL(u32, Canary, 0xCDCDCDCD);
        size_t _Canary;
        void MakeCanary() { _Canary = Canary; }
        bool TestCanary() const { return (Canary == _Canary); }
#endif

        FBinnedChunk_* Owner() const {
            FBinnedChunk_* owner = (FBinnedChunk_*)((uintptr_t(this) & ChunkSizeMask));
            Assert(owner->CheckCanaries_());
            return owner;
        }
    };
    STATIC_ASSERT(sizeof(FBlock) <= MinSizeInBytes);

    FBinnedChunk_(const FBinnedChunk_&) = delete;
    FBinnedChunk_& operator =(const FBinnedChunk_&) = delete;

    ~FBinnedChunk_() {
        Assert(CheckCanaries_());
        Assert(CheckThreadSafety_());
    }

    size_t Class() const { return _class; }
    size_t BlockSizeInBytes() const { return GClassesSize_[_class]; }
    size_t BlockTotalCount() const { return (ChunkAvailable / GClassesSize_[_class]); }
    size_t BlockAllocatedCount() const { return (BlockTotalCount() - _freeCount); }

    FBinnedThreadCache_* ThreadCache() const { return _threadCache; }

    bool empty() const { return (BlockTotalCount() == _freeCount); }

    FORCE_INLINE FBlock* Allocate() {
        Assert(CheckCanaries_());
        Assert(CheckThreadSafety_());

        Likely(_freeCount);
        if (0 == _freeCount)
            return nullptr;

        FBlock* p;
        if (_freeBlock) {
            Assert(_freeBlock->TestCanary());

            p = _freeBlock;
            _freeBlock = p->Next;
        }
        else {
            Assert(_usedCount < BlockTotalCount());

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

        Assert(p->TestCanary());
        Assert(IS_ALIGNED(Alignment, p));

        ONLY_IF_ASSERT(FillBlockDeleted_(p, GClassesSize_[_class]));
        ONLY_IF_ASSERT(p->MakeCanary());

        Assert(!_freeBlock || _freeBlock->TestCanary());

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

    FBinnedChunk_(FBinnedThreadCache_* threadCache, size_t class_)
        : _class(checked_cast<u32>(class_))
        , _freeCount(checked_cast<u32>(BlockTotalCount()))
        , _usedCount(0)
        , _freeBlock(nullptr)
        , _threadCache(threadCache) {
        Assert(_threadCache);
        Assert(GClassesSize_[_class] > 0);
        Assert(IS_ALIGNED(ChunkSizeInBytes, this));
        Assert((u8*)BlockAt_(0) >= (u8*)(this + 1));
        Assert((u8*)BlockAt_(BlockTotalCount() - 1) + BlockSizeInBytes() <= (u8*)this + ChunkSizeInBytes);
        ONLY_IF_ASSERT(FillBlockPage_(BlockAt_(0), BlockTotalCount(), BlockSizeInBytes()));
    }

    FBlock* BlockAt_(size_t index) const {
        Assert(index < BlockTotalCount());

        FBlock* pblock;
#if 1
        // blocks in chunk are located in such way to achieve a maximum possible alignment
        pblock = ((FBlock*)((u8*)this + ChunkSizeInBytes - (BlockTotalCount() - index) * BlockSizeInBytes()));
#else
        pblock = ((FBlock*)((u8*)(this + 1) + index * BlockSizeInBytes()));
#endif
        Assert((u8*)pblock >= (u8*)(this + 1));
        Assert((u8*)pblock + BlockSizeInBytes() <= (u8*)this + ChunkSizeInBytes);

        return pblock;
    }

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

    FBinnedThreadCache_* const _threadCache;

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
    };
};
STATIC_ASSERT(sizeof(FBinnedChunk_) == CACHELINE_SIZE);
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedGlobalCache_ {
    STATIC_CONST_INTEGRAL(size_t, FreePagesMax, 128); // <=> 8 mo global cache (128 * 64 * 1024)

    static FBinnedGlobalCache_ GInstance;

    FBinnedGlobalCache_(const FBinnedGlobalCache_&) = delete;
    FBinnedGlobalCache_& operator =(const FBinnedGlobalCache_&) = delete;

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
#ifdef USE_MEMORY_DOMAINS
                MEMORY_DOMAIN_TRACKING_DATA(MallocBinned).Deallocate(1, FBinnedPage_::PageSize);
#endif
                return p;
            };
        }
        return FBinnedPage_::Allocate();
    }

    void ReleasePage(FBinnedPage_* page) {
        Assert(page);
        Likely(_freePagesCount < FreePagesMax);
        if (_freePagesCount != FreePagesMax) {
            Assert(_freePagesCount < FreePagesMax);

            const FAtomicSpinLock::FScope scopeLock(_barrier);

            Assert(_freePagesCount < FreePagesMax);
            _freePages.PushFront(page);
            _freePagesCount++;

#ifdef USE_MEMORY_DOMAINS
            MEMORY_DOMAIN_TRACKING_DATA(MallocBinned).Allocate(1, FBinnedPage_::PageSize);
#endif
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
    STATIC_CONST_INTEGRAL(size_t, FreePagesMax, 16); // <=> 1 mo cache per thread (16 * 64 * 1024)

    static THREAD_LOCAL FBinnedThreadCache_ GInstanceTLS;

    FBinnedThreadCache_(const FBinnedThreadCache_&) = delete;
    FBinnedThreadCache_& operator =(const FBinnedThreadCache_&) = delete;

    ~FBinnedThreadCache_() {
        // release pending blocks before checking for leaks :
        ReleasePendingBlocks();

#ifdef WITH_CORE_ASSERT
        // check for leaks :
        {
            size_t leakedNumBlocks = 0;
            size_t leakedSizeInBytes = 0;
            forrange(sizeClass, 0, FBinnedChunk_::NumClasses) {
                for (FBinnedChunk_* p = _buckets[sizeClass]; p; p = p->_node.Next) {
                    const size_t n = p->BlockAllocatedCount();
                    Assert(n);

                    leakedNumBlocks += n;
                    leakedSizeInBytes += n * p->BlockSizeInBytes();

                    forrange(i, 0, p->_usedCount) {
                        using block_type = FBinnedChunk_::FBlock;
                        block_type* usedBlock = p->BlockAt_(i);

                        bool isLeak = true;
                        for (block_type* freeBlock = p->_freeBlock; freeBlock; freeBlock = freeBlock->Next) {
                            if (freeBlock == usedBlock) {
                                isLeak = false;
                                break;
                            }
                        }

                        if (isLeak) {
                            size_t blockSizeInBytes = 0;
                            FDecodedCallstack callstack;
                            if (FetchMemoryBlockDebugInfos(usedBlock, &callstack, &blockSizeInBytes, true)) {
                                LOG(Error, L"[MallocBinned] leaked block {0} :\n{1}", FSizeInBytes{ blockSizeInBytes }, callstack);
                            }
                        }
                    }
                }
            }

            if (leakedNumBlocks) {
                LOG(Error, L"[MallocBinned] leaked {0} blocks ({1})", 
                    FCountOfElements{ leakedNumBlocks },
                    FSizeInBytes{ leakedSizeInBytes } );

                AssertNotReached();
            }
        }
#endif

        // worst case scenario, still recycle pages :
        for (FBinnedChunk_*& p : _buckets) {
            using list_type = FBinnedChunk_::list_type;
            while (FBinnedChunk_* chunk = list_type::PopHead(&p, nullptr)) {
                Assert(chunk->_threadCache == this);
                ONLY_IF_ASSERT(chunk->~FBinnedChunk_());
                ONLY_IF_ASSERT(FillBlockDeleted_(chunk, FBinnedChunk_::ChunkSizeInBytes));
                ReleasePage_((FBinnedPage_*)chunk);
            }
        }

        // release pages in cache
        while (FBinnedPage_* p = _freePages.PopHead()) {
            Assert(_freePagesCount);
            FBinnedPage_::Release(p);
            _freePagesCount--;
        }
    }

    void* Allocate(size_t sizeInBytes) {
        using list_type = FBinnedChunk_::list_type;

        const size_t sizeClass = FBinnedChunk_::MakeClass(sizeInBytes);
        Assert(sizeClass < lengthof(_buckets));
        Assert(sizeInBytes <= FBinnedChunk_::GClassesSize_[sizeClass]);

    TRY_CHUNK_ALLOCATION:
        FBinnedChunk_* chunk = _buckets[sizeClass];
        for (; chunk; chunk = chunk->_node.Next) {
            Assert(this == chunk->_threadCache);
            if (void* p = (void*)chunk->Allocate()) {
                // poke the chunk with free blocks first if it's not already
                if (chunk != _buckets[sizeClass] && chunk->_freeCount)
                    list_type::Poke(&_buckets[sizeClass], nullptr, chunk);
                return p;
            }
            Assert(0 == chunk->_freeCount);
        }

        if (ReleasePendingBlocks())
            goto TRY_CHUNK_ALLOCATION;

        FBinnedPage_* const page = AllocPage_();
        ONLY_IF_ASSERT(FillBlockUninitialized_(page, FBinnedPage_::PageSize));

        chunk = new ((void*)page) FBinnedChunk_(this, sizeClass);
        list_type::PushFront(&_buckets[sizeClass], nullptr, chunk);
        void* p = (void*)chunk->Allocate();
        Assert(p);

        return p;
    }

    void Release(void* p) {
        typedef FBinnedChunk_::list_type list_type;

        Assert(p);
        FBinnedChunk_::FBlock* block = (FBinnedChunk_::FBlock*)p;
        ONLY_IF_ASSERT(block->MakeCanary());
        FBinnedChunk_* chunk = block->Owner();

        // Register the block for deletion in another thread
        if (this != chunk->_threadCache) {
            chunk->_threadCache->RegisterPendingBlock(block);
            return;
        }

        // Check that the block if freed from the same thread
        Assert(list_type::Contains(_buckets[chunk->_class], chunk));

        chunk->Release(block);

        Assert(chunk->_freeCount);
        if (chunk->empty()) {
            list_type::Erase(&_buckets[chunk->_class], nullptr, chunk);
            Assert(chunk->_threadCache == this);
            ONLY_IF_ASSERT(chunk->~FBinnedChunk_());
            ONLY_IF_ASSERT(FillBlockDeleted_(chunk, FBinnedChunk_::ChunkSizeInBytes));
            ReleasePage_((FBinnedPage_*)chunk);
        }
        else if (chunk != _buckets[chunk->_class]) {
            // poke the chunk with free blocks first if it's not already
            list_type::Poke(&_buckets[chunk->_class], nullptr, chunk);
        }
    }

    void RegisterPendingBlock(FBinnedChunk_::FBlock* block) {
        Assert(block);
        Assert(block->TestCanary());
        Assert(this == block->Owner()->_threadCache);

        const FAtomicSpinLock::FScope scopeLock(_pendingBarrier);

        const size_t slot = _pendingBlocksCount++;
        AssertRelease(slot < PendingBlocksCapacity);
        Assert(nullptr == _pendingBlocks[slot]);

        _pendingBlocks[slot] = block;

        ONLY_IF_ASSERT(FillBlockPending_(block, block->Owner()->BlockSizeInBytes()));
        ONLY_IF_ASSERT(block->MakeCanary());
    }

    bool ReleasePendingBlocks() {
        Likely(0 == _pendingBlocks);
        if (0 == _pendingBlocksCount)
            return false;

        const FAtomicSpinLock::FScope scopeLock(_pendingBarrier);

        const size_t n = _pendingBlocksCount.exchange(0);
        forrange(i, 0, n) {
            FBinnedChunk_::FBlock*& block = _pendingBlocks[i];
            Assert(block);
            Assert(block->TestCanary());
            Assert(this == block->Owner()->_threadCache);

            Release(block);

            ONLY_IF_ASSERT(block = nullptr);
        }

        return true;
    }

private:
    FBinnedChunk_* _buckets[FBinnedChunk_::NumClasses] = { 0 };
    INTRUSIVESINGLELIST(&FBinnedPage_::Node) _freePages;
    size_t _freePagesCount;

    FAtomicSpinLock _pendingBarrier;
    std::atomic<size_t> _pendingBlocksCount;
    STATIC_CONST_INTEGRAL(size_t, PendingBlocksCapacity, 128);
    FBinnedChunk_::FBlock* _pendingBlocks[PendingBlocksCapacity] = { 0 };

    FBinnedThreadCache_() : _freePagesCount(0), _pendingBlocksCount(0) {}

    FBinnedPage_* AllocPage_() {
        if (FBinnedPage_* p = _freePages.PopHead()) {
            Assert(_freePagesCount);
            _freePagesCount--;
#ifdef USE_MEMORY_DOMAINS
            MEMORY_DOMAIN_TRACKING_DATA(MallocBinned).Deallocate(1, FBinnedPage_::PageSize);
#endif
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
#ifdef USE_MEMORY_DOMAINS
            MEMORY_DOMAIN_TRACKING_DATA(MallocBinned).Allocate(1, FBinnedPage_::PageSize);
#endif
        }
    }
};
THREAD_LOCAL FBinnedThreadCache_ FBinnedThreadCache_::GInstanceTLS;
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedAllocator_ {
    STATIC_CONST_INTEGRAL(size_t, VMCacheBlocks,        32);
    STATIC_CONST_INTEGRAL(size_t, VMCacheSizeInBytes,   16*1024*1024); // <=> 16 mo global cache for large blocks

    static FBinnedAllocator_ GInstance;
#ifdef WITH_CORE_ASSERT
    static THREAD_LOCAL bool GIsInAllocatorTLS;
    struct FCheckReentrancy {
        FCheckReentrancy() {
            Assert(!GIsInAllocatorTLS);
            GIsInAllocatorTLS = true;
        }
        ~FCheckReentrancy() {
            Assert(GIsInAllocatorTLS);
            GIsInAllocatorTLS = false;
        }
    };
#endif

    FBinnedAllocator_(const FBinnedAllocator_&) = delete;
    FBinnedAllocator_& operator =(const FBinnedAllocator_&) = delete;

    FORCE_INLINE static void* Allocate(size_t sizeInBytes) {
        ONLY_IF_ASSERT(const FCheckReentrancy reentrancy);
        Likely(sizeInBytes <= FBinnedChunk_::MaxSizeInBytes);
        if (0 == sizeInBytes)
            return nullptr;
        else if (sizeInBytes <= FBinnedChunk_::MaxSizeInBytes)
            return FBinnedThreadCache_::GInstanceTLS.Allocate(sizeInBytes);
        else
            return GInstance.AllocLargeBlock_(sizeInBytes);
    }

    FORCE_INLINE static void Release(void* p) {
        ONLY_IF_ASSERT(const FCheckReentrancy reentrancy);
        Likely(not IS_ALIGNED(FBinnedChunk_::ChunkSizeInBytes, p));
        if (nullptr == p)
            return;
        else if (not IS_ALIGNED(FBinnedChunk_::ChunkSizeInBytes, p))
            FBinnedThreadCache_::GInstanceTLS.Release(p);
        else
            GInstance.ReleaseLargeBlock_(p);
    }

    FORCE_INLINE static size_t RegionSize(void* p) {
        Likely(not IS_ALIGNED(FBinnedChunk_::ChunkSizeInBytes, p));
        if (nullptr == p)
            return 0;
        else if (not IS_ALIGNED(FBinnedChunk_::ChunkSizeInBytes, p))
            return ((FBinnedChunk_::FBlock*)p)->Owner()->BlockSizeInBytes();
        else
            return FVirtualMemory::AllocSizeInBytes(p);
    }

private:
    FAtomicSpinLock _barrier;
    VIRTUALMEMORYCACHE(MallocBinned, VMCacheBlocks, VMCacheSizeInBytes) _vm;

    FBinnedAllocator_() {
        STATIC_ASSERT(FBinnedPage_::PageSize == FBinnedChunk_::ChunkSizeInBytes);
    }

    void* AllocLargeBlock_(size_t sizeInBytes) {
        STATIC_ASSERT(FBinnedPage_::PageSize == 64 * 1024);
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
#ifdef WITH_CORE_ASSERT
THREAD_LOCAL bool FBinnedAllocator_::GIsInAllocatorTLS = false;
#endif
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
    Likely(ptr);
    if (ptr) {
        const size_t old = FBinnedAllocator_::RegionSize(ptr);

        // Skip if the same block can be reused :
        if (FBinnedChunk_::MakeClass(old) == FBinnedChunk_::MakeClass(size))
            return ptr;

        void* const result = FBinnedAllocator_::Allocate(size);

        if (const size_t cpy = Min(old, size)) {
            Assert(result);
            ::memcpy(result, ptr, cpy);
        }

        FBinnedAllocator_::Release(ptr);

        return result;
    }
    else {
        return FBinnedAllocator_::Allocate(size);
    }
}
//----------------------------------------------------------------------------
void* FMallocBinned::AlignedMalloc(size_t size, size_t alignment) {
    Assert(Meta::IsPow2(alignment));
#if 0
    return alignment--, FBinnedAllocator_::Allocate((size + alignment) & ~alignment);
#else
    // assume block is always aligned :
     void* result = FBinnedAllocator_::Allocate(size);
     Assert(IS_ALIGNED(alignment, result));
     return result;
#endif
}
//----------------------------------------------------------------------------
void FMallocBinned::AlignedFree(void* ptr) {
    return FBinnedAllocator_::Release(ptr);
}
//----------------------------------------------------------------------------
void* FMallocBinned::AlignedRealloc(void* ptr, size_t size, size_t alignment) {
    Assert(Meta::IsPow2(alignment));
#if 0
    return alignment--, FMallocBinned::Realloc(ptr, (size + alignment) & ~alignment);
#else
    void* result = Realloc(ptr, size);
    Assert(IS_ALIGNED(alignment, result));
    return result;
#endif
}
//----------------------------------------------------------------------------
void FMallocBinned::ReleasePendingBlocks() {
    FBinnedThreadCache_::GInstanceTLS.ReleasePendingBlocks();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
