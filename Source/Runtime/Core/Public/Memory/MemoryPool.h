#pragma once

#include "Core.h"

#include "Container/IntrusiveList.h"
#include "Thread/AtomicSpinLock.h"

#include <mutex>

namespace PPE {
class FMemoryTracking;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMemoryPoolChunk;
//----------------------------------------------------------------------------
class FMemoryPool {
public:
    FMemoryPool(size_t blockSize, size_t minChunkSize, size_t maxChunkSize);
    virtual ~FMemoryPool();

    FMemoryPool(FMemoryPool&&) = delete;
    FMemoryPool& operator =(FMemoryPool&&) = delete;

    FMemoryPool(const FMemoryPool&) = delete;
    FMemoryPool& operator =(const FMemoryPool&) = delete;

    size_t BlockSize() const { return _blockSize; }
    size_t MinChunkSize() const { return _minChunkSize; }
    size_t MaxChunksize() const { return _maxChunkSize; }
    size_t CurrentChunkSize() const { return _currentChunksize; }
    size_t ChunkCount() const { return _chunkCount; }

    void* Allocate(FMemoryTracking *trackingData = nullptr);
    void Deallocate(void *ptr, FMemoryTracking *trackingData = nullptr);

    void Clear_AssertCompletelyFree();
    void Clear_IgnoreLeaks();
    void Clear_UnusedMemory();

private:
    size_t BlockCountPerChunk_(size_t chunkSize) const;

    void GrowChunkSizeIFP_();
    void ResetChunkSize_();

    void AddChunk_(FMemoryPoolChunk *chunk);

    void* TryAllocate_FailIfNoBlockAvailable_();
    FMemoryPoolChunk* Deallocate_ReturnChunkToRelease_(void* ptr);

    FMemoryPoolChunk* ClearOneChunk_AssertCompletelyFree_();
    FMemoryPoolChunk* ClearOneChunk_IgnoreLeaks_();
    FMemoryPoolChunk* ClearOneChunk_UnusedMemory_();

    void SpareChunk_(FMemoryPoolChunk *chunk);
    FMemoryPoolChunk* ReviveChunk_();
    FMemoryPoolChunk* ReleaseChunk_();

    FMemoryPoolChunk* AllocateChunk_();
    void DeallocateChunk_(FMemoryPoolChunk* chunk);

    FMemoryPoolChunk* _chunksHead;
    FMemoryPoolChunk* _chunksTail;

    FMemoryPoolChunk* _sparesHead;
    FMemoryPoolChunk* _sparesTail;

    u32 _chunkCount;
    u32 _usedSize;
    u32 _totalSize;
    u32 _currentChunksize;

    const u32 _blockSize;
    const u32 _minChunkSize;
    const u32 _maxChunkSize;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IMemoryPool {
public:
    virtual ~IMemoryPool() {
        Assert(nullptr == _node.Next);
        Assert(nullptr == _node.Prev);
    }

    virtual void Clear_AssertCompletelyFree() = 0;
    virtual void Clear_IgnoreLeaks() = 0;
    virtual void Clear_UnusedMemory() = 0;

    const TIntrusiveListNode<IMemoryPool>& Node() const { return _node; }

private:
    friend class FMemoryPoolList;
    TIntrusiveListNode<IMemoryPool> _node;
};
//----------------------------------------------------------------------------
class FMemoryPoolThreadLocal : public IMemoryPool, Meta::FThreadResource {
public:
    FMemoryPoolThreadLocal(size_t blockSize, size_t minChunkSize, size_t maxChunkSize)
        : _pool(blockSize, minChunkSize, maxChunkSize) {}
    virtual ~FMemoryPoolThreadLocal() {}

    FMemoryPoolThreadLocal(FMemoryPoolThreadLocal&&) = delete;
    FMemoryPoolThreadLocal& operator =(FMemoryPoolThreadLocal&&) = delete;

    FMemoryPoolThreadLocal(const FMemoryPoolThreadLocal&) = delete;
    FMemoryPoolThreadLocal& operator =(const FMemoryPoolThreadLocal&) = delete;

    size_t BlockSize() const { return _pool.BlockSize(); }
    size_t MinChunkSize() const { return _pool.MinChunkSize(); }
    size_t MaxChunksize() const { return _pool.MaxChunksize(); }
    size_t CurrentChunkSize() const { return _pool.CurrentChunkSize(); }
    size_t ChunkCount() const { return _pool.ChunkCount(); }

    void* Allocate(FMemoryTracking* trackingData = nullptr);
    void Deallocate(void *ptr, FMemoryTracking* trackingData = nullptr);

    virtual void Clear_AssertCompletelyFree() override final;
    virtual void Clear_IgnoreLeaks() override final;
    virtual void Clear_UnusedMemory() override final;

private:
    FMemoryPool _pool;
};
//----------------------------------------------------------------------------
class FMemoryPoolThreadSafe : public IMemoryPool {
public:
    FMemoryPoolThreadSafe(size_t blockSize, size_t minChunkSize, size_t maxChunkSize)
        : _pool(blockSize, minChunkSize, maxChunkSize) {}
    virtual ~FMemoryPoolThreadSafe() {}

    FMemoryPoolThreadSafe(FMemoryPoolThreadSafe&&) = delete;
    FMemoryPoolThreadSafe& operator =(FMemoryPoolThreadSafe&&) = delete;

    FMemoryPoolThreadSafe(const FMemoryPoolThreadSafe&) = delete;
    FMemoryPoolThreadSafe& operator =(const FMemoryPoolThreadSafe&) = delete;

    size_t BlockSize() const { return _pool.BlockSize(); }
    size_t MinChunkSize() const { return _pool.MinChunkSize(); }
    size_t MaxChunksize() const { return _pool.MaxChunksize(); }
    size_t CurrentChunkSize() const { return _pool.CurrentChunkSize(); }
    size_t ChunkCount() const { return _pool.ChunkCount(); }

    void* Allocate(FMemoryTracking* trackingData = nullptr);
    void Deallocate(void *ptr, FMemoryTracking* trackingData = nullptr);

    virtual void Clear_AssertCompletelyFree() override final;
    virtual void Clear_IgnoreLeaks() override final;
    virtual void Clear_UnusedMemory() override final;

private:
    std::mutex _barrier;
    FMemoryPool _pool;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMemoryPoolList {
public:
    FMemoryPoolList();
    ~FMemoryPoolList();

    FMemoryPoolList(const FMemoryPoolList& ) = delete;
    FMemoryPoolList& operator =(const FMemoryPoolList& ) = delete;

    void Insert(IMemoryPool* ppool);
    void Remove(IMemoryPool* ppool);

    void ClearAll_AssertCompletelyFree();
    void ClearAll_IgnoreLeaks();
    void ClearAll_UnusedMemory();

private:
    typedef INTRUSIVELIST(&IMemoryPool::_node) list_type;

    FAtomicSpinLock _barrier;
    list_type _pools;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
