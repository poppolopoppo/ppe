#pragma once

#include "Core/Core.h"

#include "Core/IO/FormatHelpers.h"
#include "Core/Meta/TypeTraits.h"

#include <iosfwd>

namespace Core {
template <typename T>
class MemoryView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MemoryTrackingData {
public:
    MemoryTrackingData( const char* optionalName = "unknown",
                        MemoryTrackingData* optionalParent = nullptr);

    const char* Name() const { return _name; }

    MemoryTrackingData* Parent() { return _parent; }
    const MemoryTrackingData* Parent() const { return _parent; }

    CountOfElements BlockCount() const { return CountOfElements{ _blockCount }; }
    CountOfElements AllocationCount() const { return CountOfElements{ _allocationCount }; }
    SizeInBytes TotalSizeInBytes() const { return SizeInBytes{ _totalSizeInBytes }; }

    CountOfElements MaxBlockCount() const { return CountOfElements{ _maxBlockCount }; }
    CountOfElements MaxAllocationCount() const { return CountOfElements{ _maxAllocationCount }; }
    SizeInBytes MaxTotalSizeInBytes() const { return SizeInBytes{ _maxTotalSizeInBytes }; }

    void Allocate(size_t blockCount, size_t strideInBytes);
    void Deallocate(size_t blockCount, size_t strideInBytes);

    // reserved for pool allocation tracking :
    void Pool_AllocateOneBlock(size_t blockSizeInBytes);
    void Pool_DeallocateOneBlock(size_t blockSizeInBytes);
    void Pool_AllocateOneChunk(size_t chunkSizeInBytes);
    void Pool_DeallocateOneChunk(size_t chunkSizeInBytes);

    void Append(const MemoryTrackingData& other);

    static MemoryTrackingData& Global();

private:
    const char* _name;
    MemoryTrackingData* _parent;

    std::atomic<size_t> _blockCount;
    std::atomic<size_t> _allocationCount;
    std::atomic<size_t> _totalSizeInBytes;

    std::atomic<size_t> _maxBlockCount;
    std::atomic<size_t> _maxAllocationCount;
    std::atomic<size_t> _maxTotalSizeInBytes;
};
//----------------------------------------------------------------------------
void ReportTrackingDatas(   std::basic_ostream<char>& oss, 
                            const char *header,
                            const MemoryView<const MemoryTrackingData *>& datas );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DecodedCallstack;
//----------------------------------------------------------------------------
class MemoryBlockHeader {
public:
    MemoryBlockHeader();
    MemoryBlockHeader(MemoryTrackingData* trackingData, size_t blockCount, size_t strideInBytes);
    ~MemoryBlockHeader();

    MemoryTrackingData* TrackingData() const { return _trackingData; }
    size_t BlockCount() const { return _blockCount; }
    size_t StrideInBytes() const{ return _strideInBytes; }

    void DecodeCallstack(DecodedCallstack* decoded) const;

private:
    enum { BacktraceMaxDepth = 6 };

    MemoryTrackingData* _trackingData;
#ifdef ARCH_X64
    uint32_t    _blockCount;
    uint32_t    _strideInBytes;
#else
    uint32_t    _blockCount : 18;
    uint32_t    _strideInBytes : 14;
#endif
    void*       _backtraceFrames[BacktraceMaxDepth];
};
//----------------------------------------------------------------------------
static_assert(  sizeof(MemoryBlockHeader) == sizeof(size_t) * 8,
                "invalid MemoryBlockHeader size");
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
