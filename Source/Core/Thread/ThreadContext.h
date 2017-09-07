#pragma once

#include "Core/Core.h"

#include <thread>

// macro enable extension outside Core::
#define CORE_THREADTAG_MAIN                 size_t(0)
#define CORE_THREADTAG_WORKER               size_t(1)
#define CORE_THREADTAG_IO                   size_t(2)
#define CORE_THREADTAG_LOWEST_PRIORITY      size_t(3)

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EThreadPriority {
    Highest = 0,
    AboveNormal,
    Normal,
    BelowNormal,
    Lowest,
};
//----------------------------------------------------------------------------
class FThreadContext {
public:
    FThreadContext(const char* name, size_t tag);
    ~FThreadContext();

    FThreadContext(const FThreadContext&) = delete;
    FThreadContext& operator =(const FThreadContext&) = delete;

    const char *Name() const { return _name; }
    size_t Tag() const { return _tag; }
    std::thread::id ThreadId() const { return _threadId; }

    size_t AffinityMask() const;
    void SetAffinityMask(size_t mask) const;

    EThreadPriority Priority() const;
    void SetPriority(EThreadPriority priority) const;

private:
    char _name[64];
    const size_t _tag;
    const std::thread::id _threadId;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FThreadContext& CurrentThreadContext();
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
inline bool IsInLowestPriorityThread() {
    return (CORE_THREADTAG_WORKER == CurrentThreadContext().Tag());
}
//----------------------------------------------------------------------------
#define AssertIsMainThread() Assert(Core::IsInMainThread())
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FThreadContextStartup is the entry and exit point for every thread.
// Constructed with the same lifetime than every thread associated with.
//----------------------------------------------------------------------------
class FThreadContextStartup {
public:
    static void Start(const char* name, size_t tag);
    static void Start_MainThread();
    static void Shutdown();

    FThreadContextStartup(const char* name, size_t tag) { Start(name, tag); }
    ~FThreadContextStartup() { Shutdown(); }

    const FThreadContext& Context() const { return CurrentThreadContext(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
