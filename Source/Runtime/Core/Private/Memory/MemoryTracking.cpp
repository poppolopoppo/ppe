#include "stdafx.h"

#include "Memory/MemoryTracking.h"

#include "Memory/MemoryDomain.h"

#define USE_PPE_MEMORY_WARN_IF_MANY_SMALLALLOCS (USE_PPE_PLATFORM_DEBUG && !USE_PPE_MEMORY_DEBUGGING)

#if USE_PPE_MEMORY_WARN_IF_MANY_SMALLALLOCS
#   include "Diagnostic/CurrentProcess.h"
#   include "Diagnostic/IgnoreList.h"
#   include "Diagnostic/Logger.h"
#   include "HAL/PlatformDebug.h"
#   include "IO/FormatHelpers.h"
#   include "Thread/CriticalSection.h"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool ShouldTrackRecursively_(const FMemoryTracking& trackingData) {
    return (!!trackingData.Parent() && (FMemoryTracking::Recursive == trackingData.Mode()));
}
//----------------------------------------------------------------------------
#if USE_PPE_MEMORY_WARN_IF_MANY_SMALLALLOCS
LOG_CATEGORY(, MemoryTracking)
STATIC_CONST_INTEGRAL(size_t, SmallAllocationCountWarning, 2000);
STATIC_CONST_INTEGRAL(size_t, SmallAllocationPercentThreshold, 5);
STATIC_CONST_INTEGRAL(size_t, SmallAllocationSizeThreshold, CODE3264(16_b, 32_b));
static void PPE_DEBUG_SECTION NO_INLINE WarnAboutSmallAllocs_(const FMemoryTracking& domain, const FMemoryTracking::FCounters& system) {
    // #TODO: WeakRef is a special case and should be optimized, but right now we ignore the warnings from this domain:
    if (&MEMORYDOMAIN_TRACKING_DATA(WeakRef) == &domain)
        return;

    const size_t totalAllocs = system.AccumulatedAllocs.load(std::memory_order_relaxed);
    const size_t smallAllocs = system.SmallAllocs.load(std::memory_order_relaxed);

    const float smallPercent = ((smallAllocs * 100.0f) / totalAllocs);
    if (smallPercent < SmallAllocationPercentThreshold) // bail if less than N% of all allocations are small
        return;

    LOG(MemoryTracking, Warning,
        L"Too many small allocations for memory domain <{0}> ({1} / {2} = {3})",
        MakeCStringView(domain.Name()),
        Fmt::CountOfElements(smallAllocs),
        Fmt::CountOfElements(totalAllocs),
        Fmt::Percentage(smallAllocs, totalAllocs) );

    if (FCurrentProcess::StartedWithDebugger()) {
        static FCriticalSection GBarrier;
        const FCriticalScope scopeLock(&GBarrier);

#   if USE_PPE_IGNORELIST
        FIgnoreList::FIgnoreKey ignoreKey;
        ignoreKey << MakeStringView("SmallAllocsWarnings") << MakeCStringView(domain.Name());
        if (FIgnoreList::HitIFP(ignoreKey) > 0)
            return; // already ignored
#   endif

        FLUSH_LOG();
        PPE_DEBUG_BREAK();

#   if USE_PPE_IGNORELIST
        if (volatile bool ignoreFurtherWarnings = false)
            FIgnoreList::AddIFP(ignoreKey);
#   endif
    }
}
#endif
//----------------------------------------------------------------------------
} //!namespace
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
    FMemoryTracking* optionalParent /*= nullptr*/,
    EMode mode/* = Recursive */) NOEXCEPT
