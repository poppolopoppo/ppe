#include "stdafx.h"

#include "ThreadContext.h"

#include "Alloca.h"
#include "Logger.h"
#include "String.h" // Copy()
#include "ThreadLocalStorage.h"
#include "ThreadLocalHeap.h"

#ifdef _DEBUG
#include <Windows.h>
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void SetWin32ThreadName_(const char* name) {
#ifdef _DEBUG
    /*
    // How to: Set a Thread Name in Native Code
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
#endif
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ThreadContext::ThreadContext(const char* name, size_t tag, std::thread::id id)
:   _tag(tag), _id(id) {
    Assert(name);
    Copy(_name, name);
    SetWin32ThreadName_(_name);
    LOG(Information, L"[Thread] Start '{0}' with tag = {1} (id:{2})", _name, _tag, _id);
}
//----------------------------------------------------------------------------
ThreadContext::~ThreadContext() {
    LOG(Information, L"[Thread] Stop '{0}' with tag = {1} (id:{2})", _name, _tag, _id);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ThreadContextStartup::Start(const char* name, size_t tag) {
    ThreadLocalManager::Instance().CreateThreadLocalStorage();
    CurrentThreadContext::Create(name, tag);
    ThreadLocalHeapStartup::Start(false);
    AllocaStartup::Start(false);
}
//----------------------------------------------------------------------------
void ThreadContextStartup::Start_MainThread() {
    ThreadLocalManager::Instance().CreateThreadLocalStorage();
    CurrentThreadContext::CreateMainThread();
    ThreadLocalHeapStartup::Start(true);
    AllocaStartup::Start(true);
}
//----------------------------------------------------------------------------
void ThreadContextStartup::Shutdown() {
    AllocaStartup::Shutdown();
    ThreadLocalHeapStartup::Shutdown();
    CurrentThreadContext::Destroy();
    ThreadLocalManager::Instance().DestroyThreadLocalStorage();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
