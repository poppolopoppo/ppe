#include "stdafx.h"

#include "MallocBinned.h"

#include "Container/IntrusiveList.h"
#include "Container/Stack.h"
#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Memory/VirtualMemory.h"
#include "Misc/TargetPlatform.h"
#include "Thread/AtomicSpinLock.h"
#include "Thread/ThreadContext.h"

#ifdef WITH_CORE_ASSERT
#   include "Diagnostic/Callstack.h"
#   include "Diagnostic/DecodedCallstack.h"
#endif

#if (defined(WITH_CORE_ASSERT) || USE_CORE_MEMORY_DEBUGGING)
#   define USE_MALLOCBINNED_PAGE_PROTECT    1// Crash when using a cached block
#else
#   define USE_MALLOCBINNED_PAGE_PROTECT    0// Crash when using a cached block
#endif

PRAGMA_INITSEG_COMPILER

namespace Core {
LOG_CATEGORY(CORE_API, MallocBinned)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_ASSERT(64 == CACHELINE_SIZE);
//----------------------------------------------------------------------------
struct FBinnedPage_;
struct FBinnedChunk_;
struct FBinnedGlobalCache_;
struct FBinnedThreadCache_;
struct FBinnedAllocator_;
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
static FORCE_INLINE void FillBlockDeleted_(void* , size_t ) {}
static void FillBlockPending_(void* p, size_t size) { ::memset(p, 0xAA, size); }
static FORCE_INLINE void FillBlockUninitialized_(void* , size_t ) {}
static FORCE_INLINE void FillBlockPage_(void* , size_t , size_t ) {}
#   endif
#endif
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedPage_ {
    STATIC_CONST_INTEGRAL(size_t, PageSize, ALLOCATION_GRANULARITY); // 64 kb

    FBinnedPage_(const FBinnedPage_&) = delete;
    FBinnedPage_& operator =(const FBinnedPage_&) = delete;

#if USE_MALLOCBINNED_PAGE_PROTECT
    void ProtectPage() { FVirtualMemory::Protect(this, PageSize, false, false); }
    void UnprotectPage() { FVirtualMemory::Protect(this, PageSize, true, true); }
#endif

    static FBinnedPage_* Allocate() {
#if USE_CORE_MEMORYDOMAINS
        auto* p = (FBinnedPage_*)FVirtualMemory::InternalAlloc(PageSize, MEMORYDOMAIN_TRACKING_DATA(SmallTables));
#else
        auto* p = (FBinnedPage_*)FVirtualMemory::InternalAlloc(PageSize);
#endif
        Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, p));
        return p;
    }

    static void Release(FBinnedPage_* p) {
        Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, p));
#if USE_CORE_MEMORYDOMAINS
        FVirtualMemory::InternalFree(p, PageSize, MEMORYDOMAIN_TRACKING_DATA(SmallTables));
#else
        FVirtualMemory::InternalFree(p, PageSize);
#endif
    }
};
//----------------------------------------------------------------------------
struct FBinnedThreadCache_;
struct CACHELINE_ALIGNED FBinnedChunk_ {
    STATIC_CONST_INTEGRAL(size_t, Alignment,        ALLOCATION_BOUNDARY);
    STATIC_CONST_INTEGRAL(size_t, MinSizeInBytes,   ALLOCATION_BOUNDARY);
    STATIC_CONST_INTEGRAL(size_t, MaxSizeInBytes,   32736);
    STATIC_CONST_INTEGRAL(size_t, ChunkSizeInBytes, FBinnedPage_::PageSize);
    STATIC_CONST_INTEGRAL(size_t, ChunkSizeMask,    ~(ChunkSizeInBytes - 1));
    STATIC_CONST_INTEGRAL(size_t, ChunkAvailable,   ChunkSizeInBytes - CACHELINE_SIZE);

