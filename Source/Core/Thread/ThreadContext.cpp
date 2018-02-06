#include "stdafx.h"

#include "ThreadContext.h"

#include "Allocator/Alloca.h"
#include "Allocator/ThreadLocalHeap.h"
#include "Diagnostic/LastError.h"
#include "Diagnostic/Logger.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "Meta/AutoSingleton.h"
#include "Meta/NumericLimits.h"

#ifndef FINAL_RELEASE
#   define WITH_CORE_THREADCONTEXT_NAME
#endif

#ifdef WITH_CORE_THREADCONTEXT_NAME
#   include "Container/AssociativeVector.h"
#   include "Thread/ReadWriteLock.h"
#endif

#ifdef PLATFORM_WINDOWS
#   include "Misc/Platform_Windows.h"
#endif

namespace Core {
LOG_CATEGORY(CORE_API, Thread);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static std::atomic<size_t> GNumThreadContext_{ 0 };
//----------------------------------------------------------------------------
class FThreadLocalContext_ : Meta::TThreadLocalSingleton<FThreadContext, FThreadLocalContext_> {
    typedef Meta::TThreadLocalSingleton<FThreadContext, FThreadLocalContext_> parent_type;
public:
#ifdef WITH_CORE_ASSERT
    using parent_type::HasInstance;
#endif
    using parent_type::Instance;
    using parent_type::Destroy;

