#include "stdafx.h"

#include "MemoryTracking.h"

#include "MemoryDomain.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMemoryTracking& FMemoryTracking::PooledMemory() {
    return MEMORYDOMAIN_TRACKING_DATA(PooledMemory);
}
//----------------------------------------------------------------------------
FMemoryTracking& FMemoryTracking::UsedMemory() {
    return MEMORYDOMAIN_TRACKING_DATA(UsedMemory);
}
//----------------------------------------------------------------------------
FMemoryTracking& FMemoryTracking::ReservedMemory() {
    return MEMORYDOMAIN_TRACKING_DATA(ReservedMemory);
}
//----------------------------------------------------------------------------
FMemoryTracking::FMemoryTracking(
    const char* optionalName /*= "unknown"*/,
    FMemoryTracking* optionalParent /*= nullptr*/)
    : _blockCount(0)
    , _allocationCount(0)
    , _totalSizeInBytes(0)
    , _maxBlockCount(0)
    , _maxAllocationCount(0)
    , _maxStrideInBytes(0)
    , _minStrideInBytes(UINT32_MAX)
    , _maxTotalSizeInBytes(0)
    , _parent(optionalParent)
    , _prev(nullptr)
    , _next(nullptr)
    , _name(optionalName)
    , _level(optionalParent ? optionalParent->_level + 1 : 0)
{}
//----------------------------------------------------------------------------
void FMemoryTracking::Allocate(size_t blockCount, size_t strideInBytes) {
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
void FMemoryTracking::Deallocate(size_t blockCount, size_t strideInBytes) {
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
void FMemoryTracking::ReleaseAll() {
    Release(_blockCount, _allocationCount, _totalSizeInBytes);
}
//----------------------------------------------------------------------------
void FMemoryTracking::Release(size_t blockCount, size_t allocationCount, size_t totalSizeInBytes) {
    Assert(_blockCount >= blockCount);
    Assert(_allocationCount >= allocationCount);
    Assert(_totalSizeInBytes >= totalSizeInBytes);

    _blockCount -= blockCount;
    _allocationCount -= allocationCount;
    _totalSizeInBytes -= totalSizeInBytes;

    if (_parent)
        _parent->Release(blockCount, allocationCount, totalSizeInBytes);
}
//----------------------------------------------------------------------------
void FMemoryTracking::Pool_AllocateOneBlock(size_t blockSizeInBytes) {
    ++_blockCount;

    _maxStrideInBytes = Max(_maxStrideInBytes.load(), blockSizeInBytes);
    _minStrideInBytes = Min(_minStrideInBytes.load(), blockSizeInBytes);

    if (_parent)
        _parent->Pool_AllocateOneBlock(blockSizeInBytes);
}
//----------------------------------------------------------------------------
void FMemoryTracking::Pool_DeallocateOneBlock(size_t blockSizeInBytes) {
    Assert(_blockCount >= 1);
    Assert(_maxStrideInBytes >= blockSizeInBytes);
    Assert(_minStrideInBytes <= blockSizeInBytes);

    --_blockCount;

    if (_parent)
        _parent->Pool_DeallocateOneBlock(blockSizeInBytes);
}
//----------------------------------------------------------------------------
void FMemoryTracking::Pool_AllocateOneChunk(size_t chunkSizeInBytes, size_t numBlocks) {
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
void FMemoryTracking::Pool_DeallocateOneChunk(size_t chunkSizeInBytes, size_t numBlocks) {
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
bool FMemoryTracking::IsChildOf(const FMemoryTracking& other) const {
    if (&other == this)
        return true;
    else if (_parent)
        return _parent->IsChildOf(other);
    else
        return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
