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
,   _currentChunksize(minChunkSize)
,   _chunkCount(0)
,   _blockSize(blockSize)
,   _minChunkSize(minChunkSize)
,   _maxChunkSize(maxChunkSize) {
    Assert(blockSize >= sizeof(MemoryPoolChunk::Block));
    Assert(_maxChunkSize >= minChunkSize);

    _currentChunksize = _minChunkSize;
    while (10*_blockSize > _currentChunksize)
        _currentChunksize *= 2;
    AssertRelease(_currentChunksize <= _maxChunkSize);

    LOG(Info,
        L"[Pool] New pool with block size = {0}\n"
        L" - Min chunk size = {1}\n"
        L" - Max chunk size = {2}\n"
        L" - Current chunk size = {3}\n"
        L" - Current blocks per chunk = {4}",
        _blockSize,
        SizeInBytes{ _minChunkSize }, SizeInBytes{ _maxChunkSize }, SizeInBytes{ _currentChunksize },
        BlockCountPerChunk(_currentChunksize) );
}
//----------------------------------------------------------------------------
MemoryPoolBase::~MemoryPoolBase() {
    AssertRelease(nullptr == _chunks);
    Assert(nullptr == _node.Next);
    Assert(nullptr == _node.Prev);

    LOG(Info,
        L"[Pool] Delete pool with block size = {0}\n"
        L" - Min chunk size = {1}\n"
        L" - Max chunk size = {2}\n"
        L" - Current chunk size = {3}\n"
        L" - Current blocks per chunk = {4}",
        _blockSize,
        SizeInBytes{ _minChunkSize }, SizeInBytes{ _maxChunkSize }, SizeInBytes{ _currentChunksize },
        BlockCountPerChunk(_currentChunksize) );
}
//----------------------------------------------------------------------------
void MemoryPoolBase::GrowChunkSizeIFP() {
    const size_t nextChunkSize = _currentChunksize * 2;
    if (nextChunkSize <= _maxChunkSize) {
        _currentChunksize = nextChunkSize;

        LOG(Info,
            L"[Pool] Grow pool with block size = {0}\n"
            L" - Min chunk size = {1}\n"
            L" - Max chunk size = {2}\n"
            L" - Current chunk size = {3}\n"
            L" - Current blocks per chunk = {4}\n"
            L" - Chunk count = {5}",
            _blockSize,
            SizeInBytes{ _minChunkSize }, SizeInBytes{ _maxChunkSize }, SizeInBytes{ _currentChunksize },
            BlockCountPerChunk(_currentChunksize),
            _chunkCount );
    }
}
//----------------------------------------------------------------------------
void MemoryPoolBase::ResetChunkSize() {
    _currentChunksize = _minChunkSize;
}
//----------------------------------------------------------------------------
void MemoryPoolBase::AddChunk(MemoryPoolChunk *chunk) {
    Assert(chunk);
    Assert(nullptr == chunk->Next());

    chunk->SetNext(_chunks);
    _chunks = chunk;

    ++_chunkCount;
}
//----------------------------------------------------------------------------
void *MemoryPoolBase::TryAllocate_FailIfNoBlockAvailable() {
    MemoryPoolChunk *chunk = _chunks;
    while (chunk)
        if (chunk->BlockAvailable())
            return chunk->AllocateBlock(_blockSize);
        else
            chunk = chunk->Next();

    return nullptr;
}
//----------------------------------------------------------------------------
MemoryPoolChunk *MemoryPoolBase::Deallocate_ReturnChunkToRelease(void *ptr) {
    Assert(_chunks);

    MemoryPoolChunk *prev = nullptr;
    MemoryPoolChunk *chunk = _chunks;
    while (chunk) {
        if (chunk->Contains(ptr, _blockSize)) {
            chunk->ReleaseBlock(ptr, _blockSize);

            if (false == chunk->CompletelyFree())
                return nullptr;

            if (nullptr == prev && nullptr == chunk->Next())
                return nullptr; // keep at least one chunk alive (if not manually cleared)

            if (nullptr == prev)
                _chunks = chunk->Next();
            else
                prev->SetNext(chunk->Next());

            Assert(chunk->CompletelyFree());
            chunk->SetNext(nullptr); // more paranoid than anything

            Assert(_chunkCount);
            --_chunkCount;

            return chunk;
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
        return nullptr;

    MemoryPoolChunk *const chunk = _chunks;
    AssertRelease(chunk->CompletelyFree());
    Assert(nullptr == chunk->Next()); // logically ...

    _chunks = _chunks->Next();

    Assert(_chunkCount);
    --_chunkCount;

    return chunk;
}
//----------------------------------------------------------------------------
MemoryPoolChunk *MemoryPoolBase::ClearOneChunk_IgnoreLeaks() {
    if (nullptr == _chunks)
        return nullptr;

    MemoryPoolChunk *const chunk = _chunks;

    _chunks = _chunks->Next();

    Assert(_chunkCount);
    --_chunkCount;

    return chunk;
}
//----------------------------------------------------------------------------
MemoryPoolChunk *MemoryPoolBase::ClearOneChunk_UnusedMemory() {
    if (nullptr == _chunks)
        return nullptr;

    MemoryPoolChunk *prev = nullptr;
    MemoryPoolChunk *chunk = _chunks;

    while (chunk && !chunk->CompletelyFree()) {
        prev = chunk;
        chunk = chunk->Next();
    }

    if (prev && chunk) {
        Assert(_chunkCount);
        --_chunkCount;
        prev->SetNext(chunk->Next());
    }

    return chunk;
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
