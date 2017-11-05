#include "stdafx.h"

#include "ThreadContext.h"

#include "Allocator/Alloca.h"
#include "Allocator/ThreadLocalHeap.h"
#include "Diagnostic/LastError.h"
#include "Diagnostic/Logger.h"
#include "IO/StringView.h"
#include "Meta/AutoSingleton.h"

#ifndef FINAL_RELEASE
#   define WITH_CORE_THREADCONTEXT_NAME
#endif

#ifdef PLATFORM_WINDOWS
#   include "Misc/Platform_Windows.h"
#endif

namespace Core {
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
    parent_type::Create(name, tag, GNumThreadContext_++);
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
#pragma warning(push)
#pragma warning(disable: 6320) // L'expression de filtre d'exception correspond à la constante EXCEPTION_EXECUTE_HANDLER.
                               // Cela risque de masquer les exceptions qui n'étaient pas destinées à être gérées.
#pragma warning(disable: 6322) // bloc empty _except.
static void SetWin32ThreadName_(const char* name) {
#ifdef WITH_CORE_THREADCONTEXT_NAME
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
#else
    UNUSED(name);
#endif
}
#pragma warning(pop)
//----------------------------------------------------------------------------
static NO_INLINE void GuaranteeStackSizeForStackOverflowRecovery_() {
#ifdef PLATFORM_WINDOWS
    ULONG stackSizeInBytes = 0;
    if(::SetThreadStackGuarantee(&stackSizeInBytes)) {
        stackSizeInBytes += 64*1024;
        if (::SetThreadStackGuarantee(&stackSizeInBytes))
            return;
    }
    LOG(Warning, L"Unable to SetThreadStackGuarantee, TStack Overflows won't be caught properly !");
#endif
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FThreadContext::FThreadContext(const char* name, size_t tag, size_t index)
:   _tag(tag)
,   _threadId(std::this_thread::get_id())
,   _threadIndex(index) {
    Assert(name);

    const size_t n = Copy(MakeView(_name), MakeStringView(name, Meta::FForceInit{}));
    Assert(n < lengthof(_name));
    _name[n] = '\0';

    SetWin32ThreadName_(_name);
    GuaranteeStackSizeForStackOverflowRecovery_();
}
//----------------------------------------------------------------------------
FThreadContext::~FThreadContext() {}
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
size_t FThreadContext::NumThreads() {
    return GNumThreadContext_;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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

    const FThreadContext& ctx = CurrentThreadContext();
    LOG(Info, L"[Thread] Start '{0}' with tag = {1} (id:{2})", ctx.Name(), ctx.Tag(), ctx.ThreadId());
}
//----------------------------------------------------------------------------
void FThreadContextStartup::Start_MainThread() {
    FThreadLocalContext_::CreateMainThread();
    Meta::FThreadLocalAutoSingletonManager::Start();
    FThreadLocalHeapStartup::Start(true);
    FAllocaStartup::Start(true);

    const FThreadContext& ctx = CurrentThreadContext();
    LOG(Info, L"[Thread] Start '{0}' with tag = {1} (id:{2}) <MainThread>", ctx.Name(), ctx.Tag(), ctx.ThreadId());
}
//----------------------------------------------------------------------------
void FThreadContextStartup::Shutdown() {
    const FThreadContext& ctx = CurrentThreadContext();
    LOG(Info, L"[Thread] Stop '{0}' with tag = {1} (id:{2})", ctx.Name(), ctx.Tag(), ctx.ThreadId());

    FAllocaStartup::Shutdown();
    FThreadLocalHeapStartup::Shutdown();
    FThreadLocalContext_::Destroy();
    Meta::FThreadLocalAutoSingletonManager::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