    STATIC_ASSERT(Meta::IsAligned(Alignment, MinSizeInBytes));
    STATIC_ASSERT(Meta::IsAligned(Alignment, MaxSizeInBytes));
    STATIC_ASSERT(Meta::IsAligned(Alignment, ChunkSizeInBytes));
    STATIC_ASSERT(Meta::IsAligned(Alignment, ChunkAvailable));

    struct FBlock {
        FBlock* Next;

#ifdef WITH_CORE_ASSERT
#   ifdef ARCH_X86
        STATIC_CONST_INTEGRAL(size_t, Canary, 0xCDCDCDCD);
#   else
        STATIC_CONST_INTEGRAL(size_t, Canary, 0xCDCDCDCDCDCDCDCDull);
#   endif
        size_t _Canary;
        void MakeCanary() { _Canary = Canary; }
        bool TestCanary() const { return (Canary == _Canary); }
#endif

        FBinnedChunk_* Owner() const {
            FBinnedChunk_* owner = (FBinnedChunk_*)((uintptr_t(this) & ChunkSizeMask));
            Assert_NoAssume(owner->CheckCanaries_());
            return owner;
        }
    };
    STATIC_ASSERT(sizeof(FBlock) <= MinSizeInBytes);

    FBinnedChunk_(const FBinnedChunk_&) = delete;
    FBinnedChunk_& operator =(const FBinnedChunk_&) = delete;

    ~FBinnedChunk_() {
        Assert_NoAssume(CheckCanaries_());
        Assert_NoAssume(CheckThreadSafety_());
    }

    size_t Class() const { return _class; }
    size_t BlockSizeInBytes() const { return GClassesSize[_class]; }
    size_t BlockTotalCount() const { return _blockTotalCount; }
    size_t BlockAllocatedCount() const { return _allocatedCount; }

    FBinnedThreadCache_* ThreadCache() const { return _threadCache; }

    bool IsCompletelyFree() const { return (0 == _allocatedCount); }
    bool HasFreeBlock() const { return (_allocatedCount < _blockTotalCount); }

    FORCE_INLINE FBlock* Allocate() {
        Assert_NoAssume(CheckCanaries_());
        Assert_NoAssume(CheckThreadSafety_());
        Assert(HasFreeBlock());

        FBlock* p;
        if (_freeBlock) {
            Assert_NoAssume(_freeBlock->TestCanary());

            p = _freeBlock;
            _freeBlock = p->Next;
        }
        else {
            Assert(_mappedCount < _blockTotalCount);

            p = BlockAt_(_mappedCount++);
        }

        ++_allocatedCount;

        Assert(Meta::IsAligned(Alignment, p));
        ONLY_IF_ASSERT(FillBlockUninitialized_(p, GClassesSize[_class]));

        return p;
    }

    FORCE_INLINE void Release(FBlock* p) {
        Assert_NoAssume(CheckCanaries_());
        Assert_NoAssume(CheckThreadSafety_());
        Assert(not IsCompletelyFree());

        Assert_NoAssume(p->TestCanary());
        Assert(Meta::IsAligned(Alignment, p));

        ONLY_IF_ASSERT(FillBlockDeleted_(p, GClassesSize[_class]));
        ONLY_IF_ASSERT(p->MakeCanary());

        Assert_NoAssume(!_freeBlock || _freeBlock->TestCanary());

        p->Next = _freeBlock;
        _freeBlock = p;
        --_allocatedCount;
    }

    void TakeOwn(FBinnedThreadCache_* newCache) {
        Assert_NoAssume(CheckCanaries_());

        _threadCache = newCache;

#ifdef WITH_CORE_ASSERT
        if (nullptr == newCache)
            Assert_NoAssume(CheckThreadSafety_());
        else
            _threadId = std::this_thread::get_id();
#endif
    }

    static constexpr size_t NumClasses = 45;

