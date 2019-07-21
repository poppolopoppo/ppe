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
    : _name(optionalName)
    , _level(optionalParent ? optionalParent->_level + 1 : 0)
    , _parent(optionalParent)
    , Node{ nullptr, nullptr }
{}
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
void FMemoryTracking::Allocate(size_t userSize, size_t systemSize) NOEXCEPT {
    Assert(userSize);
    Assert(systemSize);
    Assert_NoAssume(userSize <= systemSize);
#if not USE_PPE_MEMORY_DEBUGGING // this is a performance consideration
    Assert_NoAssume(userSize * 2 > systemSize);
#endif

    _user.Allocate(userSize);
    _system.Allocate(systemSize);

    if (_parent)
        _parent->Allocate(userSize, systemSize);
}
//----------------------------------------------------------------------------
void FMemoryTracking::Deallocate(size_t userSize, size_t systemSize) NOEXCEPT {
    Assert(userSize);
    Assert(systemSize);
    Assert_NoAssume(userSize <= systemSize);

    _user.Deallocate(userSize);
    _system.Deallocate(systemSize);

    if (_parent)
        _parent->Deallocate(userSize, systemSize);
}
//----------------------------------------------------------------------------
void FMemoryTracking::AllocateUser(size_t size) NOEXCEPT {
    Assert(size);

    _user.Allocate(size);
    Assert_NoAssume(_user.TotalSize <= _system.TotalSize);

    if (_parent)
        _parent->AllocateUser(size);
}
//----------------------------------------------------------------------------
void FMemoryTracking::DeallocateUser(size_t size) NOEXCEPT {
    Assert(size);

    _user.Deallocate(size);

    if (_parent)
        _parent->DeallocateUser(size);
}
//----------------------------------------------------------------------------
void FMemoryTracking::AllocateSystem(size_t size) NOEXCEPT {
    Assert(size);

    _system.Allocate(size);
    Assert_NoAssume(_user.TotalSize <= _system.TotalSize);

    if (_parent)
        _parent->AllocateSystem(size);
}
//----------------------------------------------------------------------------
void FMemoryTracking::DeallocateSystem(size_t size) NOEXCEPT {
    Assert(size);

    _system.Deallocate(size);
    Assert_NoAssume(_user.TotalSize <= _system.TotalSize);

    if (_parent)
        _parent->DeallocateSystem(size);
}
//----------------------------------------------------------------------------
void FMemoryTracking::ReleaseBatch(size_t numAllocs, size_t userTotal, size_t systemTotal) NOEXCEPT {
    Assert(userTotal);
    Assert(systemTotal);
    Assert_NoAssume(userTotal <= systemTotal);

    _user.ReleaseBatch(numAllocs, userTotal);
    _system.ReleaseBatch(numAllocs, systemTotal);

    if (_parent)
        _parent->ReleaseBatch(numAllocs, userTotal, systemTotal);
}
//----------------------------------------------------------------------------
void FMemoryTracking::ReleaseBatchUser(size_t numAllocs, size_t totalSize) NOEXCEPT {
    Assert(numAllocs);
    Assert(totalSize);

    _user.ReleaseBatch(numAllocs, totalSize);

    if (_parent)
        _parent->ReleaseBatchUser(numAllocs, totalSize);
}
//----------------------------------------------------------------------------
void FMemoryTracking::ReleaseBatchSystem(size_t numAllocs, size_t totalSize) NOEXCEPT {
    Assert(numAllocs);
    Assert(totalSize);

    _system.ReleaseBatch(numAllocs, totalSize);
    Assert_NoAssume(_user.TotalSize <= _system.TotalSize);

    if (_parent)
        _parent->ReleaseBatchSystem(numAllocs, totalSize);
}
//----------------------------------------------------------------------------
void FMemoryTracking::ReleaseAllUser() NOEXCEPT {
    if (const size_t n = _user.NumAllocs) {
        const size_t sz = _user.TotalSize;
        Assert(sz);

        _user.ReleaseBatch(n, sz);
        Assert_NoAssume(_user.TotalSize <= _system.TotalSize);

        // DON'T call ReleaseAllUser() recursively since it would completely empty the parents !
        if (_parent)
            _parent->ReleaseBatchUser(n, sz);
    }
    else {
        Assert_NoAssume(0 == _user.TotalSize);
    }
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

    const size_t n = ++NumAllocs;

    if (Unlikely(n > PeakAllocs))
        PeakAllocs = n;

    ++AccumulatedAllocs;
    AccumulatedSize += s;
}
//----------------------------------------------------------------------------
void FMemoryTracking::FCounters_::Deallocate(size_t s) {
    Assert(NumAllocs);
    Assert_NoAssume(MinSize <= s);
    Assert_NoAssume(MaxSize >= s);
    Assert(TotalSize >= s);

    --NumAllocs;
    TotalSize -= s;
}
//----------------------------------------------------------------------------
void FMemoryTracking::FCounters_::ReleaseBatch(size_t n, size_t s) {
    Assert(NumAllocs >= n);
    Assert(TotalSize >= s);

    NumAllocs -= n;
    TotalSize -= s;
}
//----------------------------------------------------------------------------
auto FMemoryTracking::FCounters_::Snapshot() const -> FSnapshot {
    return FSnapshot{
        checked_cast<i64>(NumAllocs),
        checked_cast<i64>(MinSize <= MaxSize ? MinSize.load() : 0),
        checked_cast<i64>(MaxSize),
        checked_cast<i64>(TotalSize),
        checked_cast<i64>(PeakAllocs),
        checked_cast<i64>(PeakSize),
        checked_cast<i64>(AccumulatedAllocs),
        checked_cast<i64>(AccumulatedSize) };
}
//----------------------------------------------------------------------------
auto FMemoryTracking::FCounters_::Substract(const FCounters_& o) const -> FSnapshot {
    const FSnapshot lhs = Snapshot();
    const FSnapshot rhs = o.Snapshot();
    return FSnapshot{
        lhs.NumAllocs - rhs.NumAllocs,
        lhs.MinSize - rhs.MinSize,
        lhs.MaxSize - rhs.MaxSize,
        lhs.TotalSize - rhs.TotalSize,
        lhs.PeakAllocs - rhs.PeakAllocs,
        lhs.PeakSize - rhs.PeakSize,
        lhs.AccumulatedAllocs - rhs.AccumulatedAllocs,
        lhs.AccumulatedSize - rhs.AccumulatedSize };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