:   Node{ nullptr, nullptr } {
    Reparent(optionalName, optionalParent, mode);
}
//----------------------------------------------------------------------------
bool FMemoryTracking::IsChildOf(const FMemoryTracking& other) const {
    if (&other == this)
        return true;
    if (_parent.Get())
        return _parent->IsChildOf(other);

    return false;
}
//----------------------------------------------------------------------------
void FMemoryTracking::Allocate(size_t userSize, size_t systemSize, const FMemoryTracking* child/* = nullptr */) NOEXCEPT {
    Assert(userSize);
    Assert(systemSize);
    Assert_NoAssume(userSize <= systemSize);
    UNUSED(child);

#if not USE_PPE_MEMORY_DEBUGGING // this is a performance consideration
    Assert_NoAssume(systemSize <= ALLOCATION_BOUNDARY || userSize * 2 > systemSize);
#endif

    _user.Allocate(userSize);
    _system.Allocate(systemSize);

#if USE_PPE_MEMORY_WARN_IF_MANY_SMALLALLOCS
    if (Unlikely((!child) & (systemSize <= SmallAllocationSizeThreshold) & (_system.SmallAllocs.load(std::memory_order_relaxed) >= SmallAllocationCountWarning)))
        WarnAboutSmallAllocs_(*this, _system);
#endif

    if (ShouldTrackRecursively_(*this))
        _parent->Allocate(userSize, systemSize, this);
}
//----------------------------------------------------------------------------
void FMemoryTracking::Deallocate(size_t userSize, size_t systemSize, const FMemoryTracking*/* = nullptr */) NOEXCEPT {
    Assert(userSize);
    Assert(systemSize);
    Assert_NoAssume(userSize <= systemSize);

    _user.Deallocate(userSize);
    _system.Deallocate(systemSize);

    if (ShouldTrackRecursively_(*this))
        _parent->Deallocate(userSize, systemSize, this);
}
//----------------------------------------------------------------------------
void FMemoryTracking::AllocateUser(size_t size, const FMemoryTracking*/* = nullptr */) NOEXCEPT {
    Assert(size);

    _user.Allocate(size);

    if (ShouldTrackRecursively_(*this))
        _parent->AllocateUser(size, this);

    Assert_NoAssume(_user.TotalSize <= _system.TotalSize);
}
//----------------------------------------------------------------------------
void FMemoryTracking::DeallocateUser(size_t size, const FMemoryTracking*/* = nullptr */) NOEXCEPT {
    Assert(size);

    _user.Deallocate(size);

    if (ShouldTrackRecursively_(*this))
        _parent->DeallocateUser(size, this);

    Assert_NoAssume(_user.TotalSize <= _system.TotalSize);
}
//----------------------------------------------------------------------------
void FMemoryTracking::AllocateSystem(size_t size, const FMemoryTracking* child/* = nullptr */) NOEXCEPT {
    Assert(size);
    UNUSED(child);

    _system.Allocate(size);

    if (ShouldTrackRecursively_(*this))
        _parent->AllocateSystem(size, this);

    Assert_NoAssume(_user.TotalSize <= _system.TotalSize);

#if USE_PPE_MEMORY_WARN_IF_MANY_SMALLALLOCS
    if (Unlikely((!child) & (size <= SmallAllocationSizeThreshold) & (_system.SmallAllocs.load(std::memory_order_relaxed) >= SmallAllocationCountWarning)))
        WarnAboutSmallAllocs_(*this, _system);
#endif
}
//----------------------------------------------------------------------------
void FMemoryTracking::DeallocateSystem(size_t size, const FMemoryTracking*/* = nullptr */) NOEXCEPT {
    Assert(size);

    _system.Deallocate(size);

    if (ShouldTrackRecursively_(*this))
        _parent->DeallocateSystem(size, this);

    Assert_NoAssume(_user.TotalSize <= _system.TotalSize);
}
//----------------------------------------------------------------------------
void FMemoryTracking::ReleaseBatch(size_t numAllocs, size_t userTotal, size_t systemTotal, const FMemoryTracking*/* = nullptr */) NOEXCEPT {
    Assert(userTotal);
    Assert(systemTotal);
    Assert_NoAssume(userTotal <= systemTotal);

    _user.ReleaseBatch(numAllocs, userTotal);
    _system.ReleaseBatch(numAllocs, systemTotal);

    if (ShouldTrackRecursively_(*this))
        _parent->ReleaseBatch(numAllocs, userTotal, systemTotal, this);
}
//----------------------------------------------------------------------------
void FMemoryTracking::ReleaseBatchUser(size_t numAllocs, size_t totalSize, const FMemoryTracking*/* = nullptr */) NOEXCEPT {
    Assert(numAllocs);
    Assert(totalSize);

    _user.ReleaseBatch(numAllocs, totalSize);

    if (ShouldTrackRecursively_(*this))
        _parent->ReleaseBatchUser(numAllocs, totalSize, this);
}
//----------------------------------------------------------------------------
void FMemoryTracking::ReleaseBatchSystem(size_t numAllocs, size_t totalSize, const FMemoryTracking*/* = nullptr */) NOEXCEPT {
    Assert(numAllocs);
    Assert(totalSize);

    _system.ReleaseBatch(numAllocs, totalSize);

    if (ShouldTrackRecursively_(*this))
        _parent->ReleaseBatchSystem(numAllocs, totalSize, this);

    Assert_NoAssume(_user.TotalSize <= _system.TotalSize);
}
//----------------------------------------------------------------------------
void FMemoryTracking::ReleaseAllUser(const FMemoryTracking*/* = nullptr */) NOEXCEPT {
    if (const size_t n = _user.NumAllocs) {
        const size_t sz = _user.TotalSize;
        Assert(sz);

        _user.ReleaseBatch(n, sz);

        // DON'T call ReleaseAllUser() recursively since it would completely empty the parents !
        if (ShouldTrackRecursively_(*this))
            _parent->ReleaseBatchUser(n, sz, this);

        Assert_NoAssume(_user.TotalSize <= _system.TotalSize);
    }
    else {
        Assert_NoAssume(0 == _user.TotalSize);
    }
}
//----------------------------------------------------------------------------
void FMemoryTracking::MoveTo(FMemoryTracking* dst) NOEXCEPT {
    Assert(dst);

    dst->_system.ResetAt(_system.Snapshot());
    dst->_user.ResetAt(_user.Snapshot());

    _system.ResetAt(Meta::MakeForceInit<FSnapshot>());
    _user.ResetAt(Meta::MakeForceInit<FSnapshot>());
}
//----------------------------------------------------------------------------
void FMemoryTracking::Swap(FMemoryTracking& other) NOEXCEPT {
    const FSnapshot sys = _system.Snapshot();
    const FSnapshot usr = _user.Snapshot();

    _system.ResetAt(other._system.Snapshot());
    _user.ResetAt(other._user.Snapshot());

    other._system.ResetAt(sys);
    other._user.ResetAt(usr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FMemoryTracking::FCounters::Allocate(size_t s) {
    std::atomic_thread_fence(std::memory_order_release);

    const size_t total = (TotalSize += s);

    if (Unlikely(MinSize > s))
        MinSize = s;
    if (Unlikely(MaxSize < s))
        MaxSize = s;
    if (Unlikely(total > PeakSize))
        PeakSize = total;

    const size_t n{ NumAllocs.fetch_add(1, std::memory_order_relaxed) + 1 };

    if (Unlikely(n > PeakAllocs))
        PeakAllocs = n;

    AccumulatedAllocs.fetch_add(1, std::memory_order_relaxed);
    AccumulatedSize.fetch_add(s, std::memory_order_relaxed);

#if USE_PPE_MEMORY_WARN_IF_MANY_SMALLALLOCS
    if (s <= SmallAllocationSizeThreshold)
        SmallAllocs.fetch_add(1, std::memory_order_relaxed);
#endif

    std::atomic_thread_fence(std::memory_order_acquire);
}
//----------------------------------------------------------------------------
void FMemoryTracking::FCounters::Deallocate(size_t s) {
    Assert_NoAssume(MinSize.load(std::memory_order_relaxed) <= s);
    Assert_NoAssume(MaxSize.load(std::memory_order_relaxed) >= s);

    Verify(NumAllocs.fetch_sub(1, std::memory_order_relaxed) > 0);
    Verify(TotalSize.fetch_sub(s, std::memory_order_relaxed) >= s);

    std::atomic_thread_fence(std::memory_order_acquire);
}
//----------------------------------------------------------------------------
void FMemoryTracking::FCounters::ReleaseBatch(size_t n, size_t s) {
    Verify(NumAllocs.fetch_sub(n, std::memory_order_relaxed) >= n);
    Verify(TotalSize.fetch_sub(s, std::memory_order_relaxed) >= s);

    std::atomic_thread_fence(std::memory_order_acquire);
}
//----------------------------------------------------------------------------
auto FMemoryTracking::FCounters::Snapshot() const -> FSnapshot {
    return FSnapshot{
        checked_cast<i64>(NumAllocs.load(std::memory_order_relaxed)),
        checked_cast<i64>(MinSize <= MaxSize ? MinSize.load(std::memory_order_relaxed) : 0),
        checked_cast<i64>(MaxSize.load(std::memory_order_relaxed)),
        checked_cast<i64>(TotalSize.load(std::memory_order_relaxed)),
        checked_cast<i64>(PeakAllocs.load(std::memory_order_relaxed)),
        checked_cast<i64>(PeakSize.load(std::memory_order_relaxed)),
        checked_cast<i64>(AccumulatedAllocs.load(std::memory_order_relaxed)),
        checked_cast<i64>(AccumulatedSize.load(std::memory_order_relaxed)),
        checked_cast<i64>(SmallAllocs.load(std::memory_order_relaxed))
    };
}
//----------------------------------------------------------------------------
auto FMemoryTracking::FCounters::Difference(const FCounters& o) const -> FSnapshot {
    const FSnapshot lhs = Snapshot();
    const FSnapshot rhs = o.Snapshot();

    std::atomic_thread_fence(std::memory_order_acquire);

    return FSnapshot{
        lhs.NumAllocs - rhs.NumAllocs,
        lhs.MinSize - rhs.MinSize,
        lhs.MaxSize - rhs.MaxSize,
        lhs.TotalSize - rhs.TotalSize,
        lhs.PeakAllocs - rhs.PeakAllocs,
        lhs.PeakSize - rhs.PeakSize,
        lhs.AccumulatedAllocs - rhs.AccumulatedAllocs,
        lhs.AccumulatedSize - rhs.AccumulatedSize,
        lhs.SmallAllocs - rhs.SmallAllocs
    };
}
//----------------------------------------------------------------------------
void FMemoryTracking::FCounters::ResetAt(const FSnapshot& snapshot) {
    NumAllocs.store(checked_cast<size_t>(snapshot.NumAllocs), std::memory_order_relaxed);
    MinSize.store(checked_cast<size_t>(snapshot.MinSize), std::memory_order_relaxed);
    MaxSize.store(checked_cast<size_t>(snapshot.MaxSize), std::memory_order_relaxed);
    TotalSize.store(checked_cast<size_t>(snapshot.TotalSize), std::memory_order_relaxed);
    PeakAllocs.store(checked_cast<size_t>(snapshot.PeakAllocs), std::memory_order_relaxed);
    PeakSize.store(checked_cast<size_t>(snapshot.PeakSize), std::memory_order_relaxed);
    AccumulatedAllocs.store(checked_cast<size_t>(snapshot.AccumulatedAllocs), std::memory_order_relaxed);
    AccumulatedSize.store(checked_cast<size_t>(snapshot.AccumulatedSize), std::memory_order_relaxed);
    SmallAllocs.store(checked_cast<size_t>(snapshot.SmallAllocs), std::memory_order_relaxed);

    std::atomic_thread_fence(std::memory_order_acquire);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
