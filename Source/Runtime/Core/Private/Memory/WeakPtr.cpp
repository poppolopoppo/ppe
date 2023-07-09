// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Memory/WeakPtr.h"

#include "HAL/PlatformProcess.h"
#include "Thread/AtomicSpinLock.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PWeakRefCounter FWeakRefCounter::Allocate(deleter_func deleter ARGS_IF_ASSERT(FWeakRefCountable* holderForDebug)) {
    Assert(deleter);
    Assert_NoAssume(holderForDebug);
    return NEW_REF(WeakRef, FWeakRefCounter, deleter ARGS_IF_ASSERT(holderForDebug));
}
//----------------------------------------------------------------------------
bool FWeakRefCounter::TryLockForWeakPtr() NOEXCEPT {
    i32 backoff = 0;
    for (int expected = _strongRefCount.load(std::memory_order_relaxed);;) {
        if (expected <= 0)
            return false;

        if (_strongRefCount.compare_exchange_weak(expected, expected + 1,
            std::memory_order_release, std::memory_order_relaxed))
            return true;

        FPlatformProcess::SleepForSpinning(backoff);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