    static constexpr u16 GClassesSize[NumClasses] = {
        16,     0,      0,      0,      32,     0,
        48,     0,      64,     80,     96,     112,
        128,    160,    192,    224,    256,    320,
        384,    448,    512,    640,    768,    896,
        1024,   1280,   1536,   1792,   2048,   2560,
        3072,   3584,   4096,   5120,   6144,   7168,
        8192,   10240,  12288,  14336,  16384,  20480,
        24576,  28672,  32736,
    };

    FORCE_INLINE static size_t MakeClass(size_t size) {
        constexpr size_t POW_N = 2;
        constexpr size_t MinClassIndex = 19;
        size = ROUND_TO_NEXT_16(size);
        const size_t index = Meta::FloorLog2((size - 1) | 1);
        return ((index << POW_N) + ((size - 1) >> (index - POW_N)) - MinClassIndex);
    }

private:
    friend struct FBinnedGlobalCache_;
    friend struct FBinnedThreadCache_;

    FBinnedChunk_(FBinnedThreadCache_* threadCache, size_t class_)
        : _class(checked_cast<u32>(class_))
        , _blockTotalCount(ChunkAvailable / GClassesSize[_class])
        , _allocatedCount(0)
        , _mappedCount(0)
        , _freeBlock(nullptr)
        , _threadCache(threadCache) {
        Assert(_threadCache);
        Assert(GClassesSize[_class] > 0);
        Assert(Meta::IsAligned(ChunkSizeInBytes, this));
        Assert((u8*)BlockAt_(0) >= (u8*)(this + 1));
        Assert((u8*)BlockAt_(_blockTotalCount - 1) + BlockSizeInBytes() <= (u8*)this + ChunkSizeInBytes);

        ONLY_IF_ASSERT(FillBlockUninitialized_((u8*)this + sizeof(*this), CACHELINE_SIZE - sizeof(*this)));
        ONLY_IF_ASSERT(FillBlockPage_(BlockAt_(0), _blockTotalCount, BlockSizeInBytes()));
    }

    FORCE_INLINE FBlock* BlockAt_(size_t index) const {
        Assert(index < _blockTotalCount);

#if 1 // blocks in chunk are located in such way to achieve a maximum possible alignment
        FBlock* const pblock = ((FBlock*)((u8*)this + ChunkSizeInBytes - (_blockTotalCount - index) * BlockSizeInBytes()));
#else
        FBlock* const pblock = ((FBlock*)((u8*)(this + 1) + index * BlockSizeInBytes()));
#endif
        Assert((u8*)pblock >= (u8*)(this + 1));
        Assert((u8*)pblock + BlockSizeInBytes() <= (u8*)this + ChunkSizeInBytes);

        return pblock;
    }

#ifdef WITH_CORE_ASSERT
    STATIC_CONST_INTEGRAL(u32, Canary0, 0xA0FACADEul);
    const u32 _canary0 = Canary0;
#endif

    const u32 _class;
    const u32 _blockTotalCount;

    u32 _allocatedCount;
    u32 _mappedCount;

    FBlock* _freeBlock;

    TIntrusiveListNode<FBinnedChunk_> _node;
    typedef INTRUSIVELIST_ACCESSOR(&FBinnedChunk_::_node) list_type;

    FBinnedThreadCache_* _threadCache;

#ifdef WITH_CORE_ASSERT
    std::thread::id _threadId = std::this_thread::get_id();

    STATIC_CONST_INTEGRAL(u32, Canary1, 0xBAADF00Dul);
    const u32 _canary1 = Canary1;

