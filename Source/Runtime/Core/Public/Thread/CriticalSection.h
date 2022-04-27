#pragma once

#include "Core.h"

#include "HAL/PlatformThread.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FCriticalSection : Meta::FNonCopyableNorMovable {
public:
    FCriticalSection() { FPlatformThread::CreateCriticalSection(&_cs); }
    ~FCriticalSection() { FPlatformThread::DestroyCriticalSection(&_cs); }

    FCriticalSection(const FCriticalSection&) = delete;
    FCriticalSection& operator =(const FCriticalSection&) = delete;

    FCriticalSection(FCriticalSection&& rvalue) = delete;
    FCriticalSection& operator =(FCriticalSection&& rvalue) = delete;

    void Lock() const { FPlatformThread::Lock(_cs); }
    bool TryLock() const { return FPlatformThread::TryLock(_cs); }
    void Unlock() const { FPlatformThread::Unlock(_cs); }

    struct FScopeLock : Meta::FNonCopyableNorMovable {
        const FCriticalSection& CS;
        FScopeLock(const FCriticalSection* pcs) : CS(*pcs) {
            Assert(pcs);
            CS.Lock();
        }
        ~FScopeLock() { CS.Unlock(); }
        FScopeLock(const FScopeLock&) = delete;
        FScopeLock& operator =(const FScopeLock&) = delete;
    };

private:
    mutable FPlatformThread::FCriticalSection _cs{};
};
//----------------------------------------------------------------------------
using FCriticalScope = FCriticalSection::FScopeLock;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
