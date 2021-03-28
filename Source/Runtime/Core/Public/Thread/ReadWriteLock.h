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
class FReadWriteLock {
public:
    FReadWriteLock() { FPlatformThread::CreateReadWriteLock(&_rwLock); }
    ~FReadWriteLock() { FPlatformThread::DestroyReadWriteLock(&_rwLock); }

    FReadWriteLock(const FReadWriteLock&) = delete;
    FReadWriteLock& operator =(const FReadWriteLock&) = delete;

    void LockRead() const { FPlatformThread::LockRead(_rwLock); }
    bool TryLockRead() const { return FPlatformThread::TryLockRead(_rwLock); }
    void UnlockRead() const { FPlatformThread::UnlockRead(_rwLock); }

    void LockWrite() { FPlatformThread::LockWrite(_rwLock); }
    bool TryLockWrite() { return FPlatformThread::TryLockWrite(_rwLock); }
    void UnlockWrite() { FPlatformThread::UnlockWrite(_rwLock); }

    struct FScopeLockRead : Meta::FNonCopyableNorMovable {
        const FReadWriteLock& RWLock;
        FScopeLockRead(const FReadWriteLock& rwlock) : RWLock(rwlock) { RWLock.LockRead(); }
        ~FScopeLockRead() { RWLock.UnlockRead(); }
        FScopeLockRead(const FScopeLockRead&) = delete;
        FScopeLockRead& operator =(const FScopeLockRead&) = delete;
    };

    struct FScopeLockWrite : Meta::FNonCopyableNorMovable {
        FReadWriteLock& RWLock;
        FScopeLockWrite(FReadWriteLock& rwlock) : RWLock(rwlock) { RWLock.LockWrite(); }
        ~FScopeLockWrite() { RWLock.UnlockWrite(); }
        FScopeLockWrite(const FScopeLockWrite&) = delete;
        FScopeLockWrite& operator =(const FScopeLockWrite&) = delete;
    };

private:
    mutable FPlatformThread::FReadWriteLock _rwLock;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
