#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Meta/ThreadResource.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MemoryPoolChunk {
public:
    struct Block { Block *Next; };

    explicit MemoryPoolChunk(MemoryPoolChunk *nextChunk = nullptr);
    ~MemoryPoolChunk();

    MemoryPoolChunk(MemoryPoolChunk&&) = delete;
    MemoryPoolChunk& operator =(MemoryPoolChunk&&) = delete;

    MemoryPoolChunk(const MemoryPoolChunk&) = delete;
    MemoryPoolChunk& operator =(const MemoryPoolChunk&) = delete;

    MemoryPoolChunk *Next() const { return _nextChunk; }
    void SetNext(MemoryPoolChunk *next) { _nextChunk = next; }

    size_t BlockUsed() const { return _blockUsed; }

    char *Storage() { return reinterpret_cast<char *>(&this[1]); }
    const char *Storage() const { return reinterpret_cast<const char *>(&this[1]); }

    bool CompletelyFree() const { return (0 == _blockUsed); }
    bool BlockAvailable(size_t blockCount) const { return (blockCount > _blockUsed); }

    bool Contains(void *block, size_t blockSize, size_t blockCount) const {
        return (Storage() <= block) && (Storage() + blockSize*blockCount > block);
    }

    void *AllocateBlock(size_t blockSize, size_t blockCount);
    void ReleaseBlock(void *ptr, size_t blockSize, size_t blockCount);

private:
    u32 _blockUsed;
    u32 _blockAdded;
    Block *_blocks;
    MemoryPoolChunk *_nextChunk;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MemoryPoolBase {
public:
    MemoryPoolBase(size_t blockSize, size_t chunkSize);
    virtual ~MemoryPoolBase();

    MemoryPoolBase(MemoryPoolBase&&) = delete;
    MemoryPoolBase& operator =(MemoryPoolBase&&) = delete;

    MemoryPoolBase(const MemoryPoolBase&) = delete;
    MemoryPoolBase& operator =(const MemoryPoolBase&) = delete;

    size_t BlockSize() const { return _blockSize; }
    size_t ChunkSize() const { return _chunkSize; }

    size_t BlockCountPerChunk() const { return (_chunkSize - sizeof(MemoryPoolChunk)) / _blockSize; }

    MemoryPoolChunk *Chunks() { return _chunks; }
    const MemoryPoolChunk *Chunks() const { return _chunks; }

    virtual void Clear_AssertCompletelyFree() = 0;
    virtual void Clear_IgnoreLeaks() = 0;
    virtual void Clear_UnusedMemory() = 0;

protected:
    void AddChunk(MemoryPoolChunk *chunk);

    void *TryAllocate_FailIfNoBlockAvailable();
    MemoryPoolChunk *Deallocate_ReturnChunkToRelease(void *ptr);

    MemoryPoolChunk *ClearOneChunk_AssertCompletelyFree();
    MemoryPoolChunk *ClearOneChunk_IgnoreLeaks();
    MemoryPoolChunk *ClearOneChunk_UnusedMemory();

private:
    u32 _blockSize;
    u32 _chunkSize;
    MemoryPoolChunk *_chunks;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator = ALLOCATOR(Pool, u64) >
class MemoryPool : private MemoryPoolBase, private _Allocator, private Meta::ThreadLock<_Lock> {
public:
    typedef Meta::ThreadLock<_Lock> threadlock_type;
    typedef _Allocator allocator_type;

    MemoryPool(size_t blockSize, size_t chunkSize);
    MemoryPool(size_t blockSize, size_t chunkSize, allocator_type&& allocator);
    MemoryPool(size_t blockSize, size_t chunkSize, const allocator_type& allocator);
    virtual ~MemoryPool();

    using MemoryPoolBase::BlockSize;
    using MemoryPoolBase::ChunkSize;
    using MemoryPoolBase::BlockCountPerChunk;

    void *Allocate();
    void Deallocate(void *ptr);

    virtual void Clear_AssertCompletelyFree() override;
    virtual void Clear_IgnoreLeaks() override;
    virtual void Clear_UnusedMemory() override;

private:
    MemoryPoolChunk *AllocateChunk_();
    void DeallocateChunk_(MemoryPoolChunk *chunk);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Memory/MemoryPool-inl.h"
