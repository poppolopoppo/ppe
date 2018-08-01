#include "stdafx.h"

#include "MemoryPool.h"

#include "Allocator/Malloc.h"
#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "Memory/MemoryTracking.h"
#include "Thread/AtomicSpinLock.h"

#include "VirtualMemory.h"

//----------------------------------------------------------------------------
// Turn to 1 to disable pool allocation (useful for memory debugging) :
//----------------------------------------------------------------------------
#define USE_PPE_MEMORYPOOL_FALLBACK_TO_MALLOC (USE_PPE_MEMORY_DEBUGGING) //%_NOCOMMIT%

namespace PPE {
EXTERN_LOG_CATEGORY(PPE_API, MemoryDomain)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FMemoryPoolAllocator_ {
public:
    static FMemoryPoolAllocator_& Get() {
        ONE_TIME_INITIALIZE(FMemoryPoolAllocator_, GInstance, );
        return GInstance;
    }

    void* Allocate(size_t sizeInBytes) {
        const FAtomicSpinLock::FScope scopeLock(_barrier);
        return VM.Allocate(sizeInBytes);
    }

    void Free(void* ptr, size_t sizeInBytes) {
        const FAtomicSpinLock::FScope scopeLock(_barrier);
        VM.Free(ptr, sizeInBytes);
    }

private:
    FMemoryPoolAllocator_() {}
    FMemoryPoolAllocator_(const FMemoryPoolAllocator_&) = delete;
    FMemoryPoolAllocator_& operator=(const FMemoryPoolAllocator_&) = delete;

    FAtomicSpinLock _barrier;
    VIRTUALMEMORYCACHE(MemoryPool, 32, 16 * 1024 * 1024) VM; // 32 entries caches with max 16 mo
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // warning C4324: 'Core::FMemoryPoolChunk' : structure was padded due to __declspec(align())
class ALIGN(16) FMemoryPoolChunk {
public:
    struct FBlock { FBlock *Next; };

    explicit FMemoryPoolChunk(size_t chunkSize, size_t blockCount);
    ~FMemoryPoolChunk();

    FMemoryPoolChunk(FMemoryPoolChunk&&) = delete;
    FMemoryPoolChunk& operator =(FMemoryPoolChunk&&) = delete;

    FMemoryPoolChunk(const FMemoryPoolChunk&) = delete;
    FMemoryPoolChunk& operator =(const FMemoryPoolChunk&) = delete;

    size_t ChunkSize() const { return _chunkSize; }
    size_t BlockCount() const { return _blockCount; }

    TIntrusiveListNode<FMemoryPoolChunk>& Node() { return _node; }

    size_t BlockUsed() const { return _blockUsed; }

    char *Storage() { return reinterpret_cast<char *>(&this[1]); }
    const char *Storage() const { return reinterpret_cast<const char *>(&this[1]); }

    bool CompletelyFree() const { return (0 == _blockUsed); }
    bool BlockAvailable() const { return (_blockCount > _blockUsed); }

    bool Contains(void *block, size_t blockSize) const {
        return (Storage() <= block) && (Storage() + blockSize*_blockCount > block);
    }

    void *AllocateBlock(size_t blockSize);
    void ReleaseBlock(void *ptr, size_t blockSize);

private:
    FBlock *_blocks;

    const u32 _blockCount;
    u32 _blockUsed;
    u32 _blockAdded;

    const u32 _chunkSize;

    friend class FMemoryPool;
    TIntrusiveListNode<FMemoryPoolChunk> _node;
    typedef INTRUSIVELIST_ACCESSOR(&FMemoryPoolChunk::_node) list_accessor;
};
STATIC_ASSERT(Meta::IsAligned(ALLOCATION_BOUNDARY, sizeof(FMemoryPoolChunk)));
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
FMemoryPoolChunk::FMemoryPoolChunk(size_t chunkSize, size_t blockCount)
:   _blocks(nullptr)
,   _blockCount(checked_cast<u32>(blockCount))
,   _blockUsed(0)
,   _blockAdded(0)
,   _chunkSize(checked_cast<u32>(chunkSize)) {
    Assert(blockCount > 10);
}
//----------------------------------------------------------------------------
FMemoryPoolChunk::~FMemoryPoolChunk() {
    AssertRelease(0 == _blockUsed);
    Assert(nullptr == _node.Next);
    Assert(nullptr == _node.Prev);
}
//----------------------------------------------------------------------------
void *FMemoryPoolChunk::AllocateBlock(size_t blockSize) {
    Assert(BlockAvailable());
    Assert(_blockUsed < _blockCount);

    void *block = nullptr;

    if (_blocks) {
        block = _blocks;
        _blocks = _blocks->Next;
    }
    else {
        Assert(_blockCount > _blockAdded);
        block = Storage() + _blockAdded++ * blockSize;
    }

    Assert(block);
    ++_blockUsed;

    Assert(Contains(block, blockSize));
    return block;
}
//----------------------------------------------------------------------------
void FMemoryPoolChunk::ReleaseBlock(void *ptr, size_t blockSize) {
    UNUSED(blockSize);
    Assert(ptr);
    Assert(!CompletelyFree());
    Assert(Contains(ptr, blockSize));
    Assert(_blockUsed);

    FBlock *const block = reinterpret_cast<FBlock *>(ptr);

    block->Next = _blocks;
    _blocks = block;

    --_blockUsed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMemoryPool::FMemoryPool(size_t blockSize, size_t minChunkSize, size_t maxChunkSize)
:   _chunksHead(nullptr)
,   _chunksTail(nullptr)
,   _sparesHead(nullptr)
,   _sparesTail(nullptr)
,   _chunkCount(0)
,   _usedSize(0)
,   _totalSize(0)
,   _currentChunksize(checked_cast<u32>(minChunkSize))
,   _blockSize(checked_cast<u32>(blockSize))
,   _minChunkSize(checked_cast<u32>(minChunkSize))
,   _maxChunkSize(checked_cast<u32>(maxChunkSize)) {
    Assert(blockSize >= sizeof(FMemoryPoolChunk::FBlock));
    Assert(_maxChunkSize >= minChunkSize);

    _currentChunksize = _minChunkSize;
    while (11*_blockSize > _currentChunksize)
        _currentChunksize *= 2;
    AssertRelease(_currentChunksize <= _maxChunkSize);

    LOG(MemoryDomain, Info,
        L"new memory pool with block size = {0}, {1} = {2} per chunk",
        _blockSize,
        Fmt::FSizeInBytes{ _currentChunksize },
        BlockCountPerChunk_(_currentChunksize) );
}
//----------------------------------------------------------------------------
FMemoryPool::~FMemoryPool() {
    Clear_AssertCompletelyFree();

    Assert(0 == _usedSize);
    Assert(0 == _totalSize);
    Assert(0 == _chunkCount);

    LOG(MemoryDomain, Info,
        L"delete memory pool with block size = {0}, {1} = {2} per chunk",
        _blockSize,
        Fmt::FSizeInBytes{ _currentChunksize },
        BlockCountPerChunk_(_currentChunksize) );
}
//----------------------------------------------------------------------------
void FMemoryPool::GrowChunkSizeIFP_() {
    const size_t nextChunkSize = size_t(_currentChunksize) * 2;
    if (nextChunkSize <= _maxChunkSize) {
        _currentChunksize = checked_cast<u32>(nextChunkSize);

        LOG(MemoryDomain, Info,
            L"grow memory pool with block size = {0}, {3} used pages, {1} = {2} per chunk ({4}/{5})",
            _blockSize,
            Fmt::FSizeInBytes{ _currentChunksize },
            BlockCountPerChunk_(_currentChunksize),
            _chunkCount,
            Fmt::FSizeInBytes{ _usedSize },
            Fmt::FSizeInBytes{ _totalSize });
    }
}
//----------------------------------------------------------------------------
void FMemoryPool::ResetChunkSize_() {
    _currentChunksize = _minChunkSize;
}
//----------------------------------------------------------------------------
void FMemoryPool::AddChunk_(FMemoryPoolChunk *chunk) {
    Assert(chunk);
    Assert(nullptr == _sparesHead);

    using list_accessor = FMemoryPoolChunk::list_accessor;
    list_accessor::PushFront(&_chunksHead, &_chunksTail, chunk);

    _chunkCount++;
    _totalSize = checked_cast<u32>(_totalSize + chunk->ChunkSize());
}
//----------------------------------------------------------------------------
void *FMemoryPool::TryAllocate_FailIfNoBlockAvailable_() {
    FMemoryPoolChunk *chunk = _chunksHead;

    while (chunk) {
        if (chunk->BlockAvailable())
            break;
        else
            chunk = chunk->Node().Next;
    }

    if (nullptr == chunk)
        chunk = ReviveChunk_();

    if (nullptr == chunk) {
        return nullptr;
    }
    else {
        Assert(chunk->BlockAvailable());

        _usedSize += _blockSize;
        Assert(_totalSize >= _usedSize);

        using list_accessor = FMemoryPoolChunk::list_accessor;
        list_accessor::PokeFront(&_chunksHead, &_chunksTail, chunk);

        return chunk->AllocateBlock(_blockSize);
    }
}
//----------------------------------------------------------------------------
FMemoryPoolChunk *FMemoryPool::Deallocate_ReturnChunkToRelease_(void *ptr) {
    Assert(_chunksHead);

    FMemoryPoolChunk *chunk = _chunksHead;
    while (chunk) {
        if (chunk->Contains(ptr, _blockSize)) {
            Assert(_usedSize >= _blockSize);
            _usedSize -= _blockSize;

            chunk->ReleaseBlock(ptr, _blockSize);

            if (chunk->CompletelyFree())
                SpareChunk_(chunk);

            return (// keep at least one page in spare :
                    (_chunkCount > 1) &&
                    // if some pages are spared :
                    (_sparesHead) &&
                    // size heuristic to decide if we give up on the empty chunk :
                    (_totalSize - _sparesHead->ChunkSize()) >= 2*_usedSize)
                ? ReleaseChunk_()
                : nullptr;
        }

        chunk = chunk->_node.Next;
    }

    AssertNotReached(); // ptr doesn't belong to this pool
}
//----------------------------------------------------------------------------
FMemoryPoolChunk *FMemoryPool::ClearOneChunk_AssertCompletelyFree_() {
    using list_accessor = FMemoryPoolChunk::list_accessor;
    FMemoryPoolChunk* const last = list_accessor::PopHead(&_chunksHead, &_chunksTail);

    if (nullptr == last) {
        return ReleaseChunk_();
    }
    else {
        AssertRelease(last->CompletelyFree());

        SpareChunk_(last);

        FMemoryPoolChunk* const release = ReleaseChunk_();
        Assert(release);
        Assert(release->CompletelyFree());

        return release;
    }
}
//----------------------------------------------------------------------------
FMemoryPoolChunk *FMemoryPool::ClearOneChunk_IgnoreLeaks_() {
    using list_accessor = FMemoryPoolChunk::list_accessor;
    FMemoryPoolChunk* const last = list_accessor::PopHead(&_chunksHead, &_chunksTail);

    if (nullptr == last) {
        return ReleaseChunk_();
    }
    else {
        SpareChunk_(last);

        FMemoryPoolChunk* const release = ReleaseChunk_();
        Assert(release);

        return release;
    }
}
//----------------------------------------------------------------------------
FMemoryPoolChunk *FMemoryPool::ClearOneChunk_UnusedMemory_() {
    if (_chunksHead && _chunksHead->CompletelyFree())
        SpareChunk_(_chunksHead);

    return ReleaseChunk_();
}
//----------------------------------------------------------------------------
void FMemoryPool::SpareChunk_(FMemoryPoolChunk *chunk) {
    Assert(chunk);
    Assert(_chunksHead);

    Assert(_totalSize >= chunk->ChunkSize());
    Assert(_usedSize <= _totalSize);

    using list_accessor = FMemoryPoolChunk::list_accessor;
    list_accessor::Erase(&_chunksHead, &_chunksTail, chunk);
    list_accessor::Insert(&_sparesHead, &_sparesTail, chunk, [](const FMemoryPoolChunk& lhs, const FMemoryPoolChunk& rhs) {
        return lhs.ChunkSize() < rhs.ChunkSize();
    });
}
//----------------------------------------------------------------------------
FMemoryPoolChunk *FMemoryPool::ReviveChunk_() {
    using list_accessor = FMemoryPoolChunk::list_accessor;
    FMemoryPoolChunk* const revive = list_accessor::PopTail(&_sparesHead, &_sparesTail);

    if (nullptr == revive) {
        return nullptr;
    }
    else {
        Assert(revive->CompletelyFree());
        list_accessor::PushFront(&_chunksHead, &_chunksTail, revive);
        return revive;
    }
}
//----------------------------------------------------------------------------
FMemoryPoolChunk *FMemoryPool::ReleaseChunk_() {
    using list_accessor = FMemoryPoolChunk::list_accessor;
    FMemoryPoolChunk* const release = list_accessor::PopHead(&_sparesHead, &_sparesTail);

    if (nullptr == release) {
        return nullptr;
    }
    else {
        Assert(0 < _chunkCount);
        Assert(_totalSize >= release->ChunkSize());
        Assert(_usedSize + release->ChunkSize() <= _totalSize);

        _chunkCount--;
        _totalSize = checked_cast<u32>(_totalSize - release->ChunkSize());

        LOG(MemoryDomain, Info,
            L"release memory chunk with block size = {0}, page size = {4} ({5} blocs), {3} remaining pages, {1} = {2} per chunk ({6:f2}%)",
            _blockSize,
            Fmt::FSizeInBytes{ _currentChunksize },
            BlockCountPerChunk_(_currentChunksize),
            _chunkCount,
            Fmt::FSizeInBytes{ release->ChunkSize() },
            BlockCountPerChunk_(release->ChunkSize()),
            (100.0f*_usedSize)/_totalSize );

        return release;
    }
}
//----------------------------------------------------------------------------
void* FMemoryPool::Allocate(FMemoryTracking *trackingData /* = nullptr */) {
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();

#if !USE_PPE_MEMORYDOMAINS
    UNUSED(trackingData);
#endif
#if USE_PPE_MEMORYPOOL_FALLBACK_TO_MALLOC
    if (trackingData)
        trackingData->Allocate(1, BlockSize());

    return Core::aligned_malloc(BlockSize(), ALLOCATION_BOUNDARY);

#else

    void *ptr = TryAllocate_FailIfNoBlockAvailable_();
    if (nullptr == ptr) {

        if (_chunksHead)
            GrowChunkSizeIFP_();

        FMemoryPoolChunk *const newChunk = AllocateChunk_();
        AddChunk_(newChunk);

        ptr = TryAllocate_FailIfNoBlockAvailable_();
        Assert(ptr);

#if USE_PPE_MEMORYDOMAINS
        if (trackingData)
            trackingData->Pool_AllocateOneChunk(newChunk->ChunkSize(), newChunk->BlockCount());
#endif
    }

#if USE_PPE_MEMORYDOMAINS
    if (trackingData)
        trackingData->Pool_AllocateOneBlock(BlockSize());
#endif

    return ptr;

#endif
}
//----------------------------------------------------------------------------
void FMemoryPool::Deallocate(void *ptr, FMemoryTracking *trackingData /* = nullptr */) {
#if !USE_PPE_MEMORYDOMAINS
    UNUSED(trackingData);
#endif
#if USE_PPE_MEMORYPOOL_FALLBACK_TO_MALLOC
    Core::aligned_free(ptr);

    if (trackingData)
        trackingData->Deallocate(1, BlockSize());

#else
    Assert(ptr);

#if USE_PPE_MEMORYDOMAINS
    if (trackingData)
        trackingData->Pool_DeallocateOneBlock(BlockSize());
#endif

    FMemoryPoolChunk *chunk;
    if (nullptr == (chunk = Deallocate_ReturnChunkToRelease_(ptr)))
        return;

#if USE_PPE_MEMORYDOMAINS
    if (trackingData)
        trackingData->Pool_DeallocateOneChunk(chunk->ChunkSize(), chunk->BlockCount());
#endif

    DeallocateChunk_(chunk);

#endif
}
//----------------------------------------------------------------------------
void FMemoryPool::Clear_AssertCompletelyFree() {
#if USE_PPE_MEMORYPOOL_FALLBACK_TO_MALLOC
    AssertRelease(0 == _chunkCount);

#else
    FMemoryPoolChunk *chunk;
    while (nullptr != (chunk = ClearOneChunk_AssertCompletelyFree_())) {
        DeallocateChunk_(chunk);
    }

#endif
}
//----------------------------------------------------------------------------
void FMemoryPool::Clear_IgnoreLeaks() {
#if USE_PPE_MEMORYPOOL_FALLBACK_TO_MALLOC
    AssertRelease(0 == _chunkCount);

#else
    FMemoryPoolChunk *chunk;
    while (nullptr != (chunk = ClearOneChunk_IgnoreLeaks_())) {
        DeallocateChunk_(chunk);
    }

#endif
}
//----------------------------------------------------------------------------
void FMemoryPool::Clear_UnusedMemory() {
#if USE_PPE_MEMORYPOOL_FALLBACK_TO_MALLOC
    AssertRelease(0 == _chunkCount);

#else
    FMemoryPoolChunk *chunk;
    while (nullptr != (chunk = ClearOneChunk_UnusedMemory_())) {
        DeallocateChunk_(chunk);
    }

#endif
}
//----------------------------------------------------------------------------
size_t FMemoryPool::BlockCountPerChunk_(size_t chunkSize) const {
    return (chunkSize - sizeof(FMemoryPoolChunk)) / _blockSize;
}
//----------------------------------------------------------------------------
FMemoryPoolChunk* FMemoryPool::AllocateChunk_() {
#if USE_PPE_MEMORYPOOL_FALLBACK_TO_MALLOC
    AssertNotReached();

#else
    const size_t currentChunkSize = CurrentChunkSize();
    const size_t currentBlockCount = BlockCountPerChunk_(currentChunkSize);
    void* const storage = FMemoryPoolAllocator_::Get().Allocate(currentChunkSize);
    return INPLACE_NEW(storage, FMemoryPoolChunk)(currentChunkSize, currentBlockCount);

#endif
}
//----------------------------------------------------------------------------
void FMemoryPool::DeallocateChunk_(FMemoryPoolChunk* chunk) {
#if USE_PPE_MEMORYPOOL_FALLBACK_TO_MALLOC
    NOOP(chunk);
    AssertNotReached();

#else
    Assert(chunk);
    const size_t chunkSize = chunk->ChunkSize();
    chunk->~FMemoryPoolChunk();
    FMemoryPoolAllocator_::Get().Free(chunk, chunkSize);

#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* FMemoryPoolThreadLocal::Allocate(FMemoryTracking* trackingData/* = nullptr */) {
    THIS_THREADRESOURCE_CHECKACCESS();
    return _pool.Allocate(trackingData);
}
//----------------------------------------------------------------------------
void FMemoryPoolThreadLocal::Deallocate(void *ptr, FMemoryTracking* trackingData/* = nullptr */) {
    THIS_THREADRESOURCE_CHECKACCESS();
    _pool.Deallocate(ptr, trackingData);
}
//----------------------------------------------------------------------------
void FMemoryPoolThreadLocal::Clear_AssertCompletelyFree() {
    THIS_THREADRESOURCE_CHECKACCESS();
    _pool.Clear_AssertCompletelyFree();
}
//----------------------------------------------------------------------------
void FMemoryPoolThreadLocal::Clear_IgnoreLeaks() {
    THIS_THREADRESOURCE_CHECKACCESS();
    _pool.Clear_IgnoreLeaks();
}
//----------------------------------------------------------------------------
void FMemoryPoolThreadLocal::Clear_UnusedMemory() {
    THIS_THREADRESOURCE_CHECKACCESS();
    _pool.Clear_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* FMemoryPoolThreadSafe::Allocate(FMemoryTracking* trackingData/* = nullptr */) {
    const Meta::FLockGuard scopeLock(_barrier);
    return _pool.Allocate(trackingData);
}
//----------------------------------------------------------------------------
void FMemoryPoolThreadSafe::Deallocate(void *ptr, FMemoryTracking* trackingData/* = nullptr */) {
    const Meta::FLockGuard scopeLock(_barrier);
    _pool.Deallocate(ptr, trackingData);
}
//----------------------------------------------------------------------------
void FMemoryPoolThreadSafe::Clear_AssertCompletelyFree() {
    const Meta::FLockGuard scopeLock(_barrier);
    _pool.Clear_AssertCompletelyFree();
}
//----------------------------------------------------------------------------
void FMemoryPoolThreadSafe::Clear_IgnoreLeaks() {
    const Meta::FLockGuard scopeLock(_barrier);
    _pool.Clear_IgnoreLeaks();
}
//----------------------------------------------------------------------------
void FMemoryPoolThreadSafe::Clear_UnusedMemory() {
    const Meta::FLockGuard scopeLock(_barrier);
    _pool.Clear_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMemoryPoolList::FMemoryPoolList() {}
//----------------------------------------------------------------------------
FMemoryPoolList::~FMemoryPoolList() {
    while (const IMemoryPool* ppool = _pools.Head()) {
        checked_delete(ppool);
        Assert(ppool != _pools.Head()); // should unregister in their dtor !
    }
    Assert(_pools.empty());
}
//----------------------------------------------------------------------------
void FMemoryPoolList::Insert(IMemoryPool* ppool) {
    Assert(ppool);
    const FAtomicSpinLock::FScope scopeLock(_barrier);
    _pools.PushFront(ppool);
}
//----------------------------------------------------------------------------
void FMemoryPoolList::Remove(IMemoryPool* ppool) {
    Assert(ppool);
    const FAtomicSpinLock::FScope scopeLock(_barrier);
    _pools.Erase(ppool);
}
//----------------------------------------------------------------------------
void FMemoryPoolList::ClearAll_AssertCompletelyFree() {
    const FAtomicSpinLock::FScope scopeLock(_barrier);
    for (IMemoryPool* phead = _pools.Head(); phead; phead = phead->Node().Next )
        phead->Clear_AssertCompletelyFree();
}
//----------------------------------------------------------------------------
void FMemoryPoolList::ClearAll_IgnoreLeaks() {
    const FAtomicSpinLock::FScope scopeLock(_barrier);
    for (IMemoryPool* phead = _pools.Head(); phead; phead = phead->Node().Next )
        phead->Clear_IgnoreLeaks();
}
//----------------------------------------------------------------------------
void FMemoryPoolList::ClearAll_UnusedMemory() {
    const FAtomicSpinLock::FScope scopeLock(_barrier);
    for (IMemoryPool* phead = _pools.Head(); phead; phead = phead->Node().Next )
        phead->Clear_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
