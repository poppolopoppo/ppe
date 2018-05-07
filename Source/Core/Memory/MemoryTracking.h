#pragma once

#include "Core/Core.h"

#include "Core/IO/TextWriter_fwd.h"
#include "Core/Meta/TypeTraits.h"

namespace Core {
template <typename T>
class TMemoryView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMemoryTracking {
public:
    FMemoryTracking(const char* optionalName = "unknown", FMemoryTracking* optionalParent = nullptr);

    const char* Name() const { return _name; }
    size_t Level() const { return _level; }

    FMemoryTracking* Parent() { return _parent; }
    const FMemoryTracking* Parent() const { return _parent; }

    FMemoryTracking* Prev() { return _prev; }
    const FMemoryTracking* Prev() const { return _prev; }
    void SetPrev(FMemoryTracking* value) { _prev = value; }

    FMemoryTracking* Next() { return _next; }
    const FMemoryTracking* Next() const { return _next; }
    void SetNext(FMemoryTracking* value) { _next = value; }

    size_t BlockCount() const { return _blockCount; }
    size_t AllocationCount() const { return _allocationCount; }
    size_t TotalSizeInBytes() const { return _totalSizeInBytes; }

    size_t MaxBlockCount() const { return _maxBlockCount; }
    size_t MaxAllocationCount() const { return _maxAllocationCount; }

    size_t MaxStrideInBytes() const { return _maxStrideInBytes; }
    size_t MinStrideInBytes() const { return _minStrideInBytes; }

    size_t MaxTotalSizeInBytes() const { return _maxTotalSizeInBytes; }

    bool IsChildOf(const FMemoryTracking& other) const;

    void Allocate(size_t blockCount, size_t strideInBytes);
    void Deallocate(size_t blockCount, size_t strideInBytes);

    // used by linear heaps :
    void ReleaseAll();
    void Release(size_t blockCount, size_t allocationCount, size_t totalSizeInBytes);

    // reserved for pool allocation tracking :
    void Pool_AllocateOneBlock(size_t blockSizeInBytes);
    void Pool_DeallocateOneBlock(size_t blockSizeInBytes);
    void Pool_AllocateOneChunk(size_t chunkSizeInBytes, size_t numBlocks);
    void Pool_DeallocateOneChunk(size_t chunkSizeInBytes, size_t numBlocks);

    static FMemoryTracking& UsedMemory();
    static FMemoryTracking& ReservedMemory();
    static FMemoryTracking& PooledMemory();

private:
    std::atomic<size_t> _blockCount;
    std::atomic<size_t> _allocationCount;
    std::atomic<size_t> _prevAllocationCount;
    std::atomic<size_t> _totalSizeInBytes;

    std::atomic<size_t> _maxBlockCount;
    std::atomic<size_t> _maxAllocationCount;

    std::atomic<size_t> _maxStrideInBytes;
    std::atomic<size_t> _minStrideInBytes;

    std::atomic<size_t> _maxTotalSizeInBytes;

    FMemoryTracking* _parent;

    FMemoryTracking* _prev;
    FMemoryTracking* _next;

    const char* _name;
    size_t _level;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
