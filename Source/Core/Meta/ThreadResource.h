#pragma once

#include "Core/Meta/Assert.h"
#include "Core/Meta/Cast.h"

#ifdef WITH_CORE_ASSERT
#   define WITH_CORE_THREADRESOURCE_CHECKS
#endif

#include <mutex>
#include <thread>

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ThreadResource {
public:
#ifdef WITH_CORE_THREADRESOURCE_CHECKS
    ThreadResource() : ThreadResource(std::this_thread::get_id()) {}
    explicit ThreadResource(std::thread::id threadId) : _threadId(threadId) {}
    ~ThreadResource() { Assert(std::this_thread::get_id() == _threadId); }
    std::thread::id ThreadId() const { return _threadId; }
    void CheckThreadId(std::thread::id threadId) const { Assert(threadId == _threadId); }
    void OwnedByThisThread() const { CheckThreadId(std::this_thread::get_id()); }
    void SetThreadId(std::thread::id threadId) { OwnedByThisThread(); _threadId = threadId; }
private:
    std::thread::id _threadId;
#else
    ThreadResource() {}
    ~ThreadResource() {}
    std::thread::id ThreadId() const { return std::thread::id(); }
    void CheckThreadId(std::thread::id) const {}
    void OwnedByThisThread() const {}
    void SetThreadId(std::thread::id ) {}
#endif
};
//----------------------------------------------------------------------------
#ifdef WITH_CORE_THREADRESOURCE_CHECKS
#   define THREADRESOURCE_CHECKACCESS(_pResource) (_pResource)->OwnedByThisThread()
#else
#   define THREADRESOURCE_CHECKACCESS(_pResource) NOOP
#endif
//----------------------------------------------------------------------------
#define THIS_THREADRESOURCE_CHECKACCESS() THREADRESOURCE_CHECKACCESS(this)
//----------------------------------------------------------------------------
#define THREADRESOURCE_CHECKACCESS_IFN(_pResource) if (_pResource) THREADRESOURCE_CHECKACCESS(_pResource)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class ThreadProxy : public ThreadResource {
public:
    ThreadProxy() {}
    ~ThreadProxy() {}

    ThreadProxy(const T& value) : _value(value) {}
    ThreadProxy(T&& rvalue) : _value(std::move(rvalue)) {}

    ThreadProxy(const ThreadProxy& ) = delete;
    ThreadProxy& operator =(const ThreadProxy& ) = delete;

    T& Value() { THIS_THREADRESOURCE_CHECKACCESS; return _value; }
    const T& Value() const { THIS_THREADRESOURCE_CHECKACCESS; return _value; }

    void SetValue(const T& value) { THIS_THREADRESOURCE_CHECKACCESS; _value = value; }
    void SetValue(T&& rvalue) { THIS_THREADRESOURCE_CHECKACCESS; _value = std::move(rvalue); }

private:
    T _value;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <bool _Lock>
class ThreadLock {
public:
    class ScopeLock {
    public:
        ScopeLock(ThreadLock&) {}
    };

    ThreadLock() {}
    ~ThreadLock() {}

    void Lock() {}
    bool TryLock() { return true; }
    void Unlock() {}
};
//----------------------------------------------------------------------------
template <>
class ThreadLock<true> {
public:
    class ScopeLock {
    public:
        ScopeLock(ThreadLock& owner) : _owner(&owner) { _owner->Lock(); }
        ~ScopeLock() { Assert(_owner); _owner->Unlock(); }
    private:
        ThreadLock *_owner;
    };

    ThreadLock() {}
    ~ThreadLock() {}

    void Lock() { _lock.lock(); }
    bool TryLock() { return _lock.try_lock(); }
    void Unlock() { _lock.unlock(); }

private:
    std::mutex _lock;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
