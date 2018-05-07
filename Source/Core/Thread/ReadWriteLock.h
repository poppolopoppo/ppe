#pragma once

#include "Core/Core.h"

#ifdef PLATFORM_WINDOWS

#   include "Core/Misc/Platform_Windows.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define READSCOPELOCK(_ReadWriteLock) \
    const ::Core::FReadWriteLock::FScopeLockRead ANONYMIZE(_scopeLockRead)(_ReadWriteLock)
//----------------------------------------------------------------------------
#define WRITESCOPELOCK(_ReadWriteLock) \
    const ::Core::FReadWriteLock::FScopeLockWrite ANONYMIZE(_scopeLockWrite)(_ReadWriteLock)
//----------------------------------------------------------------------------
class FReadWriteLock {
public:
    FReadWriteLock() { ::InitializeSRWLock(&_srwLock); }

    FReadWriteLock(const FReadWriteLock&) = delete;
    FReadWriteLock& operator =(const FReadWriteLock&) = delete;

    void LockRead() const { ::AcquireSRWLockShared(&_srwLock); }
    bool TryLockRead() const { return ::TryAcquireSRWLockShared(&_srwLock); }
    void UnlockRead() const { ::ReleaseSRWLockShared(&_srwLock); }

    void LockWrite() { ::AcquireSRWLockExclusive(&_srwLock); }
    bool TryLockWrite() { return ::TryAcquireSRWLockExclusive(&_srwLock); }
    void UnlockWrite() { ::ReleaseSRWLockExclusive(&_srwLock); }

    struct FScopeLockRead {
        const FReadWriteLock& RWLock;
        FScopeLockRead(const FReadWriteLock& rwlock) : RWLock(rwlock) { RWLock.LockRead(); }
        ~FScopeLockRead() { RWLock.UnlockRead(); }
        FScopeLockRead(const FScopeLockRead&) = delete;
        FScopeLockRead& operator =(const FScopeLockRead&) = delete;
    };

    struct FScopeLockWrite {
        FReadWriteLock& RWLock;
        FScopeLockWrite(FReadWriteLock& rwlock) : RWLock(rwlock) { RWLock.LockWrite(); }
        ~FScopeLockWrite() { RWLock.UnlockWrite(); }
        FScopeLockWrite(const FScopeLockWrite&) = delete;
        FScopeLockWrite& operator =(const FScopeLockWrite&) = delete;
    };

private:
    mutable ::SRWLOCK _srwLock;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#else
#   error "unsupported platform !"
#endif //!PLATFORM_WINDOWS

#if 0 //deprecated

#include <mutex>
#include <shared_mutex>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define READSCOPELOCK(_ReadWriteLock) \
    const auto ANONYMIZE(readScopeLock_) = static_cast<const FReadWriteLock&>(_ReadWriteLock).Read()
//----------------------------------------------------------------------------
#define WRITESCOPELOCK(_ReadWriteLock) \
    const auto ANONYMIZE(writeScopeLock_) = static_cast<FReadWriteLock&>(_ReadWriteLock).Write()
//----------------------------------------------------------------------------
#define DEFERREDREADSCOPELOCK(_ReadWriteLock) \
    const auto ANONYMIZE(readScopeLock_) = static_cast<const FReadWriteLock&>(_ReadWriteLock).DeferredRead()
//----------------------------------------------------------------------------
#define DEFERREDWRITESCOPELOCK(_ReadWriteLock) \
    const auto ANONYMIZE(writeScopeLock_) = static_cast<FReadWriteLock&>(_ReadWriteLock).DeferredWrite()
//----------------------------------------------------------------------------
struct FReadWriteLock {
    typedef std::shared_timed_mutex mutex_type;

    typedef std::shared_lock<mutex_type> readscope_type;
    typedef std::unique_lock<mutex_type> writescope_type;

    mutable mutex_type Mutex;

    readscope_type Read() const { return readscope_type(Mutex, std::adopt_lock); }
    writescope_type Write() { return writescope_type(Mutex, std::adopt_lock); }

    readscope_type DeferredRead() const { return readscope_type(Mutex, std::defer_lock); }
    writescope_type DeferredWrite() { return writescope_type(Mutex, std::defer_lock); }

    readscope_type TryRead() const { return readscope_type(Mutex, std::try_to_lock); }
    writescope_type TryWrite() { return writescope_type(Mutex, std::try_to_lock); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!deprecated