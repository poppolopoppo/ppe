#include "stdafx.h"

#include "Memory/WeakPtr.h"

#include "HAL/PlatformProcess.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
PWeakRefCounter FWeakRefCounter::Allocate(FWeakRefCountable* holder, deleter_func deleter) {
    Assert(holder);
    Assert(deleter);
    return NEW_REF(WeakRef, FWeakRefCounter, holder, deleter);
}
#else
PWeakRefCounter FWeakRefCounter::Allocate(deleter_func deleter) {
    Assert(deleter);
    return NEW_REF(WeakRef, FWeakRefCounter, deleter);
}
#endif
//----------------------------------------------------------------------------
bool FWeakRefCounter::TryLockForWeakPtr() {
    size_t backoff = 0;
    for (;;) {
        int expected = _strongRefCount;

        if (0 >= expected)
            return false;
        else if (_strongRefCount.compare_exchange_weak(expected, expected + 1,
            std::memory_order_release, std::memory_order_relaxed))
            return true;

        FPlatformProcess::SleepForSpinning(backoff);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
