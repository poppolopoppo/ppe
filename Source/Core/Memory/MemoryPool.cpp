#include "stdafx.h"

#include "MemoryPool.h"

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"

namespace Core {
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
FMemoryPoolBase::FMemoryPoolBase(size_t blockSize, size_t minChunkSize, size_t maxChunkSize)
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
        BlockCountPerChunk(_currentChunksize) );
}
//----------------------------------------------------------------------------
FMemoryPoolBase::~FMemoryPoolBase() {
    Assert(0 == _usedSize);
    Assert(0 == _totalSize);
    Assert(0 == _chunkCount);
    Assert(nullptr == _node.Next);
    Assert(nullptr == _node.Prev);

    LOG(Info,
        L"[Pool] Delete pool with block size = {0}, {1} = {2} per chunk",
        _blockSize,
        FSizeInBytes{ _currentChunksize },
        BlockCountPerChunk(_currentChunksize) );
}
//----------------------------------------------------------------------------
void FMemoryPoolBase::GrowChunkSizeIFP() {
    const size_t nextChunkSize = _currentChunksize * 2;
    if (nextChunkSize <= _maxChunkSize) {
        _currentChunksize = nextChunkSize;

        LOG(Info,
            L"[Pool] Grow pool with block size = {0}, {3} used pages, {1} = {2} per chunk ({4}/{5})",
            _blockSize,
            FSizeInBytes{ _currentChunksize },
            BlockCountPerChunk(_currentChunksize),
            _chunkCount,
            FSizeInBytes{ _usedSize },
            FSizeInBytes{ _totalSize });
    }
}
//----------------------------------------------------------------------------
void FMemoryPoolBase::ResetChunkSize() {
    _currentChunksize = _minChunkSize;
}
//----------------------------------------------------------------------------
void FMemoryPoolBase::AddChunk(FMemoryPoolChunk *chunk) {
    Assert(chunk);
    Assert(_spares.empty());

    _chunks.PushFront(chunk);

    _chunkCount++;
    _totalSize += chunk->ChunkSize();
}
//----------------------------------------------------------------------------
void *FMemoryPoolBase::TryAllocate_FailIfNoBlockAvailable() {
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
FMemoryPoolChunk *FMemoryPoolBase::Deallocate_ReturnChunkToRelease(void *ptr) {
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
FMemoryPoolChunk *FMemoryPoolBase::ClearOneChunk_AssertCompletelyFree() {
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
FMemoryPoolChunk *FMemoryPoolBase::ClearOneChunk_IgnoreLeaks() {
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
FMemoryPoolChunk *FMemoryPoolBase::ClearOneChunk_UnusedMemory() {
    if (_chunks.Head() && _chunks.Head()->CompletelyFree())
        SpareChunk_(_chunks.Head());

    return ReleaseChunk_();
}
//----------------------------------------------------------------------------
void FMemoryPoolBase::SpareChunk_(FMemoryPoolChunk *chunk) {
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
FMemoryPoolChunk *FMemoryPoolBase::ReviveChunk_() {
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
FMemoryPoolChunk *FMemoryPoolBase::ReleaseChunk_() {
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
            BlockCountPerChunk(_currentChunksize),
            _chunkCount,
            FSizeInBytes{ release->ChunkSize() },
            BlockCountPerChunk(release->ChunkSize()),
            (100.0f*_usedSize)/_totalSize );

        return release;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMemoryPoolBaseList::FMemoryPoolBaseList() {}
//----------------------------------------------------------------------------
FMemoryPoolBaseList::~FMemoryPoolBaseList() {
    while (nullptr != _pools.Head())
        checked_delete(_pools.Head());
    Assert(_pools.empty());
}
//----------------------------------------------------------------------------
void FMemoryPoolBaseList::Insert(FMemoryPoolBase* ppool) {
    Assert(ppool);
    const FAtomicSpinLock::FScope scopeLock(_barrier);
    _pools.PushFront(ppool);
}
//----------------------------------------------------------------------------
void FMemoryPoolBaseList::Remove(FMemoryPoolBase* ppool) {
    Assert(ppool);
    const FAtomicSpinLock::FScope scopeLock(_barrier);
    _pools.Erase(ppool);
}
//----------------------------------------------------------------------------
void FMemoryPoolBaseList::ClearAll_AssertCompletelyFree() {
    const FAtomicSpinLock::FScope scopeLock(_barrier);
    for (FMemoryPoolBase* phead = _pools.Head(); phead; phead = phead->Node().Next )
        phead->Clear_AssertCompletelyFree();
}
//----------------------------------------------------------------------------
void FMemoryPoolBaseList::ClearAll_IgnoreLeaks() {
    const FAtomicSpinLock::FScope scopeLock(_barrier);
    for (FMemoryPoolBase* phead = _pools.Head(); phead; phead = phead->Node().Next )
        phead->Clear_IgnoreLeaks();
}
//----------------------------------------------------------------------------
void FMemoryPoolBaseList::ClearAll_UnusedMemory() {
    const FAtomicSpinLock::FScope scopeLock(_barrier);
    for (FMemoryPoolBase* phead = _pools.Head(); phead; phead = phead->Node().Next )
        phead->Clear_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
