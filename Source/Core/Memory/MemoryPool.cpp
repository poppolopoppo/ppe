#include "stdafx.h"

#include "MemoryPool.h"

#include "Allocator/VirtualMemory.h"
#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"

#include <mutex>

// Uncomment to disable pool allocation (useful for memory debugging) :
//#define WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC //%__NOCOMMIT%

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FMemoryPoolAllocator_ {
public:
    static FMemoryPoolAllocator_& Instance() {
        ONE_TIME_INITIALIZE(FMemoryPoolAllocator_, GInstance, );
        return GInstance;
    }

    void* Allocate(size_t sizeInBytes) {
        const std::unique_lock<std::mutex> scopeLock(_barrier);
        return VM.Allocate(sizeInBytes);
    }

    void Free(void* ptr, size_t sizeInBytes) {
        const std::unique_lock<std::mutex> scopeLock(_barrier);
        VM.Free(ptr, sizeInBytes);
    }

private:
    FMemoryPoolAllocator_() {}
    FMemoryPoolAllocator_(const FMemoryPoolAllocator_&) = delete;
    FMemoryPoolAllocator_& operator=(const FMemoryPoolAllocator_&) = delete;

    std::mutex _barrier;
    VIRTUALMEMORYCACHE(MemoryPool, 32, 16 * 1024 * 1024) VM; // 32 entries caches with max 16 mo
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMemoryPoolChunk::FMemoryPoolChunk(size_t chunkSize, size_t blockCount)
:   _blocks(nullptr)
,   _blockCount(checked_cast<u32>(blockCount))
,   _blockUsed(0)
,   _blockAdded(0)
,   _chunkSize(chunkSize) {
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
:   _chunkCount(0)
,   _usedSize(0)
,   _totalSize(0)
,   _currentChunksize(minChunkSize)
,   _blockSize(blockSize)
,   _minChunkSize(minChunkSize)
,   _maxChunkSize(maxChunkSize) {
    Assert(blockSize >= sizeof(FMemoryPoolChunk::FBlock));
    Assert(_maxChunkSize >= minChunkSize);

    _currentChunksize = _minChunkSize;
    while (11*_blockSize > _currentChunksize)
        _currentChunksize *= 2;
    AssertRelease(_currentChunksize <= _maxChunkSize);

    LOG(Info,
        L"[Pool] New pool with block size = {0}, {1} = {2} per chunk",
        _blockSize,
        FSizeInBytes{ _currentChunksize },
        BlockCountPerChunk_(_currentChunksize) );
}
//----------------------------------------------------------------------------
FMemoryPool::~FMemoryPool() {
    Clear_AssertCompletelyFree();

    Assert(0 == _usedSize);
    Assert(0 == _totalSize);
    Assert(0 == _chunkCount);

    LOG(Info,
        L"[Pool] Delete pool with block size = {0}, {1} = {2} per chunk",
        _blockSize,
        FSizeInBytes{ _currentChunksize },
        BlockCountPerChunk_(_currentChunksize) );
}
//----------------------------------------------------------------------------
void FMemoryPool::GrowChunkSizeIFP_() {
    const size_t nextChunkSize = _currentChunksize * 2;
    if (nextChunkSize <= _maxChunkSize) {
        _currentChunksize = nextChunkSize;

        LOG(Info,
            L"[Pool] Grow pool with block size = {0}, {3} used pages, {1} = {2} per chunk ({4}/{5})",
            _blockSize,
            FSizeInBytes{ _currentChunksize },
            BlockCountPerChunk_(_currentChunksize),
            _chunkCount,
            FSizeInBytes{ _usedSize },
            FSizeInBytes{ _totalSize });
    }
}
//----------------------------------------------------------------------------
void FMemoryPool::ResetChunkSize_() {
    _currentChunksize = _minChunkSize;
}
//----------------------------------------------------------------------------
void FMemoryPool::AddChunk_(FMemoryPoolChunk *chunk) {
    Assert(chunk);
    Assert(_spares.empty());

    _chunks.PushFront(chunk);

    _chunkCount++;
    _totalSize += chunk->ChunkSize();
}
//----------------------------------------------------------------------------
void *FMemoryPool::TryAllocate_FailIfNoBlockAvailable_() {
    FMemoryPoolChunk *chunk = _chunks.Head();

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

        _chunks.Poke(chunk);

        return chunk->AllocateBlock(_blockSize);
    }
}
//----------------------------------------------------------------------------
FMemoryPoolChunk *FMemoryPool::Deallocate_ReturnChunkToRelease_(void *ptr) {
    Assert(false == _chunks.empty());

    FMemoryPoolChunk *chunk = _chunks.Head();
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
                    (not _spares.empty()) &&
                    // size heuristic to decide if we give up on the empty chunk :
                    (_totalSize - _spares.Head()->ChunkSize()) >= 2*_usedSize)
                ? ReleaseChunk_()
                : nullptr;
        }

        chunk = _chunks.Next(chunk);
    }

    AssertNotReached(); // ptr doesn't belong to this pool
    return nullptr;
}
//----------------------------------------------------------------------------
FMemoryPoolChunk *FMemoryPool::ClearOneChunk_AssertCompletelyFree_() {
    FMemoryPoolChunk* const last = _chunks.PopHead();

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
    FMemoryPoolChunk* const last = _chunks.PopHead();

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
    if (_chunks.Head() && _chunks.Head()->CompletelyFree())
        SpareChunk_(_chunks.Head());

    return ReleaseChunk_();
}
//----------------------------------------------------------------------------
void FMemoryPool::SpareChunk_(FMemoryPoolChunk *chunk) {
    Assert(chunk);
    Assert(not _chunks.empty());

    Assert(_totalSize >= chunk->ChunkSize());
    Assert(_usedSize <= _totalSize);

    _chunks.Erase(chunk);
    _spares.Insert(chunk, [](const FMemoryPoolChunk& lhs, const FMemoryPoolChunk& rhs) {
        return lhs.ChunkSize() < rhs.ChunkSize();
    });
}
//----------------------------------------------------------------------------
FMemoryPoolChunk *FMemoryPool::ReviveChunk_() {
    FMemoryPoolChunk* const revive = _spares.PopTail();

    if (nullptr == revive) {
        return nullptr;
    }
    else {
        Assert(revive->CompletelyFree());
        _chunks.PushFront(revive);
        return revive;
    }
}
//----------------------------------------------------------------------------
FMemoryPoolChunk *FMemoryPool::ReleaseChunk_() {
    FMemoryPoolChunk* const release = _spares.PopHead();

    if (nullptr == release) {
        return nullptr;
    }
    else {
        Assert(0 < _chunkCount);
        Assert(_totalSize >= release->ChunkSize());
        Assert(_usedSize + release->ChunkSize() <= _totalSize);

        _chunkCount--;
        _totalSize -= release->ChunkSize();

        LOG(Info,
            L"[Pool] Release chunk with block size = {0}, page size = {4} ({5} blocs), {3} remaining pages, {1} = {2} per chunk ({6:f2}%)",
            _blockSize,
            FSizeInBytes{ _currentChunksize },
            BlockCountPerChunk_(_currentChunksize),
            _chunkCount,
            FSizeInBytes{ release->ChunkSize() },
            BlockCountPerChunk_(release->ChunkSize()),
            (100.0f*_usedSize)/_totalSize );

        return release;
    }
}
//----------------------------------------------------------------------------
void* FMemoryPool::Allocate(FMemoryTrackingData *trackingData /* = nullptr */) {
#ifndef USE_MEMORY_DOMAINS
    UNUSED(trackingData);
#endif
#ifdef WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC
    if (trackingData)
        trackingData->Allocate(1, BlockSize());

    return Core::aligned_malloc(BlockSize(), 16);

#else
    
    void *ptr = TryAllocate_FailIfNoBlockAvailable_();
    if (nullptr == ptr) {

        if (_chunks.Head())
            GrowChunkSizeIFP_();

        FMemoryPoolChunk *const newChunk = AllocateChunk_();
        AddChunk_(newChunk);

        ptr = TryAllocate_FailIfNoBlockAvailable_();
        Assert(ptr);

#ifdef USE_MEMORY_DOMAINS
        if (trackingData && trackingData->Parent())
            trackingData->Parent()->Pool_AllocateOneChunk(newChunk->ChunkSize());
#endif
    }

#ifdef USE_MEMORY_DOMAINS
    if (trackingData)
        trackingData->Pool_AllocateOneBlock(BlockSize());
#endif

    return ptr;

#endif
}
//----------------------------------------------------------------------------
void FMemoryPool::Deallocate(void *ptr, FMemoryTrackingData *trackingData /* = nullptr */) {
#ifndef USE_MEMORY_DOMAINS
    UNUSED(trackingData);
#endif
#ifdef WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC
    Core::aligned_free(ptr);

    if (trackingData)
        trackingData->Deallocate(1, BlockSize());

#else
    Assert(ptr);

#ifdef USE_MEMORY_DOMAINS
    if (trackingData)
        trackingData->Pool_DeallocateOneBlock(BlockSize());
#endif

    FMemoryPoolChunk *chunk;
    if (nullptr == (chunk = Deallocate_ReturnChunkToRelease_(ptr)))
        return;

#ifdef USE_MEMORY_DOMAINS
    if (trackingData && trackingData->Parent())
        trackingData->Parent()->Pool_DeallocateOneChunk(chunk->ChunkSize());
#endif

    DeallocateChunk_(chunk);

#endif
}
//----------------------------------------------------------------------------
void FMemoryPool::Clear_AssertCompletelyFree() {
#ifdef WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC
    AssertRelease(nullptr == Chunks());

#else
    FMemoryPoolChunk *chunk;
    while (nullptr != (chunk = ClearOneChunk_AssertCompletelyFree_())) {
        DeallocateChunk_(chunk);
    }

#endif
}
//----------------------------------------------------------------------------
void FMemoryPool::Clear_IgnoreLeaks() {
#ifdef WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC
    AssertRelease(nullptr == Chunks());

#else
    FMemoryPoolChunk *chunk;
    while (nullptr != (chunk = ClearOneChunk_IgnoreLeaks_())) {
        DeallocateChunk_(chunk);
    }

#endif
}
//----------------------------------------------------------------------------
void FMemoryPool::Clear_UnusedMemory() {
#ifdef WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC
    AssertRelease(nullptr == Chunks());

#else
    FMemoryPoolChunk *chunk;
    while (nullptr != (chunk = ClearOneChunk_UnusedMemory_())) {
        DeallocateChunk_(chunk);
    }

#endif
}
//----------------------------------------------------------------------------
FMemoryPoolChunk* FMemoryPool::AllocateChunk_() {
#ifdef WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC
    AssertNotReached();
#else
    const size_t currentChunkSize = CurrentChunkSize();
    const size_t currentBlockCount = BlockCountPerChunk_(currentChunkSize);
    void* const storage = FMemoryPoolAllocator_::Instance().Allocate(currentChunkSize);
    return new (storage) FMemoryPoolChunk(currentChunkSize, currentBlockCount);
#endif
}
//----------------------------------------------------------------------------
void FMemoryPool::DeallocateChunk_(FMemoryPoolChunk *chunk) {
#ifdef WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC
    AssertNotReached();

#else
    Assert(chunk);
    const size_t chunkSize = chunk->ChunkSize();
    chunk->~FMemoryPoolChunk();
    FMemoryPoolAllocator_::Instance().Free(chunk, chunkSize);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMemoryPoolList::FMemoryPoolList() {}
//----------------------------------------------------------------------------
FMemoryPoolList::~FMemoryPoolList() {
    while (nullptr != _pools.Head())
        checked_delete(_pools.Head());
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
} //!namespace Core