    bool CheckCanaries_() const { return (_canary0 == Canary0 && _canary1 == Canary1); }
    bool CheckThreadSafety_() const { return (std::this_thread::get_id() == _threadId); }
#endif
};
STATIC_ASSERT(sizeof(FBinnedChunk_) == CACHELINE_SIZE);
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedGlobalCache_ {
    STATIC_CONST_INTEGRAL(size_t, FreePagesMax, 128); // <=> 8 mo global cache (128 * 64 * 1024), 1k table

    static FBinnedGlobalCache_& Instance() {
        static FBinnedGlobalCache_ GInstance;
        return GInstance;
    }

    FBinnedGlobalCache_(const FBinnedGlobalCache_&) = delete;
    FBinnedGlobalCache_& operator =(const FBinnedGlobalCache_&) = delete;

    NO_INLINE ~FBinnedGlobalCache_() {
        // release dangling chunks, report potentially leaked blocks
        const FAtomicSpinLock::FScope scopeLock(_barrier);

        INTRUSIVELIST(&FBinnedChunk_::_node) chunksToRelease = _danglingChunks;
        _danglingChunks.Clear();

        while (FBinnedChunk_* chunk = chunksToRelease.PopHead()) {
            Assert(chunk->_threadCache == nullptr);

            if (chunk->IsCompletelyFree()) {
                ONLY_IF_ASSERT(chunk->~FBinnedChunk_());
                ONLY_IF_ASSERT(FillBlockDeleted_(chunk, FBinnedChunk_::ChunkSizeInBytes));

                FBinnedPage_::Release((FBinnedPage_*)chunk);
            }
            else {
                // keep leaking blocks alive to avoid crashing
                _danglingChunks.PushFront(chunk);
            }
        }

        // release pages in cache
        FBinnedPage_* page;
        while (_globalFreePages.Pop(&page))
            FBinnedPage_::Release(page);

        // will not try to cache next pages (in case of very late deallocation in another static)
        _globalFreePages.ForbidFurtherAccess();
    }

    FBinnedPage_* AllocPage() {
        if (Likely(_globalFreePages.size())) {
            const FAtomicSpinLock::FScope scopeLock(_barrier);

            FBinnedPage_* page;
            if (_globalFreePages.Pop(&page)) {
                Assert(page);
#if USE_MALLOCBINNED_PAGE_PROTECT
                page->UnprotectPage();
#endif
                return page;
            };
        }

        return FBinnedPage_::Allocate();
    }

    void ReleasePage(FBinnedPage_* page) {
        Assert(page);
        Assert(not _globalFreePages.Contains(page)); // not thread safe but ok for assert (read only)

        if (Likely(not _globalFreePages.full())) {
            const FAtomicSpinLock::FScope scopeLock(_barrier);

            if (Likely(not _globalFreePages.full())) {
#if USE_MALLOCBINNED_PAGE_PROTECT
                page->ProtectPage();
#endif
                _globalFreePages.Push(page);
                return;
            }
        }

        FBinnedPage_::Release(page);
    }

    NO_INLINE void RegisterDanglingChunk(FBinnedChunk_* chunk) {
        Assert(chunk);
        Assert(chunk->BlockAllocatedCount() > 0);

        const FAtomicSpinLock::FScope scopeLock(_barrier);

        Assert(not _danglingChunks.Contains(chunk));

        chunk->TakeOwn(nullptr);
        _danglingChunks.PushFront(chunk);
    }

    NO_INLINE bool StealDanglingChunk(FBinnedChunk_* chunk, FBinnedThreadCache_* newCache) {
        Assert(chunk);
        Assert(chunk->BlockAllocatedCount() > 0);
        Assert(newCache);

        const FAtomicSpinLock::FScope scopeLock(_barrier);

        if (chunk->ThreadCache())
            return false;

        Assert(_danglingChunks.Contains(chunk));

        chunk->TakeOwn(newCache);
        _danglingChunks.Erase(chunk);

        return true;
    }

private:
    FAtomicSpinLock _barrier;
    INTRUSIVELIST(&FBinnedChunk_::_node) _danglingChunks;
    TFixedSizeStack<FBinnedPage_*, FreePagesMax> _globalFreePages;

    FBinnedGlobalCache_() {}
};
//----------------------------------------------------------------------------
struct FBinnedThreadCache_ {
    STATIC_CONST_INTEGRAL(size_t, FreePagesMax, 16); // <=> 1 mo cache per thread (16 * 64 * 1024)

