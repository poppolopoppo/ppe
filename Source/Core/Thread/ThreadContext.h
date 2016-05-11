#pragma once

#include "Core/Core.h"

#include <thread>

// macro enable extension outside Core::
#define CORE_THREADTAG_MAIN     0
#define CORE_THREADTAG_WORKER   1
#define CORE_THREADTAG_IO       2

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ThreadContext {
public:
    ThreadContext(const char* name, size_t tag);
    ~ThreadContext();

    ThreadContext(const ThreadContext&) = delete;
    ThreadContext& operator =(const ThreadContext&) = delete;

    const char *Name() const { return _name; }
    size_t Tag() const { return _tag; }
    std::thread::id ThreadId() const { return _threadId; }

    size_t AffinityMask() const;
    void SetAffinityMask(size_t mask) const;

private:
    char _name[64];
    const size_t _tag;
    const std::thread::id _threadId;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const ThreadContext& CurrentThreadContext();
//----------------------------------------------------------------------------
inline bool IsInMainThread() {
    return (CORE_THREADTAG_MAIN == CurrentThreadContext().Tag());
}
//----------------------------------------------------------------------------
inline bool IsInIOThread() {
    return (CORE_THREADTAG_IO == CurrentThreadContext().Tag());
}
//----------------------------------------------------------------------------
inline bool IsInWorkerThread() {
    return (CORE_THREADTAG_WORKER == CurrentThreadContext().Tag());
}
//----------------------------------------------------------------------------
#define AssertIsMainThread() Assert(Core::IsInMainThread())
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

    const ThreadContext& Context() const { return CurrentThreadContext(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
