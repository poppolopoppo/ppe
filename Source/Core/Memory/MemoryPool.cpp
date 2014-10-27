#include "stdafx.h"

#include "MemoryPool.h"

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MemoryPoolChunk::MemoryPoolChunk(MemoryPoolChunk *nextChunk /* = nullptr */)
: _blockUsed(0), _blockAdded(0), _blocks(nullptr), _nextChunk(nextChunk) {}
//----------------------------------------------------------------------------
MemoryPoolChunk::~MemoryPoolChunk() {
    AssertRelease(0 == _blockUsed);
}
//----------------------------------------------------------------------------
void *MemoryPoolChunk::AllocateBlock(size_t blockSize, size_t blockCount) {
    Assert(BlockAvailable(blockCount));
    Assert(_blockUsed < blockCount);

    void *block = nullptr;

    if (_blocks) {
        block = _blocks;
        _blocks = _blocks->Next;
    }
    else {
        Assert(blockCount > _blockAdded);
        block = Storage() + _blockAdded++ * blockSize;
    }

    Assert(block);
    ++_blockUsed;

    return block;
}
//----------------------------------------------------------------------------
void MemoryPoolChunk::ReleaseBlock(void *ptr, size_t blockSize, size_t blockCount) {
    Assert(ptr);
    Assert(!CompletelyFree());
    Assert(Contains(ptr, blockSize, blockCount));
    Assert(_blockUsed);

    Block *const block = reinterpret_cast<Block *>(ptr);

    block->Next = _blocks;
    _blocks = block;

    --_blockUsed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MemoryPoolBase::MemoryPoolBase(size_t blockSize, size_t chunkSize)
:   _blockSize(checked_cast<u32>(blockSize)),
    _chunkSize(checked_cast<u32>(chunkSize)),
    _chunks(nullptr) {
    Assert(blockSize);
    Assert(chunkSize);
    Assert(BlockCountPerChunk() > 50); // sanity check
    Assert(blockSize >= sizeof(MemoryPoolChunk::Block));

    LOG(Information, L"[POOL] New pool with block size = {0}, chunk size = {1}, blocks per chunk = {2}",
        _blockSize, SizeInBytes{ _chunkSize }, BlockCountPerChunk());
}
//----------------------------------------------------------------------------
MemoryPoolBase::~MemoryPoolBase() {
    AssertRelease(nullptr == _chunks);

    LOG(Information, L"[POOL] Delete pool with block size = {0}, chunk size = {1}, blocks per chunk = {2}",
        _blockSize, SizeInBytes{ _chunkSize }, BlockCountPerChunk());
}
//----------------------------------------------------------------------------
void MemoryPoolBase::AddChunk(MemoryPoolChunk *chunk) {
    Assert(chunk);
    Assert(nullptr == chunk->Next());

    chunk->SetNext(_chunks);
    _chunks = chunk;
}
//----------------------------------------------------------------------------
void *MemoryPoolBase::TryAllocate_FailIfNoBlockAvailable() {
    const size_t blockCount = BlockCountPerChunk();

    MemoryPoolChunk *chunk = _chunks;
    while (chunk)
        if (chunk->BlockAvailable(blockCount))
            return chunk->AllocateBlock(_blockSize, blockCount);
        else
            chunk = chunk->Next();

    return nullptr;
}
//----------------------------------------------------------------------------
MemoryPoolChunk *MemoryPoolBase::Deallocate_ReturnChunkToRelease(void *ptr) {
    Assert(_chunks);

    const size_t blockCount = BlockCountPerChunk();

    MemoryPoolChunk *prev = nullptr;
    MemoryPoolChunk *chunk = _chunks;
    while (chunk) {
        if (chunk->Contains(ptr, _blockSize, blockCount)) {
            chunk->ReleaseBlock(ptr, _blockSize, blockCount);

            if (!chunk->CompletelyFree())
                return nullptr;

            if (nullptr == prev && nullptr == chunk->Next())
                return nullptr; // keep at least one chunk alive (if not manually cleared)

            if (nullptr == prev)
                _chunks = chunk->Next();
            else
                prev->SetNext(chunk->Next());

            Assert(chunk->CompletelyFree());
            chunk->SetNext(nullptr); // more paranoid than anything

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
    return chunk;
}
//----------------------------------------------------------------------------
MemoryPoolChunk *MemoryPoolBase::ClearOneChunk_IgnoreLeaks() {
    if (nullptr == _chunks)
        return nullptr;

    MemoryPoolChunk *const chunk = _chunks;

    _chunks = _chunks->Next();
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

    if (prev && chunk)
        prev->SetNext(chunk->Next());

    return chunk;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
