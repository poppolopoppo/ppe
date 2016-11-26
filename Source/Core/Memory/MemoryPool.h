#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Container/IntrusiveList.h"
#include "Core/Meta/ThreadResource.h"
#include "Core/Thread/AtomicSpinLock.h"

// Uncomment to disable pool allocation (useful for memory debugging) :
//#define WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC //%__NOCOMMIT%

namespace Core {
class FMemoryTrackingData;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4324) // warning C4324: 'Core::FMemoryPoolChunk' : structure was padded due to __declspec(align())
//----------------------------------------------------------------------------
ALIGN(16) class FMemoryPoolChunk {
public:
    friend class FMemoryPoolBase;

    struct FBlock { FBlock *Next; };

    explicit FMemoryPoolChunk(size_t chunkSize, size_t blockCount);
    ~FMemoryPoolChunk();

    FMemoryPoolChunk(FMemoryPoolChunk&&) = delete;
    FMemoryPoolChunk& operator =(FMemoryPoolChunk&&) = delete;

    FMemoryPoolChunk(const FMemoryPoolChunk&) = delete;
    FMemoryPoolChunk& operator =(const FMemoryPoolChunk&) = delete;

    size_t ChunkSize() const { return _chunkSize; }
    size_t BlockCount() const { return _blockCount; }

    IntrusiveListNode<FMemoryPoolChunk>& Node() { return _node; }

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

    u32 _blockCount;
    u32 _blockUsed;
    u32 _blockAdded;

    const size_t _chunkSize;

    IntrusiveListNode<FMemoryPoolChunk> _node;
};
//----------------------------------------------------------------------------
STATIC_ASSERT(IS_ALIGNED(16, sizeof(FMemoryPoolChunk)));
//----------------------------------------------------------------------------
#pragma warning(pop)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMemoryPoolBase {
public:
    friend class FMemoryPoolBaseList;

    FMemoryPoolBase(size_t blockSize, size_t minChunkSize, size_t maxChunkSize);
    virtual ~FMemoryPoolBase();

    FMemoryPoolBase(FMemoryPoolBase&&) = delete;
    FMemoryPoolBase& operator =(FMemoryPoolBase&&) = delete;

    FMemoryPoolBase(const FMemoryPoolBase&) = delete;
    FMemoryPoolBase& operator =(const FMemoryPoolBase&) = delete;

    size_t BlockSize() const { return _blockSize; }
    size_t MinChunkSize() const { return _minChunkSize; }
    size_t MaxChunksize() const { return _maxChunkSize; }
    size_t CurrentChunkSize() const { return _currentChunksize; }
    size_t ChunkCount() const { return _chunkCount; }

    FMemoryPoolChunk *Chunks() { return _chunks.Head(); }
    const FMemoryPoolChunk *Chunks() const { return _chunks.Head(); }

    size_t BlockCountPerChunk(size_t chunkSize) const { return (chunkSize - sizeof(FMemoryPoolChunk)) / _blockSize; }

    virtual void Clear_AssertCompletelyFree() = 0;
    virtual void Clear_IgnoreLeaks() = 0;
    virtual void Clear_UnusedMemory() = 0;

    const IntrusiveListNode<FMemoryPoolBase>& Node() const { return _node; }

protected:
    void GrowChunkSizeIFP();
    void ResetChunkSize();

    void AddChunk(FMemoryPoolChunk *chunk);

    void *TryAllocate_FailIfNoBlockAvailable();
    FMemoryPoolChunk *Deallocate_ReturnChunkToRelease(void *ptr);

    FMemoryPoolChunk *ClearOneChunk_AssertCompletelyFree();
    FMemoryPoolChunk *ClearOneChunk_IgnoreLeaks();
    FMemoryPoolChunk *ClearOneChunk_UnusedMemory();

private:
    void SpareChunk_(FMemoryPoolChunk *chunk);
    FMemoryPoolChunk* ReviveChunk_();
    FMemoryPoolChunk* ReleaseChunk_();

    typedef INTRUSIVELIST(&FMemoryPoolChunk::_node) list_type;

    list_type _chunks;
    list_type _spares;

    size_t _chunkCount;
    size_t _usedSize;
    size_t _totalSize;

    size_t _currentChunksize;

    const size_t _blockSize;
    const size_t _minChunkSize;
    const size_t _maxChunkSize;

    IntrusiveListNode<FMemoryPoolBase> _node;
};
//----------------------------------------------------------------------------
class FMemoryPoolBaseList {
public:
    FMemoryPoolBaseList();
    ~FMemoryPoolBaseList();

    FMemoryPoolBaseList(const FMemoryPoolBaseList& ) = delete;
    FMemoryPoolBaseList& operator =(const FMemoryPoolBaseList& ) = delete;

    void Insert(FMemoryPoolBase* ppool);
    void Remove(FMemoryPoolBase* ppool);

    void ClearAll_AssertCompletelyFree();
    void ClearAll_IgnoreLeaks();
    void ClearAll_UnusedMemory();

private:
    typedef INTRUSIVELIST(&FMemoryPoolBase::_node) list_type;

    FAtomicSpinLock _barrier;
    list_type _pools;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator = ALLOCATOR(Pool, u64) >
class TMemoryPool : protected FMemoryPoolBase, private _Allocator, private Meta::TThreadLock<_Lock> {
public:
    typedef Meta::TThreadLock<_Lock> threadlock_type;
    typedef _Allocator allocator_type;

    enum : bool { IsLocked = _Lock };

    TMemoryPool(size_t blockSize, size_t minChunkSize, size_t maxChunkSize);
    TMemoryPool(size_t blockSize, size_t minChunkSize, size_t maxChunkSize, allocator_type&& allocator);
    TMemoryPool(size_t blockSize, size_t minChunkSize, size_t maxChunkSize, const allocator_type& allocator);
    virtual ~TMemoryPool();

    using FMemoryPoolBase::BlockSize;
    using FMemoryPoolBase::CurrentChunkSize;
    using FMemoryPoolBase::BlockCountPerChunk;
    using FMemoryPoolBase::ResetChunkSize;

    void *Allocate(FMemoryTrackingData *trackingData = nullptr);
    void Deallocate(void *ptr, FMemoryTrackingData *trackingData = nullptr);

    virtual void Clear_AssertCompletelyFree() override final;
    virtual void Clear_IgnoreLeaks() override final;
    virtual void Clear_UnusedMemory() override final;

private:
    FMemoryPoolChunk *AllocateChunk_();
    void DeallocateChunk_(FMemoryPoolChunk *chunk);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Memory/MemoryPool-inl.h"
