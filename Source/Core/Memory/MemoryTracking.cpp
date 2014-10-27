#include "stdafx.h"

#include "MemoryTracking.h"

#include "Diagnostic/Callstack.h"

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
// TODO: not DLL safe
static MemoryTrackingData gGlobalMemoryTrackingData{ "$" };
MemoryTrackingData& MemoryTrackingData::Global() {
    return gGlobalMemoryTrackingData;
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
