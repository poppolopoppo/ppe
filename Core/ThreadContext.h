#pragma once

#include "Core.h"
#include "ThreadLocalSingleton.h"

#include <atomic>

// macro enable extension outside Core::
#define MAIN_THREADTAG      0
#define WORKER_THREADTAG    1

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ThreadContext {
protected:
    friend struct Meta::Activator<ThreadContext>;
    ThreadContext(const char* name, size_t tag, std::thread::id id);
    ~ThreadContext();

public:
    ThreadContext(const ThreadContext&) = delete;
    ThreadContext& operator =(const ThreadContext&) = delete;

    const char* Name() const { return _name; }
    size_t Tag() const { return _tag; }
    std::thread::id Id() const { return _id; }

private:
    char _name[64];
    const size_t _tag;
    const std::thread::id _id;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CurrentThreadContext : Meta::ThreadLocalSingleton<ThreadContext, CurrentThreadContext> {
    typedef Meta::ThreadLocalSingleton<ThreadContext, CurrentThreadContext> parent_type;
public:
    using parent_type::HasInstance;
    using parent_type::Instance;
    using parent_type::Destroy;

    static void Create(const char* name, size_t tag);
    static void CreateMainThread();
};
//----------------------------------------------------------------------------
inline void CurrentThreadContext::Create(const char* name, size_t tag) {
    parent_type::Create("Core::CurrentThreadContext", name, tag, std::this_thread::get_id());
}
//----------------------------------------------------------------------------
inline void CurrentThreadContext::CreateMainThread() {
    CurrentThreadContext::Create("MainThread", MAIN_THREADTAG);
}
//----------------------------------------------------------------------------
inline bool IsInMainThread() {
    Assert(CurrentThreadContext::HasInstance());
    return (MAIN_THREADTAG == CurrentThreadContext::Instance().Tag());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// ThreadContextStartup is the entry and exit point for every thread.
// Constructed with the same lifetime than every thread associated with.
//----------------------------------------------------------------------------
class ThreadContextStartup {
public:
    static void Start(const char* name, size_t tag);
    static void Start_MainThread();
    static void Shutdown();

    ThreadContextStartup(const char* name, size_t tag) { Start(name, tag); }
    ~ThreadContextStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