    static void Create(const char* name, size_t tag);
    static void CreateMainThread();
};
//----------------------------------------------------------------------------
inline void FThreadLocalContext_::Create(const char* name, size_t tag) {
    const size_t threadIndex = (GNumThreadContext_++);
    AssertRelease(threadIndex < CORE_MAX_CORES);
    GCurrentThreadIndex = threadIndex;
    parent_type::Create(name, tag, threadIndex);
}
//----------------------------------------------------------------------------
inline void FThreadLocalContext_::CreateMainThread() {
    FThreadLocalContext_::Create("MainThread", CORE_THREADTAG_MAIN);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#ifdef WITH_CORE_THREADCONTEXT_NAME
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(6320) // L'expression de filtre d'exception correspond a la constante EXCEPTION_EXECUTE_HANDLER.
                                  // Cela risque de masquer les exceptions qui n'etaient pas destinees a etre gerees.
PRAGMA_MSVC_WARNING_DISABLE(6322) // bloc empty _except.
static void SetWin32ThreadName_(const char* name) {
    /*
    // How to: Set a Thread FName in Native Code
    // http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
    */
    const DWORD MS_VC_EXCEPTION = 0x406D1388;
#   pragma pack(push,8)
    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType; // Must be 0x1000.
        LPCSTR szName; // Pointer to name (in user addr space).
        DWORD dwThreadID; // Thread ID (-1=caller thread).
        DWORD dwFlags; // Reserved for future use, must be zero.
    }   THREADNAME_INFO;
#   pragma pack(pop)

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = checked_cast<DWORD>(-1);
    info.dwFlags = 0;

    __try
    {
        ::RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
}
PRAGMA_MSVC_WARNING_POP()
#endif //!WITH_CORE_THREADCONTEXT_NAME
//----------------------------------------------------------------------------
static NO_INLINE void GuaranteeStackSizeForStackOverflowRecovery_() {
#ifdef PLATFORM_WINDOWS
    ULONG stackSizeInBytes = 0;
    if(::SetThreadStackGuarantee(&stackSizeInBytes)) {
        stackSizeInBytes += 64*1024;
        if (::SetThreadStackGuarantee(&stackSizeInBytes))
            return;
    }
    LOG(Thread, Warning, L"unable to SetThreadStackGuarantee, TStack Overflows won't be caught properly !");
#endif
}
//----------------------------------------------------------------------------
#ifdef WITH_CORE_THREADCONTEXT_NAME
struct FThreadNames_ {
    FReadWriteLock RWLock;
    ASSOCIATIVE_VECTORINSITU(Diagnostic, std::thread::id, FStringView, 32) Names;
    static FThreadNames_& Instance() {
        ONE_TIME_DEFAULT_INITIALIZE(FThreadNames_, GInstance);
        return GInstance;
    }
};
static FStringView GetThreadName_(std::thread::id thread_id) {
    FThreadNames_& thread_names = FThreadNames_::Instance();
    READSCOPELOCK(thread_names.RWLock);
    const auto it = thread_names.Names.Find(thread_id);
    return (thread_names.Names.end() == it ? "UnkownThread" : it->second);
}
#endif //!WITH_CORE_THREADCONTEXT_NAME
//----------------------------------------------------------------------------
static void RegisterThreadName_(std::thread::id thread_id, const char* name) {
#ifdef WITH_CORE_THREADCONTEXT_NAME
    SetWin32ThreadName_(name);
    FThreadNames_& thread_names = FThreadNames_::Instance();
    WRITESCOPELOCK(thread_names.RWLock);
    thread_names.Names.Insert_AssertUnique(thread_id, MakeCStringView(name));
#else
    UNUSED(thread_id);
    UNUSED(name);
#endif
}
//----------------------------------------------------------------------------
static void UnregisterThreadName_(std::thread::id thread_id) {
#ifdef WITH_CORE_THREADCONTEXT_NAME
    FThreadNames_& thread_names = FThreadNames_::Instance();
    WRITESCOPELOCK(thread_names.RWLock);
    thread_names.Names.Remove_AssertExists(thread_id);
#else
    UNUSED(thread_id);
#endif
}
//----------------------------------------------------------------------------
static auto GetThreadHash_(std::thread::id id) {
    union UThreadIdIntegral_ {
        std::thread::id ThreadId;
        TIntegral<std::thread::id>::type Integral;
    };
    return UThreadIdIntegral_{ id }.Integral;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FThreadContext::FThreadContext(const char* name, size_t tag, size_t index)
:   _tag(tag)
,   _threadIndex(index)
,   _threadId(std::this_thread::get_id()) {
    Assert(name);

    const size_t n = Copy(MakeView(_name), MakeCStringView(name));
    Assert(n < lengthof(_name));
    _name[n] = '\0';

    RegisterThreadName_(_threadId, _name);
    GuaranteeStackSizeForStackOverflowRecovery_();
}
//----------------------------------------------------------------------------
FThreadContext::~FThreadContext() {
    UnregisterThreadName_(_threadId);
}
//----------------------------------------------------------------------------
size_t FThreadContext::AffinityMask() const {
    Assert(std::this_thread::get_id() == _threadId);
#ifdef PLATFORM_WINDOWS
    HANDLE hThread = ::GetCurrentThread();
    DWORD_PTR affinityMask = ::SetThreadAffinityMask(hThread, 0xFFul);
    if (0 == affinityMask) {
        CORE_THROW_IT(FLastErrorException());
    }
    if (0 == ::SetThreadAffinityMask(hThread, affinityMask)) {
        FLastErrorException e;
        CORE_THROW_IT(FLastErrorException());
    }
    return checked_cast<size_t>(affinityMask);
#else
#   error "platform not supported"
#endif
}
//----------------------------------------------------------------------------
void FThreadContext::SetAffinityMask(size_t mask) const {
    Assert(0 != mask);
    Assert(std::this_thread::get_id() == _threadId);

#ifdef PLATFORM_WINDOWS
    HANDLE hThread = ::GetCurrentThread();
    DWORD_PTR affinityMask = ::SetThreadAffinityMask(hThread, mask);
    if (0 == affinityMask) {
        CORE_THROW_IT(FLastErrorException());
    }
    Assert(mask == AffinityMask());
#else
#   error "platform not supported"
#endif
}
//----------------------------------------------------------------------------
EThreadPriority FThreadContext::Priority() const {
    Assert(std::this_thread::get_id() == _threadId);

#ifdef PLATFORM_WINDOWS
    HANDLE hThread = ::GetCurrentThread();
    switch (::GetThreadPriority(hThread))
    {
    case THREAD_PRIORITY_HIGHEST:
        return EThreadPriority::Highest;
    case THREAD_PRIORITY_ABOVE_NORMAL:
        return EThreadPriority::AboveNormal;
    case THREAD_PRIORITY_NORMAL:
        return EThreadPriority::Normal;
    case THREAD_PRIORITY_BELOW_NORMAL:
        return EThreadPriority::BelowNormal;
    case THREAD_PRIORITY_LOWEST:
        return EThreadPriority::Lowest;
    case THREAD_PRIORITY_ERROR_RETURN:
        AssertNotReached();
        break;
    default:
        AssertNotImplemented();
        break;
    }
    return EThreadPriority::Normal;
#else
#   error "platform not supported"
#endif
}
//----------------------------------------------------------------------------
void FThreadContext::SetPriority(EThreadPriority priority) const {
Assert(std::this_thread::get_id() == _threadId);

#ifdef PLATFORM_WINDOWS
    HANDLE hThread = ::GetCurrentThread();
    int priorityWin32;
    switch (priority)
    {
    case Core::EThreadPriority::Highest:
        priorityWin32 = THREAD_PRIORITY_HIGHEST;
        break;
    case Core::EThreadPriority::AboveNormal:
        priorityWin32 = THREAD_PRIORITY_ABOVE_NORMAL;
        break;
    case Core::EThreadPriority::Normal:
        priorityWin32 = THREAD_PRIORITY_NORMAL;
        break;
    case Core::EThreadPriority::BelowNormal:
        priorityWin32 = THREAD_PRIORITY_BELOW_NORMAL;
        break;
    case Core::EThreadPriority::Lowest:
        priorityWin32 = THREAD_PRIORITY_LOWEST;
        break;
    default:
        AssertNotImplemented();
        return;
    }

    if (0 == ::SetThreadPriority(hThread, priorityWin32))
        AssertNotReached();
#else
#   error "platform not supported"
#endif
}
//----------------------------------------------------------------------------
// you can put here everything you'll want to tick regularly
void FThreadContext::DutyCycle() const {
    // chance to cleanup thread local allocator
    malloc_release_pending_blocks();
}
//----------------------------------------------------------------------------
size_t FThreadContext::NumThreads() {
    return GNumThreadContext_;
}
//----------------------------------------------------------------------------
size_t FThreadContext::GetThreadHash(std::thread::id thread_id) {
    return checked_cast<size_t>(GetThreadHash_(thread_id));
}
//----------------------------------------------------------------------------
FStringView FThreadContext::GetThreadName(std::thread::id thread_id) {
#ifdef WITH_CORE_THREADCONTEXT_NAME
    return GetThreadName_(thread_id);
#else
    return FStringView();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
thread_local size_t GCurrentThreadIndex = INDEX_NONE;
//----------------------------------------------------------------------------
const FThreadContext& CurrentThreadContext() {
    return FThreadLocalContext_::Instance();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FThreadContextStartup::Start(const char* name, size_t tag) {
    FThreadLocalContext_::Create(name, tag);
    Meta::FThreadLocalAutoSingletonManager::Start();
    FThreadLocalHeapStartup::Start(false);
    FAllocaStartup::Start(false);

#ifdef USE_DEBUG_LOGGER
    const FThreadContext& ctx = CurrentThreadContext();
    LOG(Thread, Info, L"start thread '{0}' with tag = {1} ({2})", MakeCStringView(ctx.Name()), ctx.Tag(), ctx.ThreadId());
#endif
}
//----------------------------------------------------------------------------
void FThreadContextStartup::Start_MainThread() {
    FThreadLocalContext_::CreateMainThread();
    Meta::FThreadLocalAutoSingletonManager::Start();
    FThreadLocalHeapStartup::Start(true);
    FAllocaStartup::Start(true);

#ifdef USE_DEBUG_LOGGER
    const FThreadContext& ctx = CurrentThreadContext();
    LOG(Thread, Info, L"start thread '{0}' with tag = {1} ({2}) <MainThread>", MakeCStringView(ctx.Name()), ctx.Tag(), ctx.ThreadId());
#endif
}
//----------------------------------------------------------------------------
void FThreadContextStartup::Shutdown() {
#ifdef USE_DEBUG_LOGGER
    const FThreadContext& ctx = CurrentThreadContext();
    LOG(Thread, Info, L"stop thread '{0}' with tag = {1} ({2})", MakeCStringView(ctx.Name()), ctx.Tag(), ctx.ThreadId());
#endif

    FAllocaStartup::Shutdown();
    FThreadLocalHeapStartup::Shutdown();
    FThreadLocalContext_::Destroy();
    Meta::FThreadLocalAutoSingletonManager::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, std::thread::id thread_id) {
#ifdef WITH_CORE_THREADCONTEXT_NAME
    return Format(oss, "thread_id:{0:#5} \"{1}\"", GetThreadHash_(thread_id), GetThreadName_(thread_id));
#else
    return Format(oss, "thread_id:{0:#5}", GetThreadHash_(thread_id));
#endif
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, std::thread::id thread_id) {
#ifdef WITH_CORE_THREADCONTEXT_NAME
    return Format(oss, L"thread_id:{0:#5} \"{1}\"", GetThreadHash_(thread_id), GetThreadName_(thread_id));
#else
    return Format(oss, L"thread_id:{0:#5}", GetThreadHash_(thread_id));
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
