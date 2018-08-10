#pragma once

#include "HAL/Generic/GenericPlatformDebug.h"

#if USE_PPE_PLATFORM_DEBUG && defined(PLATFORM_WINDOWS)

#include "HAL/Windows/WindowsPlatformIncludes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformDebug : FGenericPlatformDebug {
public:
    STATIC_CONST_INTEGRAL(bool, HasDebugger, true);

    static FORCE_INLINE void CheckMemory() { _CrtCheckMemory(); }

    static FORCE_INLINE void DebugBreak() { _CrtDbgBreak(); }
    static FORCE_INLINE void DebugBreakAttach() { ::DebugBreak(); }

    static FORCE_INLINE bool IsDebuggerPresent() { return (::IsDebuggerPresent() ? true : false); }

    static FORCE_INLINE void OutputDebug(const char* text) { ::OutputDebugStringA(text); }
    static FORCE_INLINE void OutputDebug(const wchar_t* text) { ::OutputDebugStringW(text); }

    static FORCE_INLINE void BeginNamedEvent(const char* name, u32 color) { /* TODO */NOOP(name, color); }
    static FORCE_INLINE void EndNamedEvent(const char* name, u32 color) { /* TODO */NOOP(name, color); }

    static void SetThreadDebugName(const char* name);
    static void GuaranteeStackSizeForStackOverflowRecovery();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_PLATFORM_DEBUG && defined(PLATFORM_WINDOWS)
