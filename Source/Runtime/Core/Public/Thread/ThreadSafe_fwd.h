#pragma once

#include "Core_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EThreadBarrier {
    None = 0,
    // blocking mutexes
    CriticalSection,
    RWLock,
    // atomic spin locking
    AtomicSpinLock,
    AtomicReadWriteLock,
    AtomicTicketRWLock,
#if USE_PPE_DEBUG || USE_PPE_FASTDEBUG
    // data race debugging
    DataRaceCheck,
    RWDataRaceCheck,
    // thread ownership
    ThreadLocal,
#else
    // disable those checks when not debugging
    DataRaceCheck = None,
    RWDataRaceCheck = None,
    ThreadLocal = None,
#endif
};
//----------------------------------------------------------------------------
CONSTEXPR bool EThreadBarrier_Safe(EThreadBarrier barrier) NOEXCEPT {
    switch (barrier) {
    case EThreadBarrier::None:
#if USE_PPE_DEBUG || USE_PPE_FASTDEBUG
    case EThreadBarrier::ThreadLocal:
#endif
        return false;
    case EThreadBarrier::CriticalSection:
    case EThreadBarrier::RWLock:
    case EThreadBarrier::AtomicSpinLock:
    case EThreadBarrier::AtomicReadWriteLock:
    case EThreadBarrier::AtomicTicketRWLock:
#if USE_PPE_DEBUG || USE_PPE_FASTDEBUG
    case EThreadBarrier::DataRaceCheck:
    case EThreadBarrier::RWDataRaceCheck:
#endif
        return true;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
template <typename T, EThreadBarrier _Barrier>
class TThreadSafe;
//----------------------------------------------------------------------------
template <EThreadBarrier _Barrier, typename T>
TThreadSafe<T, _Barrier> MakeThreadSafe(T&& rvalue) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
