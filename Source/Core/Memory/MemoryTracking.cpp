#include "stdafx.h"

#include "MemoryTracking.h"

#include "Diagnostic/Callstack.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/Stream.h"
#include "IO/String.h"
#include "Memory/UniqueView.h"
#include "Meta/OneTimeInitialize.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MemoryTrackingData::MemoryTrackingData(
    const char* optionalName /*= "unknown"*/,
    MemoryTrackingData* optionalParent /*= nullptr*/)
:   _name(optionalName), _parent(optionalParent),
    _blockCount(0), _allocationCount(0), _totalSizeInBytes(0),
    _maxBlockCount(0), _maxAllocationCount(0), _maxTotalSizeInBytes(0) {}
//----------------------------------------------------------------------------
void MemoryTrackingData::Allocate(size_t blockCount, size_t strideInBytes) {
    _blockCount += blockCount;
    _totalSizeInBytes += blockCount * strideInBytes;
    ++_allocationCount;

    _maxBlockCount = max(_maxBlockCount, _blockCount).load();
    _maxAllocationCount = max(_maxAllocationCount, _allocationCount).load();
    _maxTotalSizeInBytes = max(_maxTotalSizeInBytes, _totalSizeInBytes).load();

    if (_parent)
        _parent->Allocate(blockCount, strideInBytes);
}
//----------------------------------------------------------------------------
void MemoryTrackingData::Deallocate(size_t blockCount, size_t strideInBytes) {
    Assert(_blockCount >= blockCount);
    Assert(_totalSizeInBytes >= blockCount * strideInBytes);
    Assert(_allocationCount);

    _blockCount -= blockCount;
    _totalSizeInBytes -= blockCount * strideInBytes;
    --_allocationCount;

    if (_parent)
        _parent->Deallocate(blockCount, strideInBytes);
}
//----------------------------------------------------------------------------
void MemoryTrackingData::Pool_AllocateOneBlock(size_t blockSizeInBytes) {
    if (_parent) {
        ++_blockCount;
        _totalSizeInBytes += blockSizeInBytes;
        ++_allocationCount;

        _maxBlockCount = max(_maxBlockCount, _blockCount).load();
        _maxAllocationCount = max(_maxAllocationCount, _allocationCount).load();
        _maxTotalSizeInBytes = max(_maxTotalSizeInBytes, _totalSizeInBytes).load();

        _parent->Pool_AllocateOneBlock(blockSizeInBytes);
    }
    else {
        ++_blockCount;
        _maxBlockCount = max(_maxBlockCount, _blockCount).load();
    }
}
//----------------------------------------------------------------------------
void MemoryTrackingData::Pool_DeallocateOneBlock(size_t blockSizeInBytes) {
    if (_parent) {
        Assert(_blockCount >= 1);
        Assert(_totalSizeInBytes >= blockSizeInBytes);
        Assert(_allocationCount);

        --_blockCount;
        _totalSizeInBytes -= blockSizeInBytes;
        --_allocationCount;

        _parent->Pool_DeallocateOneBlock(blockSizeInBytes);
    }
    else {
        Assert(_blockCount > 0);
        --_blockCount;
    }
}
//----------------------------------------------------------------------------
void MemoryTrackingData::Pool_AllocateOneChunk(size_t chunkSizeInBytes) {
    if (_parent) {
        _parent->Pool_AllocateOneChunk(chunkSizeInBytes);
    }
    else {
        _totalSizeInBytes += chunkSizeInBytes;
        ++_allocationCount;

        _maxAllocationCount = max(_maxAllocationCount, _allocationCount).load();
        _maxTotalSizeInBytes = max(_maxTotalSizeInBytes, _totalSizeInBytes).load();
    }
}
//----------------------------------------------------------------------------
void MemoryTrackingData::Pool_DeallocateOneChunk(size_t chunkSizeInBytes) {
    if (_parent) {
        _parent->Pool_DeallocateOneChunk(chunkSizeInBytes);
    }
    else {
        Assert(_totalSizeInBytes >= chunkSizeInBytes);
        Assert(_allocationCount);

        _totalSizeInBytes -= chunkSizeInBytes;
        --_allocationCount;
    }
}
//----------------------------------------------------------------------------
void MemoryTrackingData::Append(const MemoryTrackingData& other) {
    _blockCount += other._blockCount;
    _allocationCount += other._allocationCount;
    _totalSizeInBytes += other._totalSizeInBytes;

    _maxBlockCount = max(_maxBlockCount, _blockCount).load();
    _maxAllocationCount = max(_maxAllocationCount, _allocationCount).load();
    _maxTotalSizeInBytes = max(_maxTotalSizeInBytes, _totalSizeInBytes).load();

    if (_parent)
        _parent->Append(other);
}
//----------------------------------------------------------------------------
MemoryTrackingData& MemoryTrackingData::Global() {
    ONE_TIME_INITIALIZE(MemoryTrackingData, gGlobalMemoryTrackingData, "$");
    return gGlobalMemoryTrackingData;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static void TrackingDataAbsoluteName_(OCStrStream *pOss, const MemoryTrackingData& trackingData) {
    if (trackingData.Parent()) {
        TrackingDataAbsoluteName_(pOss, *trackingData.Parent());
        *pOss << "::";
    }
    *pOss << trackingData.Name();
    pOss->PutEOS();
}
//----------------------------------------------------------------------------
static bool LessTrackingData_(const MemoryTrackingData& lhs, const MemoryTrackingData& rhs) {
    Assert(lhs.Name());
    Assert(rhs.Name());
    return (lhs.Name() != rhs.Name()) && CompareI(lhs.Name(), rhs.Name()) < 0;
}
//----------------------------------------------------------------------------
void ReportTrackingDatas(   std::basic_ostream<char>& oss, 
                            const char *header, 
                            const MemoryView<const MemoryTrackingData *>& datas ) {
    Assert(header);

    STACKLOCAL_POD_ARRAY(const MemoryTrackingData *, sortedDatas, datas.size());
    memcpy(sortedDatas.Pointer(), datas.Pointer(), datas.SizeInBytes());

    std::stable_sort(sortedDatas.begin(), sortedDatas.end(), [](const MemoryTrackingData *lhs, const MemoryTrackingData *rhs) {
        const MemoryTrackingData *lhsp = lhs->Parent();
        const MemoryTrackingData *rhsp = rhs->Parent();
        if (lhsp && rhsp)
            return (LessTrackingData_(*lhsp, *rhsp) || (lhsp->Name() == rhsp->Name() && LessTrackingData_(*lhs, *rhs)));
        else if (lhsp)
            return LessTrackingData_(*lhsp, *rhs);
        else if (rhsp)
            return lhs->Name() == rhsp->Name() || LessTrackingData_(*lhs, *rhsp);
        else
            return LessTrackingData_(*lhs, *rhs);
    });
    
    const size_t width = 139;
    const char *fmt = " {0:-73}|{1:10} {2:10} |{3:7} {4:7} |{5:11} {6:11}\n";

    oss << Repeat<width>("-") << std::endl
        << "    " << header << " (" << datas.size() << " elements)" << std::endl
        << Repeat<width>("-") << std::endl;

    Format(oss, fmt,    "Tracking Data Name",
                        "Block", "Max",
                        "Alloc", "Max",
                        "Total", "Max" );

    oss << Repeat<width>("-") << std::endl;

    char absoluteName[1024];
    for (const MemoryTrackingData *data : datas) {
        Assert(data);
        OCStrStream tmp(absoluteName);
        TrackingDataAbsoluteName_(&tmp, *data);
        Format(oss, fmt,    absoluteName,
                            data->BlockCount(),
                            data->MaxBlockCount(),
                            data->AllocationCount(),
                            data->MaxAllocationCount(),
                            data->TotalSizeInBytes(),
                            data->MaxTotalSizeInBytes() );
    }

    oss << Repeat<width>("-") << std::endl;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MemoryBlockHeader::MemoryBlockHeader()
:   _trackingData(nullptr), _blockCount(0), _strideInBytes(0) {
    _backtraceFrames[0] = nullptr;
}
//----------------------------------------------------------------------------
MemoryBlockHeader::MemoryBlockHeader(MemoryTrackingData* trackingData, size_t blockCount, size_t strideInBytes)
: _trackingData(trackingData), _blockCount(checked_cast<uint32_t>(blockCount)), _strideInBytes(checked_cast<uint32_t>(strideInBytes)) {
    Assert(blockCount == _blockCount);
    Assert(strideInBytes == _strideInBytes);
    if (_trackingData)
        _trackingData->Allocate(_blockCount, _strideInBytes);
    const size_t backtraceDepth = Callstack::Capture(MakeView(_backtraceFrames), nullptr, 2, BacktraceMaxDepth);
    if (backtraceDepth < BacktraceMaxDepth)
        _backtraceFrames[backtraceDepth] = nullptr;
}
//----------------------------------------------------------------------------
MemoryBlockHeader::~MemoryBlockHeader() {
    if (_trackingData)
        _trackingData->Deallocate(_blockCount, _strideInBytes);
}
//----------------------------------------------------------------------------
void MemoryBlockHeader::DecodeCallstack(DecodedCallstack* decoded) const {
    size_t backtraceDepth = 0;
    while (backtraceDepth < BacktraceMaxDepth && _backtraceFrames[backtraceDepth])
        ++backtraceDepth;
    auto frames = MakeView(&_backtraceFrames[0], &_backtraceFrames[backtraceDepth]);
    Callstack::Decode(decoded, 0, frames);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
