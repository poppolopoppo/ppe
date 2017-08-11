#pragma once

#include "Core/Core.h"

#include "Core/Container/IntrusiveList.h"
#include "Core/Thread/AtomicSpinLock.h"

#include <mutex>

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
    friend class FMemoryPool;

    struct FBlock { FBlock *Next; };

    explicit FMemoryPoolChunk(size_t chunkSize, size_t blockCount);
    ~FMemoryPoolChunk();

    FMemoryPoolChunk(FMemoryPoolChunk&&) = delete;
    FMemoryPoolChunk& operator =(FMemoryPoolChunk&&) = delete;

    FMemoryPoolChunk(const FMemoryPoolChunk&) = delete;
    FMemoryPoolChunk& operator =(const FMemoryPoolChunk&) = delete;

    size_t ChunkSize() const { return _chunkSize; }
    size_t BlockCount() const { return _blockCount; }

    TIntrusiveListNode<FMemoryPoolChunk>& Node() { return _node; }

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

    TIntrusiveListNode<FMemoryPoolChunk> _node;
};
//----------------------------------------------------------------------------
STATIC_ASSERT(IS_ALIGNED(16, sizeof(FMemoryPoolChunk)));
//----------------------------------------------------------------------------
#pragma warning(pop)
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
class FMemoryPool : public IMemoryPool {
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

    void* Allocate(FMemoryTrackingData *trackingData = nullptr);
    void Deallocate(void *ptr, FMemoryTrackingData *trackingData = nullptr);

    virtual void Clear_AssertCompletelyFree() override;
    virtual void Clear_IgnoreLeaks() override;
    virtual void Clear_UnusedMemory() override;

private:
    size_t BlockCountPerChunk_(size_t chunkSize) const { return (chunkSize - sizeof(FMemoryPoolChunk)) / _blockSize; }

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
};
//----------------------------------------------------------------------------
#ifndef WITH_CORE_ASSERT
using FMemoryPoolThreadLocal = FMemoryPool;
#else
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

    void* Allocate(FMemoryTrackingData *trackingData = nullptr) {
        THIS_THREADRESOURCE_CHECKACCESS();
        return _pool.Allocate(trackingData);
    }

    void Deallocate(void *ptr, FMemoryTrackingData *trackingData = nullptr) {
        THIS_THREADRESOURCE_CHECKACCESS();
        _pool.Deallocate(ptr, trackingData);
    }

    virtual void Clear_AssertCompletelyFree() override {
        THIS_THREADRESOURCE_CHECKACCESS();
        _pool.Clear_AssertCompletelyFree();
    }

    virtual void Clear_IgnoreLeaks() override {
        THIS_THREADRESOURCE_CHECKACCESS();
        _pool.Clear_IgnoreLeaks();
    }

    virtual void Clear_UnusedMemory() override {
        THIS_THREADRESOURCE_CHECKACCESS();
        _pool.Clear_UnusedMemory();
    }

private:
    FMemoryPool _pool;
};
#endif //!WITH_CORE_ASSERT
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

    void* Allocate(FMemoryTrackingData *trackingData = nullptr) {
        const std::unique_lock<std::mutex> scopeLock(_barrier);
        return _pool.Allocate(trackingData);
    }

    void Deallocate(void *ptr, FMemoryTrackingData *trackingData = nullptr) {
        const std::unique_lock<std::mutex> scopeLock(_barrier);
        _pool.Deallocate(ptr, trackingData);
    }

    virtual void Clear_AssertCompletelyFree() override {
        const std::unique_lock<std::mutex> scopeLock(_barrier);
        _pool.Clear_AssertCompletelyFree();
    }

    virtual void Clear_IgnoreLeaks() override {
        const std::unique_lock<std::mutex> scopeLock(_barrier);
        _pool.Clear_IgnoreLeaks();
    }

    virtual void Clear_UnusedMemory() override {
        const std::unique_lock<std::mutex> scopeLock(_barrier);
        _pool.Clear_UnusedMemory();
    }

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
} //!namespace Core
