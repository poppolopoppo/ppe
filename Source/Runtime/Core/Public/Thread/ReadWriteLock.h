#pragma once

#include "Core.h"

#include "HAL/PlatformThread.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define READSCOPELOCK(_ReadWriteLock) \
    const ::PPE::FReadWriteLock::FScopeLockRead ANONYMIZE(_scopeLockRead)(_ReadWriteLock)
//----------------------------------------------------------------------------
#define WRITESCOPELOCK(_ReadWriteLock) \
    const ::PPE::FReadWriteLock::FScopeLockWrite ANONYMIZE(_scopeLockWrite)(_ReadWriteLock)
//----------------------------------------------------------------------------
class FReadWriteLock : Meta::FNonCopyable {
public:
    FReadWriteLock() NOEXCEPT { FPlatformThread::CreateReadWriteLock(&_rwLock); }
    ~FReadWriteLock() NOEXCEPT { FPlatformThread::DestroyReadWriteLock(&_rwLock); }

    FReadWriteLock(FReadWriteLock&& ) NOEXCEPT { NOOP(); /* don't copy or move anything, but allow move */ }
    FReadWriteLock& operator =(FReadWriteLock&& ) NOEXCEPT = delete;

    void LockRead() const NOEXCEPT { FPlatformThread::LockRead(_rwLock); }
    bool TryLockRead() const NOEXCEPT { return FPlatformThread::TryLockRead(_rwLock); }
    void UnlockRead() const NOEXCEPT { FPlatformThread::UnlockRead(_rwLock); }

    void LockWrite() NOEXCEPT { FPlatformThread::LockWrite(_rwLock); }
    bool TryLockWrite() NOEXCEPT { return FPlatformThread::TryLockWrite(_rwLock); }
    void UnlockWrite() NOEXCEPT { FPlatformThread::UnlockWrite(_rwLock); }

    struct FScopeLockRead : Meta::FNonCopyableNorMovable {
        const FReadWriteLock& RWLock;
        FScopeLockRead(const FReadWriteLock& rwlock) NOEXCEPT : RWLock(rwlock) { RWLock.LockRead(); }
        ~FScopeLockRead() NOEXCEPT { RWLock.UnlockRead(); }
    };

    struct FScopeLockWrite : Meta::FNonCopyableNorMovable {
        FReadWriteLock& RWLock;
        FScopeLockWrite(FReadWriteLock& rwlock) NOEXCEPT : RWLock(rwlock) { RWLock.LockWrite(); }
        ~FScopeLockWrite() NOEXCEPT { RWLock.UnlockWrite(); }
    };

private:
    mutable FPlatformThread::FReadWriteLock _rwLock{};
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