    static FBinnedThreadCache_& InstanceTLS() {
        static THREAD_LOCAL FBinnedThreadCache_ GInstanceTLS;
        return GInstanceTLS;
    }

    FBinnedThreadCache_(const FBinnedThreadCache_&) = delete;
    FBinnedThreadCache_& operator =(const FBinnedThreadCache_&) = delete;

    void* Allocate(size_t sizeInBytes) {
        const size_t sizeClass = FBinnedChunk_::MakeClass(sizeInBytes);
        Assert(sizeClass < lengthof(_buckets));
        Assert(sizeInBytes <= FBinnedChunk_::GClassesSize[sizeClass]);

        auto& bucket = _buckets[sizeClass];

        // chunks with free blocks are always at head, so if it failed here we need a new page
        FBinnedChunk_* chunk = bucket.Head();
        Assert(!chunk || this == chunk->_threadCache);

        if (Likely(chunk) && chunk->HasFreeBlock()) {
            // allocate from the head chunk
            void* const p = chunk->Allocate();

            // push empty chunks to the tail of the bucket list
            // if there is still free space it should be at head after that
            if (not chunk->HasFreeBlock())
                bucket.PokeTail(chunk);

            return p;
        }

        return AllocateFromNewChunk_(sizeClass);
    }

    void Release(void* p) {
        Assert(p);
        FBinnedChunk_::FBlock* block = (FBinnedChunk_::FBlock*)p;
        ONLY_IF_ASSERT(block->MakeCanary());
        FBinnedChunk_* chunk = block->Owner();

        // register the block for deletion in another thread
        if (Unlikely(this != chunk->_threadCache) && TryStealDanglingBlock_(chunk, block))
            return;

        // check if the block is freed from the correct thread
        Assert(this == chunk->_threadCache);
        Assert(_buckets[chunk->_class].Contains(chunk));

        chunk->Release(block);
        Assert(chunk->HasFreeBlock());

        if (chunk->IsCompletelyFree())
            ReleaseChunk_(chunk);
        else // poke chunk to head, meaning there's always a free block at head IFP
            _buckets[chunk->_class].PokeFront(chunk);
    }

    void RegisterPendingBlock(FBinnedChunk_::FBlock* block) {
        Assert(block);
        Assert_NoAssume(block->TestCanary());
        Assert(this == block->Owner()->_threadCache);

        const FAtomicSpinLock::FScope scopeLock(_pending.Barrier);

        _pending.NumBlocks++;

        ONLY_IF_ASSERT(FillBlockPending_(block, block->Owner()->BlockSizeInBytes()));
        ONLY_IF_ASSERT(block->MakeCanary());

        block->Next = _pending.FirstBlock;
        _pending.FirstBlock = block;
    }

    bool ReleasePendingBlocks() {
        if (0 == _pending.NumBlocks)
            return false;

        // synchronize to release all pending blocks
        const FAtomicSpinLock::FScope scopeLock(_pending.Barrier);

        while (_pending.FirstBlock) {
            Assert(_pending.NumBlocks--);
            Assert_NoAssume(_pending.FirstBlock->TestCanary());
            Assert(this == _pending.FirstBlock->Owner()->_threadCache);

            FBinnedChunk_::FBlock* next = _pending.FirstBlock->Next;

            Release(_pending.FirstBlock);

            _pending.FirstBlock = next;
        }

        Assert(0 == _pending.NumBlocks);
        _pending.NumBlocks = 0;

        return true;
    }

