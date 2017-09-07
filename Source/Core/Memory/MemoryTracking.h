#pragma once

#include "Core/Core.h"

#include "Core/IO/FormatHelpers.h"
#include "Core/Meta/TypeTraits.h"

#include <iosfwd>

namespace Core {
template <typename T>
class TMemoryView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMemoryTrackingData {
public:
    FMemoryTrackingData(const char* optionalName = "unknown",
                        FMemoryTrackingData* optionalParent = nullptr);

    const char* Name() const { return _name; }

    FMemoryTrackingData* Parent() { return _parent; }
    const FMemoryTrackingData* Parent() const { return _parent; }

    FCountOfElements BlockCount() const { return FCountOfElements{ _blockCount }; }
    FCountOfElements AllocationCount() const { return FCountOfElements{ _allocationCount }; }
    FSizeInBytes TotalSizeInBytes() const { return FSizeInBytes{ _totalSizeInBytes }; }

    FCountOfElements MaxBlockCount() const { return FCountOfElements{ _maxBlockCount }; }
    FCountOfElements MaxAllocationCount() const { return FCountOfElements{ _maxAllocationCount }; }
    
    FSizeInBytes MaxStrideInBytes() const { return FSizeInBytes{ _maxStrideInBytes }; }
    FSizeInBytes MinStrideInBytes() const { return FSizeInBytes{ _minStrideInBytes }; }

    FSizeInBytes MaxTotalSizeInBytes() const { return FSizeInBytes{ _maxTotalSizeInBytes }; }

    void Allocate(size_t blockCount, size_t strideInBytes);
    void Deallocate(size_t blockCount, size_t strideInBytes);

    // reserved for pool allocation tracking :
    void Pool_AllocateOneBlock(size_t blockSizeInBytes);
    void Pool_DeallocateOneBlock(size_t blockSizeInBytes);
    void Pool_AllocateOneChunk(size_t chunkSizeInBytes, size_t numBlocks);
    void Pool_DeallocateOneChunk(size_t chunkSizeInBytes, size_t numBlocks);

    static FMemoryTrackingData& Global();

private:
    std::atomic<size_t> _blockCount;
    std::atomic<size_t> _allocationCount;
    std::atomic<size_t> _totalSizeInBytes;

    std::atomic<size_t> _maxBlockCount;
    std::atomic<size_t> _maxAllocationCount;

    std::atomic<size_t> _maxStrideInBytes;
    std::atomic<size_t> _minStrideInBytes;

    std::atomic<size_t> _maxTotalSizeInBytes;

    FMemoryTrackingData* _parent;
    const char* _name;
};
//----------------------------------------------------------------------------
void ReportTrackingDatas(   std::basic_ostream<wchar_t>& oss,
                            const wchar_t *header,
                            const TMemoryView<const FMemoryTrackingData * const>& datas );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
