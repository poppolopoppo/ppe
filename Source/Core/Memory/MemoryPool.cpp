#include "stdafx.h"

#include "MemoryPool.h"

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MemoryPoolChunk::MemoryPoolChunk(size_t chunkSize, size_t blockCount, MemoryPoolChunk *nextChunk /* = nullptr */)
:   _blocks(nullptr)
,   _blockCount(checked_cast<u32>(blockCount))
,   _blockUsed(0)
,   _blockAdded(0)
,   _chunkSize(chunkSize) {
    Assert(blockCount > 10);
}
//----------------------------------------------------------------------------
MemoryPoolChunk::~MemoryPoolChunk() {
    AssertRelease(0 == _blockUsed);
    Assert(nullptr == _node.Next);
    Assert(nullptr == _node.Prev);
}
//----------------------------------------------------------------------------
void *MemoryPoolChunk::AllocateBlock(size_t blockSize) {
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
void MemoryPoolChunk::ReleaseBlock(void *ptr, size_t blockSize) {
    Assert(ptr);
    Assert(!CompletelyFree());
    Assert(Contains(ptr, blockSize));
    Assert(_blockUsed);

    Block *const block = reinterpret_cast<Block *>(ptr);

    block->Next = _blocks;
    _blocks = block;

    --_blockUsed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MemoryPoolBase::MemoryPoolBase(size_t blockSize, size_t minChunkSize, size_t maxChunkSize)
:   _chunkCount(0)
,   _usedSize(0)
,   _totalSize(0)
,   _currentChunksize(minChunkSize)
,   _blockSize(blockSize)
,   _minChunkSize(minChunkSize)
,   _maxChunkSize(maxChunkSize) {
    Assert(blockSize >= sizeof(MemoryPoolChunk::Block));
    Assert(_maxChunkSize >= minChunkSize);

    _currentChunksize = _minChunkSize;
    while (11*_blockSize > _currentChunksize)
        _currentChunksize *= 2;
    AssertRelease(_currentChunksize <= _maxChunkSize);

    LOG(Info,
        L"[Pool] New pool with block size = {0}, {1} = {2} per chunk",
        _blockSize,
        SizeInBytes{ _currentChunksize },
        BlockCountPerChunk(_currentChunksize) );
}
//----------------------------------------------------------------------------
MemoryPoolBase::~MemoryPoolBase() {
    Assert(0 == _usedSize);
    Assert(0 == _totalSize);
    Assert(0 == _chunkCount);
    Assert(nullptr == _node.Next);
    Assert(nullptr == _node.Prev);

    LOG(Info,
        L"[Pool] Delete pool with block size = {0}, {1} = {2} per chunk",
        _blockSize,
        SizeInBytes{ _currentChunksize },
        BlockCountPerChunk(_currentChunksize) );
}
//----------------------------------------------------------------------------
void MemoryPoolBase::GrowChunkSizeIFP() {
    const size_t nextChunkSize = _currentChunksize * 2;
    if (nextChunkSize <= _maxChunkSize) {
        _currentChunksize = nextChunkSize;

        LOG(Info,
            L"[Pool] Grow pool with block size = {0}, {3} used pages, {1} = {2} per chunk ({4}/{5})",
            _blockSize,
            SizeInBytes{ _currentChunksize },
            BlockCountPerChunk(_currentChunksize),
            _chunkCount,
            SizeInBytes{ _usedSize },
            SizeInBytes{ _totalSize });
    }
}
//----------------------------------------------------------------------------
void MemoryPoolBase::ResetChunkSize() {
    _currentChunksize = _minChunkSize;
}
//----------------------------------------------------------------------------
void MemoryPoolBase::AddChunk(MemoryPoolChunk *chunk) {
    Assert(chunk);
    Assert(_spares.empty());

    _chunks.PushFront(chunk);

    _chunkCount++;
    _totalSize += chunk->ChunkSize();
}
//----------------------------------------------------------------------------
void *MemoryPoolBase::TryAllocate_FailIfNoBlockAvailable() {
    MemoryPoolChunk *chunk = _chunks.Head();

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
MemoryPoolChunk *MemoryPoolBase::Deallocate_ReturnChunkToRelease(void *ptr) {
    Assert(false == _chunks.empty());

    MemoryPoolChunk *chunk = _chunks.Head();
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
MemoryPoolChunk *MemoryPoolBase::ClearOneChunk_AssertCompletelyFree() {
    MemoryPoolChunk* const last = _chunks.PopHead();

    if (nullptr == last) {
        return ReleaseChunk_();
    }
    else {
        AssertRelease(last->CompletelyFree());

        SpareChunk_(last);

        MemoryPoolChunk* const release = ReleaseChunk_();
        Assert(release);
        Assert(release->CompletelyFree());

        return release;
    }
}
//----------------------------------------------------------------------------
MemoryPoolChunk *MemoryPoolBase::ClearOneChunk_IgnoreLeaks() {
    MemoryPoolChunk* const last = _chunks.PopHead();

    if (nullptr == last) {
        return ReleaseChunk_();
    }
    else {
        SpareChunk_(last);

        MemoryPoolChunk* const release = ReleaseChunk_();
        Assert(release);

        return release;
    }
}
//----------------------------------------------------------------------------
MemoryPoolChunk *MemoryPoolBase::ClearOneChunk_UnusedMemory() {
    if (_chunks.Head() && _chunks.Head()->CompletelyFree())
        SpareChunk_(_chunks.Head());

    return ReleaseChunk_();
}
//----------------------------------------------------------------------------
void MemoryPoolBase::SpareChunk_(MemoryPoolChunk *chunk) {
    Assert(chunk);
    Assert(not _chunks.empty());

    Assert(_totalSize >= chunk->ChunkSize());
    Assert(_usedSize <= _totalSize);

    _chunks.Erase(chunk);
    _spares.Insert(chunk, [](const MemoryPoolChunk& lhs, const MemoryPoolChunk& rhs) {
        return lhs.ChunkSize() < rhs.ChunkSize();
    });
}
//----------------------------------------------------------------------------
MemoryPoolChunk *MemoryPoolBase::ReviveChunk_() {
    MemoryPoolChunk* const revive = _spares.PopTail();

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
MemoryPoolChunk *MemoryPoolBase::ReleaseChunk_() {
    MemoryPoolChunk* const release = _spares.PopHead();

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
            SizeInBytes{ _currentChunksize },
            BlockCountPerChunk(_currentChunksize),
            _chunkCount,
            SizeInBytes{ release->ChunkSize() },
            BlockCountPerChunk(release->ChunkSize()),
            (100.0f*_usedSize)/_totalSize );

        return release;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MemoryPoolBaseList::MemoryPoolBaseList() {}
//----------------------------------------------------------------------------
MemoryPoolBaseList::~MemoryPoolBaseList() {
    while (nullptr != _pools.Head())
        checked_delete(_pools.Head());
    Assert(_pools.empty());
}
//----------------------------------------------------------------------------
void MemoryPoolBaseList::Insert(MemoryPoolBase* ppool) {
    Assert(ppool);
    const AtomicSpinLock::Scope scopeLock(_barrier);
    _pools.PushFront(ppool);
}
//----------------------------------------------------------------------------
void MemoryPoolBaseList::Remove(MemoryPoolBase* ppool) {
    Assert(ppool);
    const AtomicSpinLock::Scope scopeLock(_barrier);
    _pools.Erase(ppool);
}
//----------------------------------------------------------------------------
void MemoryPoolBaseList::ClearAll_AssertCompletelyFree() {
    const AtomicSpinLock::Scope scopeLock(_barrier);
    for (MemoryPoolBase* phead = _pools.Head(); phead; phead = phead->Node().Next )
        phead->Clear_AssertCompletelyFree();
}
//----------------------------------------------------------------------------
void MemoryPoolBaseList::ClearAll_IgnoreLeaks() {
    const AtomicSpinLock::Scope scopeLock(_barrier);
    for (MemoryPoolBase* phead = _pools.Head(); phead; phead = phead->Node().Next )
        phead->Clear_IgnoreLeaks();
}
//----------------------------------------------------------------------------
void MemoryPoolBaseList::ClearAll_UnusedMemory() {
    const AtomicSpinLock::Scope scopeLock(_barrier);
    for (MemoryPoolBase* phead = _pools.Head(); phead; phead = phead->Node().Next )
        phead->Clear_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
