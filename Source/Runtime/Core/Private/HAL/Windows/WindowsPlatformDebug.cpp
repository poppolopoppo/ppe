#include "stdafx.h"

#include "WindowsPlatformDebug.h"

#if USE_PPE_PLATFORM_DEBUG && defined(PLATFORM_WINDOWS)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(6320) // L'expression de filtre d'exception correspond a la constante EXCEPTION_EXECUTE_HANDLER.
                                  // Cela risque de masquer les exceptions qui n'etaient pas destinees a etre gerees.
PRAGMA_MSVC_WARNING_DISABLE(6322) // bloc empty _except.
void FWindowsPlatformDebug::SetThreadDebugName(const char* name) {
    /*
    // How to: Set a Thread FName in Native Code
    // http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
    */
    const ::DWORD MS_VC_EXCEPTION = 0x406D1388;
#   pragma pack(push,8)
    typedef struct tagTHREADNAME_INFO
    {
        ::DWORD dwType; // Must be 0x1000.
        ::LPCSTR szName; // Pointer to name (in user addr space).
        ::DWORD dwThreadID; // Thread ID (-1=caller thread).
        ::DWORD dwFlags; // Reserved for future use, must be zero.
    }   THREADNAME_INFO;
#   pragma pack(pop)

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = checked_cast<::DWORD>(-1);
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
//----------------------------------------------------------------------------
void FWindowsPlatformDebug::GuaranteeStackSizeForStackOverflowRecovery() {
    ULONG stackSizeInBytes = 0;
    if (::SetThreadStackGuarantee(&stackSizeInBytes)) {
        stackSizeInBytes += 64 * 1024;
        if (::SetThreadStackGuarantee(&stackSizeInBytes))
            return;
    }
    AssertNotReached();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_PLATFORM_DEBUG && defined(PLATFORM_WINDOWS)