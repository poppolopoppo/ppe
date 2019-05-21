#include "stdafx.h"

#include "Memory/MemoryTracking.h"

#include "Memory/MemoryDomain.h"

namespace PPE {
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMemoryTracking::FMemoryTracking(
    const char* optionalName /*= "unknown"*/,
    FMemoryTracking* optionalParent /*= nullptr*/)
    : _numAllocs(0)
    , _peakAllocs(0)
    , _name(optionalName)
    , _level(optionalParent ? optionalParent->_level + 1 : 0)
    , _parent(optionalParent)
    , Node{ nullptr, nullptr }
{}
//----------------------------------------------------------------------------
void FMemoryTracking::Allocate(size_t userSize, size_t systemSize) NOEXCEPT {
    Assert(userSize);
    Assert(systemSize);
    Assert_NoAssume(userSize <= systemSize);

    const size_t n = (++_numAllocs);
    if (n > _peakAllocs)
        _peakAllocs = n;

    _user.Allocate(userSize);
    _system.Allocate(systemSize);

    if (_parent)
        _parent->Allocate(userSize, systemSize);
}
//----------------------------------------------------------------------------
void FMemoryTracking::Deallocate(size_t userSize, size_t systemSize) NOEXCEPT {
    Assert(userSize);
    Assert(systemSize);
    Assert(_numAllocs);
    Assert_NoAssume(userSize <= systemSize);

    --_numAllocs;

    _user.Deallocate(userSize);
    _system.Deallocate(systemSize);

    if (_parent)
        _parent->Deallocate(userSize, systemSize);
}
//----------------------------------------------------------------------------
void FMemoryTracking::ReleaseAll() NOEXCEPT {
    if (_numAllocs)
        ReleaseBatch(_numAllocs, _user.TotalSize, _system.TotalSize);
}
//----------------------------------------------------------------------------
void FMemoryTracking::ReleaseBatch(size_t numAllocs, size_t userTotal, size_t systemTotal) NOEXCEPT {
    Assert(userTotal);
    Assert(systemTotal);
    Assert(_numAllocs >= numAllocs);
    Assert_NoAssume(userTotal <= systemTotal);

    _numAllocs -= numAllocs;

    _user.ReleaseBatch(userTotal);
    _system.ReleaseBatch(systemTotal);

    if (_parent)
        _parent->ReleaseBatch(numAllocs, userTotal, systemTotal);
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
void FMemoryTracking::FCounters_::Allocate(size_t s) {
    const size_t total = (TotalSize += s);

    if (Unlikely(MinSize > s))
        MinSize = s;
    if (Unlikely(MaxSize < s))
        MaxSize = s;
    if (Unlikely(total > PeakSize))
        PeakSize = total;
}
//----------------------------------------------------------------------------
void FMemoryTracking::FCounters_::Deallocate(size_t s) {
    Assert(TotalSize >= s);
    Assert_NoAssume(MinSize <= s);
    Assert_NoAssume(MaxSize >= s);

    TotalSize -= s;
}
//----------------------------------------------------------------------------
void FMemoryTracking::FCounters_::ReleaseBatch(size_t s) {
    Assert(TotalSize >= s);

    TotalSize -= s;
}
//----------------------------------------------------------------------------
auto FMemoryTracking::FCounters_::Snapshot() const -> FSnapshot {
    return FSnapshot{
        MinSize <= MaxSize ? MinSize.load() : 0,
        MaxSize,
        PeakSize,
        TotalSize };
}
//----------------------------------------------------------------------------
auto FMemoryTracking::FCounters_::Substract(const FCounters_& o) const -> FSnapshot {
    const FSnapshot lhs = Snapshot();
    const FSnapshot rhs = o.Snapshot();
    return FSnapshot{
        rhs.MinSize - lhs.MinSize,
        lhs.MaxSize - rhs.MaxSize,
        lhs.PeakSize - rhs.PeakSize,
        lhs.TotalSize - rhs.TotalSize };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