    NO_INLINE ~FBinnedThreadCache_() {
        LOG(MallocBinned, Debug, L"shutdown thread cache {0}", std::this_thread::get_id());

        FBinnedGlobalCache_& globalCache = FBinnedGlobalCache_::Instance();

        // thread safety while releasing pending blocks, alive deallocations will need to still the chunk
        {
            const FAtomicSpinLock::FScope scopeLock(_pending.Barrier);

            // release pending blocks before checking for leaks :
            while (_pending.FirstBlock) {
                Assert_NoAssume(_pending.FirstBlock->TestCanary());
                Assert(this == _pending.FirstBlock->Owner()->_threadCache);

                FBinnedChunk_::FBlock* next = _pending.FirstBlock->Next;

                Release(_pending.FirstBlock);

                _pending.FirstBlock = next;
            }

            // look for chunks still alive
            forrange(sizeClass, 0, FBinnedChunk_::NumClasses) {
                while (FBinnedChunk_* b = _buckets[sizeClass].PopHead()) {
                    // detach the chunk from this thread
                    globalCache.RegisterDanglingChunk(b);
                }
            }
        }

        // release pages in local cache
        FBinnedPage_* page;
        while (_localFreePages.Pop(&page))
            globalCache.ReleasePage(page);

        // will not try to cache next pages (in case of very late deallocation in another static)
        _localFreePages.ForbidFurtherAccess();
    }

private:
    struct CACHELINE_ALIGNED/* avoid false sharing */ FPendingBlocks_ {
        std::atomic<size_t> NumBlocks{ 0 };
        FAtomicSpinLock Barrier;
        FBinnedChunk_::FBlock* FirstBlock = nullptr;
    };

    FPendingBlocks_ _pending;
    INTRUSIVELIST(&FBinnedChunk_::_node) _buckets[FBinnedChunk_::NumClasses];
    TFixedSizeStack<FBinnedPage_*, FreePagesMax> _localFreePages;

    FBinnedThreadCache_() {
        LOG(MallocBinned, Debug, L"start thread cache {0}", std::this_thread::get_id());
    }

    // force NO_INLINE for cold path functions (better chance for inlining, better instruction cache)

    NO_INLINE void* AllocateFromNewChunk_(size_t sizeClass) {
        // try to release pending blocks to get free memory before reserving a new page
        if (ReleasePendingBlocks())
            return Allocate(FBinnedChunk_::GClassesSize[sizeClass]);

        FBinnedPage_* page;
        if (_localFreePages.Pop(&page)) {
            Assert(page);
#if USE_MALLOCBINNED_PAGE_PROTECT
            page->UnprotectPage();
#endif
        }
        else {
            page = FBinnedGlobalCache_::Instance().AllocPage();
        }

        // register new page at head of buckets
        auto* chunk = INPLACE_NEW(page, FBinnedChunk_)(this, sizeClass);
        _buckets[sizeClass].PushFront(chunk);

        return chunk->Allocate();
    }

    NO_INLINE void ReleaseChunk_(FBinnedChunk_* chunk) {
        // unregister the chunk from buckets
        _buckets[chunk->_class].Erase(chunk);
        ONLY_IF_ASSERT(chunk->~FBinnedChunk_());
        ONLY_IF_ASSERT(FillBlockDeleted_(chunk, FBinnedChunk_::ChunkSizeInBytes));

        // release page to local or global cache
        auto* page = (FBinnedPage_*)chunk;

        Assert(not _localFreePages.Contains(page));

        if (_localFreePages.full()) {
            FBinnedGlobalCache_::Instance().ReleasePage(page);
        }
        else {
#if USE_MALLOCBINNED_PAGE_PROTECT
            page->ProtectPage();
#endif
            _localFreePages.Push(page);
        }
    }

