#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Container/IntrusiveList.h"
#include "Core/Meta/ThreadResource.h"
#include "Core/Thread/AtomicSpinLock.h"

// Uncomment to disable pool allocation (useful for memory debugging) :
//#define WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC //%__NOCOMMIT%

namespace Core {
class MemoryTrackingData;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4324) // warning C4324: 'Core::MemoryPoolChunk' : structure was padded due to __declspec(align())
//----------------------------------------------------------------------------
ALIGN(16) class MemoryPoolChunk {
public:
    struct Block { Block *Next; };

    explicit MemoryPoolChunk(size_t chunkSize, size_t blockCount, MemoryPoolChunk *nextChunk = nullptr);
    ~MemoryPoolChunk();

    MemoryPoolChunk(MemoryPoolChunk&&) = delete;
    MemoryPoolChunk& operator =(MemoryPoolChunk&&) = delete;

    MemoryPoolChunk(const MemoryPoolChunk&) = delete;
    MemoryPoolChunk& operator =(const MemoryPoolChunk&) = delete;

    size_t ChunkSize() const { return _chunkSize; }
    size_t BlockCount() const { return _blockCount; }

    MemoryPoolChunk *Next() const { return _nextChunk; }
    void SetNext(MemoryPoolChunk *next) { _nextChunk = next; }

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
    Block *_blocks;

    u32 _blockCount;
    u32 _blockUsed;
    u32 _blockAdded;

    const size_t _chunkSize;
    MemoryPoolChunk *_nextChunk;
};
//----------------------------------------------------------------------------
STATIC_ASSERT(IS_ALIGNED(16, sizeof(MemoryPoolChunk)));
//----------------------------------------------------------------------------
#pragma warning(pop)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MemoryPoolBase {
public:
    friend class MemoryPoolBaseList;

    MemoryPoolBase(size_t blockSize, size_t minChunkSize, size_t maxChunkSize);
    virtual ~MemoryPoolBase();

    MemoryPoolBase(MemoryPoolBase&&) = delete;
    MemoryPoolBase& operator =(MemoryPoolBase&&) = delete;

    MemoryPoolBase(const MemoryPoolBase&) = delete;
    MemoryPoolBase& operator =(const MemoryPoolBase&) = delete;

    size_t BlockSize() const { return _blockSize; }
    size_t MinChunkSize() const { return _minChunkSize; }
    size_t MaxChunksize() const { return _maxChunkSize; }
    size_t CurrentChunkSize() const { return _currentChunksize; }
    size_t ChunkCount() const { return _chunkCount; }

    MemoryPoolChunk *Chunks() { return _chunks; }
    const MemoryPoolChunk *Chunks() const { return _chunks; }

    size_t BlockCountPerChunk(size_t chunkSize) const { return (chunkSize - sizeof(MemoryPoolChunk)) / _blockSize; }

    virtual void Clear_AssertCompletelyFree() = 0;
    virtual void Clear_IgnoreLeaks() = 0;
    virtual void Clear_UnusedMemory() = 0;

    const IntrusiveListNode<MemoryPoolBase>& Node() const { return _node; }

protected:
    void GrowChunkSizeIFP();
    void ResetChunkSize();

    void AddChunk(MemoryPoolChunk *chunk);

    void *TryAllocate_FailIfNoBlockAvailable();
    MemoryPoolChunk *Deallocate_ReturnChunkToRelease(void *ptr);

    MemoryPoolChunk *ClearOneChunk_AssertCompletelyFree();
    MemoryPoolChunk *ClearOneChunk_IgnoreLeaks();
    MemoryPoolChunk *ClearOneChunk_UnusedMemory();

private:
    MemoryPoolChunk *_chunks;
    size_t _currentChunksize;
    size_t _chunkCount;

    const size_t _blockSize;
    const size_t _minChunkSize;
    const size_t _maxChunkSize;

    IntrusiveListNode<MemoryPoolBase> _node;
};
//----------------------------------------------------------------------------
class MemoryPoolBaseList {
public:
    MemoryPoolBaseList();
    ~MemoryPoolBaseList();

    MemoryPoolBaseList(const MemoryPoolBaseList& ) = delete;
    MemoryPoolBaseList& operator =(const MemoryPoolBaseList& ) = delete;

    void Insert(MemoryPoolBase* ppool);
    void Remove(MemoryPoolBase* ppool);

    void ClearAll_AssertCompletelyFree();
    void ClearAll_IgnoreLeaks();
    void ClearAll_UnusedMemory();

private:
    typedef INTRUSIVELIST(&MemoryPoolBase::_node) list_type;

    AtomicSpinLock _barrier;
    MemoryPoolBase* _head;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator = ALLOCATOR(Pool, u64) >
class MemoryPool : private MemoryPoolBase, private _Allocator, private Meta::ThreadLock<_Lock> {
public:
    typedef Meta::ThreadLock<_Lock> threadlock_type;
    typedef _Allocator allocator_type;

    enum : bool { IsLocked = _Lock };

    MemoryPool(size_t blockSize, size_t minChunkSize, size_t maxChunkSize);
    MemoryPool(size_t blockSize, size_t minChunkSize, size_t maxChunkSize, allocator_type&& allocator);
    MemoryPool(size_t blockSize, size_t minChunkSize, size_t maxChunkSize, const allocator_type& allocator);
    virtual ~MemoryPool();

    using MemoryPoolBase::BlockSize;
    using MemoryPoolBase::CurrentChunkSize;
    using MemoryPoolBase::BlockCountPerChunk;
    using MemoryPoolBase::ResetChunkSize;

    void *Allocate(MemoryTrackingData *trackingData = nullptr);
    void Deallocate(void *ptr, MemoryTrackingData *trackingData = nullptr);

    virtual void Clear_AssertCompletelyFree() override;
    virtual void Clear_IgnoreLeaks() override;
    virtual void Clear_UnusedMemory() override;

    friend MemoryPoolBase* _MemoryPoolBase(MemoryPool* ppool) { return ppool; }

private:
    MemoryPoolChunk *AllocateChunk_();
    void DeallocateChunk_(MemoryPoolChunk *chunk);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Memory/MemoryPool-inl.h"
