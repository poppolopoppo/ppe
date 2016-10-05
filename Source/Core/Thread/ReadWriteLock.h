#pragma once

#include "Core/Core.h"

#include <mutex>
#include <shared_mutex>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define READSCOPELOCK(_ReadWriteLock) \
    const auto CONCAT(readScopeLock_, __LINE__) = static_cast<const FReadWriteLock&>(_ReadWriteLock).Read()
//----------------------------------------------------------------------------
#define WRITESCOPELOCK(_ReadWriteLock) \
    const auto CONCAT(writeScopeLock_, __LINE__) = static_cast<FReadWriteLock&>(_ReadWriteLock).Write()
//----------------------------------------------------------------------------
#define DEFERREDREADSCOPELOCK(_ReadWriteLock) \
    const auto CONCAT(readScopeLock_, __LINE__) = static_cast<const FReadWriteLock&>(_ReadWriteLock).DeferredRead()
//----------------------------------------------------------------------------
#define DEFERREDWRITESCOPELOCK(_ReadWriteLock) \
    const auto CONCAT(writeScopeLock_, __LINE__) = static_cast<FReadWriteLock&>(_ReadWriteLock).DeferredWrite()
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