    NO_INLINE bool TryStealDanglingBlock_(FBinnedChunk_* chunk, FBinnedChunk_::FBlock* block) {
        if (FBinnedGlobalCache_::Instance().StealDanglingChunk(chunk, this)) {
            _buckets[chunk->_class].PushFront(chunk);
            return false;
        }
        else {
            Assert(chunk->_threadCache);
            chunk->_threadCache->RegisterPendingBlock(block);
            return true;
        }
    }
};
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedAllocator_ {
    STATIC_CONST_INTEGRAL(size_t, VMCacheBlocks, 32);
    STATIC_CONST_INTEGRAL(size_t, VMCacheSizeInBytes, 16*1024*1024); // <=> 16 mo global cache for large blocks

    static FBinnedAllocator_& Instance() {
        static FBinnedAllocator_ GInstance;
        return GInstance;
    }

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
        if (Unlikely(0 == sizeInBytes))
            return nullptr;
        else if (Likely(sizeInBytes <= FBinnedChunk_::MaxSizeInBytes))
            return FBinnedThreadCache_::InstanceTLS().Allocate(sizeInBytes);
        else
            return Instance().AllocLargeBlock_(sizeInBytes);
    }

    FORCE_INLINE static void Release(void* p) {
        ONLY_IF_ASSERT(const FCheckReentrancy reentrancy);
        if (Unlikely(Meta::IsAligned(FBinnedChunk_::ChunkSizeInBytes, p)))
            Instance().ReleaseLargeBlock_(p);
        else
            FBinnedThreadCache_::InstanceTLS().Release(p);
    }

    FORCE_INLINE static size_t SnapSize(size_t sizeInBytes) {
        STATIC_ASSERT(ALLOCATION_GRANULARITY == 64<<10);
        return (sizeInBytes <= FBinnedChunk_::MaxSizeInBytes
            ? FBinnedChunk_::GClassesSize[FBinnedChunk_::MakeClass(sizeInBytes)]
            : ROUND_TO_NEXT_64K(sizeInBytes) );
    }

    FORCE_INLINE static size_t RegionSize(void* p) {
        if (Unlikely(nullptr == p))
            return 0;
        else if (Likely(not Meta::IsAligned(FBinnedChunk_::ChunkSizeInBytes, p)))
            return ((FBinnedChunk_::FBlock*)p)->Owner()->BlockSizeInBytes();
        else
            return FVirtualMemory::SizeInBytes(p);
    }

private:
    FAtomicSpinLock _barrier;
    VIRTUALMEMORYCACHE(LargeBlocks, VMCacheBlocks, VMCacheSizeInBytes) _vm;

    FBinnedAllocator_() {
        STATIC_ASSERT(FBinnedPage_::PageSize == FBinnedChunk_::ChunkSizeInBytes);
        LOG(MallocBinned, Debug, L"start allocator");
    }

    ~FBinnedAllocator_() {
        LOG(MallocBinned, Debug, L"shutdown allocator");
    }

    NO_INLINE void* AllocLargeBlock_(size_t sizeInBytes) {
        STATIC_ASSERT(FBinnedPage_::PageSize == 64 * 1024);
        sizeInBytes = ROUND_TO_NEXT_64K(sizeInBytes);
        const FAtomicSpinLock::FScope scopeLock(_barrier);
        return _vm.Allocate(sizeInBytes);
    }

    NO_INLINE void ReleaseLargeBlock_(void* p) {
        if (nullptr == p)
            return;

        const FAtomicSpinLock::FScope scopeLock(_barrier);
        _vm.Free(p);
    }
};
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
    if (Likely(ptr)) {
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
    return FBinnedAllocator_::Allocate(Meta::RoundToNext(size, alignment));
}
//----------------------------------------------------------------------------
void FMallocBinned::AlignedFree(void* ptr) {
    return FBinnedAllocator_::Release(ptr);
}
//----------------------------------------------------------------------------
void* FMallocBinned::AlignedRealloc(void* ptr, size_t size, size_t alignment) {
    return Realloc(ptr, Meta::RoundToNext(size, alignment));
}
//----------------------------------------------------------------------------
void FMallocBinned::ReleasePendingBlocks() {
    FBinnedThreadCache_::InstanceTLS().ReleasePendingBlocks();
}
//----------------------------------------------------------------------------
size_t FMallocBinned::SnapSize(size_t size) {
    return FBinnedAllocator_::SnapSize(size);
}
//----------------------------------------------------------------------------
size_t FMallocBinned::RegionSize(void* ptr) {
    Assert(ptr);
    return FBinnedAllocator_::RegionSize(ptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

