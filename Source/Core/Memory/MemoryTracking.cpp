#include "stdafx.h"

#include "MemoryTracking.h"

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
FMemoryTrackingData::FMemoryTrackingData(
    const char* optionalName /*= "unknown"*/,
    FMemoryTrackingData* optionalParent /*= nullptr*/)
:   _blockCount(0), _allocationCount(0), _totalSizeInBytes(0),
    _maxBlockCount(0), _maxAllocationCount(0), _maxStrideInBytes(0), _minStrideInBytes(UINT32_MAX), _maxTotalSizeInBytes(0),
    _parent(optionalParent), _name(optionalName) {}
//----------------------------------------------------------------------------
void FMemoryTrackingData::Allocate(size_t blockCount, size_t strideInBytes) {
    if (0 == blockCount)
        return;

    _blockCount += blockCount;
    _totalSizeInBytes += blockCount * strideInBytes;
    ++_allocationCount;

    _maxStrideInBytes = Max(_maxStrideInBytes.load(), strideInBytes);
    _minStrideInBytes = Min(_minStrideInBytes.load(), strideInBytes);

    _maxBlockCount = Max(_maxBlockCount.load(), _blockCount.load());
    _maxAllocationCount = Max(_maxAllocationCount.load(), _allocationCount.load());
    _maxTotalSizeInBytes = Max(_maxTotalSizeInBytes.load(), _totalSizeInBytes.load());

    if (_parent)
        _parent->Allocate(blockCount, strideInBytes);
}
//----------------------------------------------------------------------------
void FMemoryTrackingData::Deallocate(size_t blockCount, size_t strideInBytes) {
    if (0 == blockCount)
        return;

    Assert(_blockCount >= blockCount);
    Assert(_totalSizeInBytes >= blockCount * strideInBytes);
    Assert(_allocationCount);
    Assert(_maxStrideInBytes >= strideInBytes);
    Assert(_minStrideInBytes <= strideInBytes);

    _blockCount -= blockCount;
    _totalSizeInBytes -= blockCount * strideInBytes;
    --_allocationCount;

    if (_parent)
        _parent->Deallocate(blockCount, strideInBytes);
}
//----------------------------------------------------------------------------
void FMemoryTrackingData::Pool_AllocateOneBlock(size_t blockSizeInBytes) {
    ++_blockCount;

    _maxStrideInBytes = Max(_maxStrideInBytes.load(), blockSizeInBytes);
    _minStrideInBytes = Min(_minStrideInBytes.load(), blockSizeInBytes);

    if (_parent)
        _parent->Pool_AllocateOneBlock(blockSizeInBytes);
}
//----------------------------------------------------------------------------
void FMemoryTrackingData::Pool_DeallocateOneBlock(size_t blockSizeInBytes) {
    Assert(_blockCount >= 1);
    Assert(_maxStrideInBytes >= blockSizeInBytes);
    Assert(_minStrideInBytes <= blockSizeInBytes);

    --_blockCount;

    if (_parent)
        _parent->Pool_DeallocateOneBlock(blockSizeInBytes);
}
//----------------------------------------------------------------------------
void FMemoryTrackingData::Pool_AllocateOneChunk(size_t chunkSizeInBytes, size_t numBlocks) {
    Assert(chunkSizeInBytes >= numBlocks);
    Assert(numBlocks > 0);

    _maxBlockCount += numBlocks;
    _totalSizeInBytes += chunkSizeInBytes;
    ++_allocationCount;

    _maxAllocationCount = Max(_maxAllocationCount.load(), _allocationCount.load());
    _maxTotalSizeInBytes = Max(_maxTotalSizeInBytes.load(), _totalSizeInBytes.load());

    if (_parent)
        _parent->Pool_AllocateOneChunk(chunkSizeInBytes, numBlocks);
}
//----------------------------------------------------------------------------
void FMemoryTrackingData::Pool_DeallocateOneChunk(size_t chunkSizeInBytes, size_t numBlocks) {
    Assert(chunkSizeInBytes >= numBlocks);
    Assert(numBlocks > 0);

    Assert(_maxBlockCount >= numBlocks);
    Assert(_totalSizeInBytes >= chunkSizeInBytes);
    Assert(_allocationCount);

    _maxBlockCount -= numBlocks;
    _totalSizeInBytes -= chunkSizeInBytes;
    --_allocationCount;

    if (_parent)
        _parent->Pool_DeallocateOneChunk(chunkSizeInBytes, numBlocks);
}
//----------------------------------------------------------------------------
FMemoryTrackingData& FMemoryTrackingData::Global() {
    ONE_TIME_INITIALIZE(FMemoryTrackingData, GGlobalMemoryTrackingData, "$");
    return GGlobalMemoryTrackingData;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static void TrackingDataAbsoluteName_(TBasicOCStrStream<char>& oss, const FMemoryTrackingData& trackingData) {
    if (trackingData.Parent() && trackingData.Parent() != &FMemoryTrackingData::Global()) {
        TrackingDataAbsoluteName_(oss, *trackingData.Parent());
        oss << "::";
    }
    oss << trackingData.Name();
}
//----------------------------------------------------------------------------
static bool LessTrackingData_(const FMemoryTrackingData& lhs, const FMemoryTrackingData& rhs) {
    Assert(lhs.Name());
    Assert(rhs.Name());
    return (lhs.Name() != rhs.Name()) &&
        CompareI(   MakeStringView(lhs.Name(), Meta::FForceInit{}),
                    MakeStringView(rhs.Name(), Meta::FForceInit{})) < 0;
}
//----------------------------------------------------------------------------
void ReportTrackingDatas(   std::basic_ostream<wchar_t>& oss,
                            const wchar_t *header,
                            const TMemoryView<const FMemoryTrackingData * const>& datas ) {
    Assert(header);

    if (datas.empty())
        return;

    oss << L"Reporting tracking data :" << eol;

    STACKLOCAL_POD_ARRAY(const FMemoryTrackingData *, sortedDatas, datas.size());
    memcpy(sortedDatas.Pointer(), datas.Pointer(), datas.SizeInBytes());

    std::stable_sort(sortedDatas.begin(), sortedDatas.end(), [](const FMemoryTrackingData *lhs, const FMemoryTrackingData *rhs) {
        const FMemoryTrackingData *lhsp = lhs->Parent();
        const FMemoryTrackingData *rhsp = rhs->Parent();
        if (lhsp && rhsp)
            return (LessTrackingData_(*lhsp, *rhsp) || (lhsp->Name() == rhsp->Name() && LessTrackingData_(*lhs, *rhs)));
        else if (lhsp)
            return LessTrackingData_(*lhsp, *rhs);
        else if (rhsp)
            return lhs->Name() == rhsp->Name() || LessTrackingData_(*lhs, *rhsp);
        else
            return LessTrackingData_(*lhs, *rhs);
    });

    const size_t width = 128;
    const wchar_t fmt[] = L" {0:-37}|{1:8} {2:10} |{3:8} {4:11} |{5:8} {6:11} |{7:11} {8:11}\n";

    oss << Fmt::Repeat(L'-', width) << eol
        << "    " << header << L" (" << datas.size() << L" elements)" << eol
        << Fmt::Repeat(L'-', width) << eol;

    Format(oss, fmt,    L"Tracking Data FName",
                        L"Block", "Max",
                        L"Alloc", "Max",
                        L"Stride", "Max",
                        L"Total", "Max" );

    oss << Fmt::Repeat(L'-', width) << eol;

    STACKLOCAL_OCSTRSTREAM(tmp, 256);
    for (const FMemoryTrackingData *data : datas) {
        Assert(data);
        tmp.Reset();
        TrackingDataAbsoluteName_(tmp, *data);
        Format(oss, fmt,
            tmp.NullTerminatedStr(),
            Fmt::FCountOfElements{ data->BlockCount() },
            Fmt::FCountOfElements{ data->MaxBlockCount() },
            Fmt::FCountOfElements{ data->AllocationCount() },
            Fmt::FCountOfElements{ data->MaxAllocationCount() },
            Fmt::FSizeInBytes{ Min(data->MaxStrideInBytes(), data->MinStrideInBytes()) },
            Fmt::FSizeInBytes{ data->MaxStrideInBytes() },
            Fmt::FSizeInBytes{ data->TotalSizeInBytes() },
            Fmt::FSizeInBytes{ data->MaxTotalSizeInBytes() });
    }

    oss << Fmt::Repeat(L'-', width) << eol;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
