#pragma once

#include "Core.h"

#include "IO/TextWriter_fwd.h"
#include "HAL/PlatformThread.h"

#include <thread>

// macro enable extension outside PPE::
#define PPE_THREADTAG_MAIN                 size_t(0)
#define PPE_THREADTAG_WORKER               size_t(1)
#define PPE_THREADTAG_IO                   size_t(2)
#define PPE_THREADTAG_HIGHPRIORITY         size_t(3)
#define PPE_THREADTAG_BACKGROUND           size_t(4)
#define PPE_THREADTAG_OTHER                size_t(5)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FThreadContext {
public:
    FThreadContext(const char* name, size_t tag, size_t index);
    ~FThreadContext();

    FThreadContext(const FThreadContext&) = delete;
    FThreadContext& operator =(const FThreadContext&) = delete;

    const char *Name() const { return _name; }
    size_t Tag() const { return _tag; }
    size_t ThreadIndex() const { return _threadIndex; }
    std::thread::id ThreadId() const { return _threadId; }

    u64 AffinityMask() const;
    void SetAffinityMask(u64 mask) const;

    EThreadPriority Priority() const;
    void SetPriority(EThreadPriority priority) const;

    void DutyCycle() const;

    static size_t NumThreads();

    static size_t GetThreadHash(std::thread::id thread_id);
    static const char* GetThreadName(std::thread::id thread_id);

    static size_t MaxThreadIndex();

private:
    const size_t _tag;
    const size_t _threadIndex;
    const std::thread::id _threadId;
    char _name[64];
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API const FThreadContext& CurrentThreadContext();
//----------------------------------------------------------------------------
inline bool IsInMainThread()        { return (PPE_THREADTAG_MAIN == CurrentThreadContext().Tag()); }
inline bool IsInIOThread()          { return (PPE_THREADTAG_IO == CurrentThreadContext().Tag()); }
inline bool IsInWorkerThread()      { return (PPE_THREADTAG_WORKER == CurrentThreadContext().Tag()); }
inline bool IsInBackgroundThread()  { return (PPE_THREADTAG_BACKGROUND == CurrentThreadContext().Tag()); }
inline bool IsInOtherThread()       { return (PPE_THREADTAG_OTHER == CurrentThreadContext().Tag()); }
//----------------------------------------------------------------------------
#define AssertIsMainThread() Assert_NoAssume(PPE::IsInMainThread())
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FThreadContextStartup is the entry and exit point for every thread.
// Constructed with the same lifetime than every thread associated with.
//----------------------------------------------------------------------------
class PPE_CORE_API FThreadContextStartup {
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
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, std::thread::id thread_id);
//----------------------------------------------------------------------------
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, std::thread::id thread_id);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
