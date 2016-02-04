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
,   _chunkSize(chunkSize)
,   _nextChunk(nextChunk) {
    Assert(blockCount > 10);
}
//----------------------------------------------------------------------------
MemoryPoolChunk::~MemoryPoolChunk() {
    AssertRelease(0 == _blockUsed);
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
:   _chunks(nullptr)
,   _spare(nullptr)
,   _chunkCount(0)
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
    AssertRelease(nullptr == _chunks);
    Assert(nullptr == _spare);
    Assert(0 == _usedSize);
    Assert(0 == _totalSize);
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
    Assert(nullptr == _spare);
    Assert(nullptr == chunk->Next());

    chunk->SetNext(_chunks);
    _chunks = chunk;
    _totalSize += chunk->ChunkSize();

    ++_chunkCount;
}
//----------------------------------------------------------------------------
void *MemoryPoolBase::TryAllocate_FailIfNoBlockAvailable() {
    MemoryPoolChunk *chunk = _chunks;
    while (chunk)
        if (chunk->BlockAvailable()) {
            _usedSize += _blockSize;
            Assert(_totalSize >= _usedSize);
            return chunk->AllocateBlock(_blockSize);
        }
        else {
            chunk = chunk->Next();
        }

    return nullptr;
}
//----------------------------------------------------------------------------
MemoryPoolChunk *MemoryPoolBase::Deallocate_ReturnChunkToRelease(void *ptr) {
    Assert(_chunks);

    MemoryPoolChunk *prev = nullptr;
    MemoryPoolChunk *chunk = _chunks;
    while (chunk) {
        if (chunk->Contains(ptr, _blockSize)) {
            Assert(_usedSize >= _blockSize);
            _usedSize -= _blockSize;

            chunk->ReleaseBlock(ptr, _blockSize);

            if (prev && chunk->CompletelyFree()) {
                prev->SetNext(chunk->Next());
                chunk->SetNext(nullptr);
                SpareChunk_(chunk);
            }

            // size heuristic to decide if we give up on the empty chunk :
            return ((nullptr != _spare) && (_totalSize - _spare->ChunkSize()) >= 2*_usedSize)
                ? ReleaseChunk_()
                : nullptr;
        }

        prev = chunk;
        chunk = chunk->Next();
    }

    AssertNotReached();
    return nullptr;
}
//----------------------------------------------------------------------------
MemoryPoolChunk *MemoryPoolBase::ClearOneChunk_AssertCompletelyFree() {
    if (nullptr == _chunks)
        return ReleaseChunk_();

    MemoryPoolChunk *const chunk = _chunks;
    AssertRelease(chunk->CompletelyFree());

    _chunks = chunk->Next();

    chunk->SetNext(nullptr);
    SpareChunk_(chunk);

    MemoryPoolChunk *const release = ReleaseChunk_();
    Assert(release);
    Assert(release->CompletelyFree());

    return release;
}
//----------------------------------------------------------------------------
MemoryPoolChunk *MemoryPoolBase::ClearOneChunk_IgnoreLeaks() {
    if (nullptr == _chunks)
        return ReleaseChunk_();

    MemoryPoolChunk *const chunk = _chunks;
    Assert(_usedSize >= chunk->BlockUsed() * _blockSize);
    _usedSize -= chunk->BlockUsed() * _blockSize;

    _chunks = chunk->Next();

    chunk->SetNext(nullptr);
    SpareChunk_(chunk);

    MemoryPoolChunk *const release = ReleaseChunk_();
    Assert(release);

    return release;
}
//----------------------------------------------------------------------------
MemoryPoolChunk *MemoryPoolBase::ClearOneChunk_UnusedMemory() {
    return ReleaseChunk_();
}
//----------------------------------------------------------------------------
void MemoryPoolBase::SpareChunk_(MemoryPoolChunk *chunk) {
    Assert(chunk);
    Assert(nullptr == chunk->Next());

    const size_t chunkSize = chunk->ChunkSize();
    Assert(_totalSize >= chunkSize);
    Assert(_usedSize <= _totalSize);

    MemoryPoolChunk* prv = nullptr;
    MemoryPoolChunk* cur = _spare;
    while (cur && cur->ChunkSize() < chunkSize) {
        prv = cur;
        cur = cur->Next();
    }

    if (prv) {
        Assert(prv->ChunkSize() <= chunkSize);
        prv->SetNext(chunk);
        chunk->SetNext(cur);
    }
    else if (cur) {
        Assert(cur->ChunkSize() >= chunkSize);
        Assert(cur == _spare);
        _spare = chunk;
        chunk->SetNext(cur);
    }
    else {
        Assert(nullptr == _spare);
        _spare = chunk;
    }
}
//----------------------------------------------------------------------------
MemoryPoolChunk *MemoryPoolBase::ReleaseChunk_() {
    if (nullptr == _spare)
        return nullptr;

    MemoryPoolChunk* release = _spare;
    _spare = _spare->Next();

    Assert(0 < _chunkCount);
    --_chunkCount;

    Assert(_totalSize >= release->ChunkSize());
    _totalSize -= release->ChunkSize();
    Assert(_usedSize <= _totalSize);



    LOG(Info,
        L"[Pool] Release chunk with block size = {0}, {4} blocs, {3} remaining pages, {1} = {2} per chunk ({5}/{6})",
        _blockSize,
        SizeInBytes{ _currentChunksize },
        BlockCountPerChunk(_currentChunksize),
        _chunkCount,
        release->ChunkSize(),
        SizeInBytes{ _usedSize },
        SizeInBytes{ _totalSize });

    Assert(nullptr == _spare || _spare->ChunkSize() >= release->ChunkSize());
    return release;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MemoryPoolBaseList::MemoryPoolBaseList() : _head(nullptr) {}
//----------------------------------------------------------------------------
MemoryPoolBaseList::~MemoryPoolBaseList() {
    while (_head)
        checked_delete(_head);
}
//----------------------------------------------------------------------------
void MemoryPoolBaseList::Insert(MemoryPoolBase* ppool) {
    const AtomicSpinLock::Scope scopeLock(_barrier);
    list_type::Queue(&_head, nullptr, ppool);
}
//----------------------------------------------------------------------------
void MemoryPoolBaseList::Remove(MemoryPoolBase* ppool) {
    const AtomicSpinLock::Scope scopeLock(_barrier);
    list_type::Deque(&_head, nullptr, ppool);
}
//----------------------------------------------------------------------------
void MemoryPoolBaseList::ClearAll_AssertCompletelyFree() {
    const AtomicSpinLock::Scope scopeLock(_barrier);
    for (MemoryPoolBase* phead = _head; phead; phead = phead->Node().Next)
        phead->Clear_AssertCompletelyFree();
}
//----------------------------------------------------------------------------
void MemoryPoolBaseList::ClearAll_IgnoreLeaks() {
    const AtomicSpinLock::Scope scopeLock(_barrier);
    for (MemoryPoolBase* phead = _head; phead; phead = phead->Node().Next)
        phead->Clear_IgnoreLeaks();
}
//----------------------------------------------------------------------------
void MemoryPoolBaseList::ClearAll_UnusedMemory() {
    const AtomicSpinLock::Scope scopeLock(_barrier);
    for (MemoryPoolBase* phead = _head; phead; phead = phead->Node().Next)
        phead->Clear_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
