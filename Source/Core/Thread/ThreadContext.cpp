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

#ifdef OS_WINDOWS
#   include <Windows.h>
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FThreadLocalContext_ : Meta::TThreadLocalSingleton<FThreadContext, FThreadLocalContext_> {
    typedef Meta::TThreadLocalSingleton<FThreadContext, FThreadLocalContext_> parent_type;
public:
    using parent_type::HasInstance;
    using parent_type::Instance;
    using parent_type::Destroy;

    static void Create(const char* name, size_t tag);
    static void CreateMainThread();
};
//----------------------------------------------------------------------------
inline void FThreadLocalContext_::Create(const char* name, size_t tag) {
    Assert(CORE_THREADTAG_MAIN != tag);
    parent_type::Create(name, tag);
}
//----------------------------------------------------------------------------
inline void FThreadLocalContext_::CreateMainThread() {
    parent_type::Create("MainThread", CORE_THREADTAG_MAIN);
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
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
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
#ifdef OS_WINDOWS
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
FThreadContext::FThreadContext(const char* name, size_t tag)
:   _tag(tag)
,   _threadId(std::this_thread::get_id()) {
    Assert(name);

    const size_t n = Copy(MakeView(_name), MakeStringView(name, Meta::noinit_tag()));
    Assert(n < lengthof(_name));
    _name[n] = '\0';

    SetWin32ThreadName_(_name);
    GuaranteeStackSizeForStackOverflowRecovery_();

    LOG(Info, L"[Thread] Start '{0}' with tag = {1} (id:{2})", _name, _tag, _threadId);
}
//----------------------------------------------------------------------------
FThreadContext::~FThreadContext() {
    LOG(Info, L"[Thread] Stop '{0}' with tag = {1} (id:{2})", _name, _tag, _threadId);
}
//----------------------------------------------------------------------------
size_t FThreadContext::AffinityMask() const {
    Assert(std::this_thread::get_id() == _threadId);
#ifdef OS_WINDOWS
    HANDLE currentThread = ::GetCurrentThread();
    DWORD_PTR affinityMask = ::SetThreadAffinityMask(currentThread, 0xFFul);
    if (0 == affinityMask) {
        FLastErrorException e;
        CORE_THROW_IT(e);
    }
    if (0 == ::SetThreadAffinityMask(currentThread, affinityMask)) {
        FLastErrorException e;
        CORE_THROW_IT(e);
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

#ifdef OS_WINDOWS
    HANDLE currentThread = ::GetCurrentThread();
    DWORD_PTR affinityMask = ::SetThreadAffinityMask(currentThread, mask);
    if (0 == affinityMask) {
        FLastErrorException e;
        CORE_THROW_IT(e);
    }
    Assert(mask == AffinityMask());
#else
#   error "platform not supported"
#endif
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
    FThreadLocalHeapStartup::Start(false);
    Meta::FThreadLocalAutoSingletonManager::Start();
    FAllocaStartup::Start(false);
}
//----------------------------------------------------------------------------
void FThreadContextStartup::Start_MainThread() {
    Meta::FThreadLocalAutoSingletonManager::Start();
    FThreadLocalContext_::CreateMainThread();
    FThreadLocalHeapStartup::Start(true);
    FAllocaStartup::Start(true);
}
//----------------------------------------------------------------------------
void FThreadContextStartup::Shutdown() {
    FAllocaStartup::Shutdown();
    FThreadLocalHeapStartup::Shutdown();
    FThreadLocalContext_::Destroy();
    Meta::FThreadLocalAutoSingletonManager::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
