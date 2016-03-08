#pragma once

#include "Core/Core.h"

#include <thread>

// macro enable extension outside Core::
#define MAIN_THREADTAG      0
#define WORKER_THREADTAG    1

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ThreadContext {
public:
    ThreadContext(const char *name, size_t tag, std::thread::id id);
    ~ThreadContext();

    ThreadContext(const ThreadContext&) = delete;
    ThreadContext& operator =(const ThreadContext&) = delete;

    const char *Name() const { return _name; }
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
const ThreadContext& ThisThreadContext();
//----------------------------------------------------------------------------
inline bool IsInMainThread() {
    return (MAIN_THREADTAG == ThisThreadContext().Tag());
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
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
