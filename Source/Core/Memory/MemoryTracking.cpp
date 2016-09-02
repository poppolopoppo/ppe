#include "stdafx.h"

#include "MemoryTracking.h"

#include "Diagnostic/Callstack.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/Stream.h"
#include "IO/StringView.h"
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

    size_t value = 0;

    value = _blockCount;
    if (_maxBlockCount < value)
        _maxBlockCount = value;

    value = _allocationCount;
    if (_maxAllocationCount < value)
        _maxAllocationCount = value;

    value = _totalSizeInBytes;
    if (_maxTotalSizeInBytes < value)
        _maxTotalSizeInBytes = value;

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

        size_t value = 0;

        value = _blockCount;
        if (_maxBlockCount < value)
            _maxBlockCount = value;

        value = _allocationCount;
        if (_maxAllocationCount < value)
            _maxAllocationCount = value;

        value = _totalSizeInBytes;
        if (_maxTotalSizeInBytes < value)
            _maxTotalSizeInBytes = value;

        _parent->Pool_AllocateOneBlock(blockSizeInBytes);
    }
    else {
        ++_blockCount;

        size_t value = _blockCount;
        if (_maxBlockCount < value)
            _maxBlockCount = value;
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
static void TrackingDataAbsoluteName_(BasicOCStrStream<char>& oss, const MemoryTrackingData& trackingData) {
    if (trackingData.Parent()) {
        TrackingDataAbsoluteName_(oss, *trackingData.Parent());
        oss << "::";
    }
    oss << trackingData.Name();
}
//----------------------------------------------------------------------------
static bool LessTrackingData_(const MemoryTrackingData& lhs, const MemoryTrackingData& rhs) {
    Assert(lhs.Name());
    Assert(rhs.Name());
    return (lhs.Name() != rhs.Name()) &&
        CompareI(   MakeStringView(lhs.Name(), Meta::noinit_tag()),
                    MakeStringView(rhs.Name(), Meta::noinit_tag()) ) < 0;
}
//----------------------------------------------------------------------------
void ReportTrackingDatas(   std::basic_ostream<wchar_t>& oss,
                            const wchar_t *header,
                            const MemoryView<const MemoryTrackingData *>& datas ) {
    Assert(header);

    oss << L"Reporting trackings data :" << std::endl;

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

    const size_t width = 109;
    const wchar_t fmt[] = L" {0:-40}|{1:8} {2:10} |{3:8} {4:11} |{5:11} {6:11}\n";

    oss << Repeat<width>(L"-") << std::endl
        << "    " << header << L" (" << datas.size() << L" elements)" << std::endl
        << Repeat<width>(L"-") << std::endl;

    Format(oss, fmt,    L"Tracking Data Name",
                        L"Block", "Max",
                        L"Alloc", "Max",
                        L"Total", "Max" );

    oss << Repeat<width>(L"-") << std::endl;

    STACKLOCAL_OCSTRSTREAM(tmp, 256);
    for (const MemoryTrackingData *data : datas) {
        Assert(data);
        tmp.Reset();
        TrackingDataAbsoluteName_(tmp, *data);
        Format(oss, fmt,    tmp.NullTerminatedStr(),
                            data->BlockCount(),
                            data->MaxBlockCount(),
                            data->AllocationCount(),
                            data->MaxAllocationCount(),
                            data->TotalSizeInBytes(),
                            data->MaxTotalSizeInBytes() );
    }

    oss << Repeat<width>(L"-") << std::endl;
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
