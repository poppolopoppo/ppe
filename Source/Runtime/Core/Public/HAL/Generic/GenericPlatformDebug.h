#pragma once

#include "HAL/TargetPlatform.h"

#define USE_PPE_PLATFORM_DEBUG (!USE_PPE_FINAL_RELEASE || USE_PPE_MEMORY_DEBUGGING)

#if USE_PPE_PLATFORM_DEBUG

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_API FGenericPlatformDebug {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasDebugger, false);

    static void CheckMemory() = delete;

    static void DebugBreak() = delete;
    static void DebugBreakAttach() = delete;

    static bool IsDebuggerPresent() = delete;

    static void OutputDebug(const char* text) = delete;
    static void OutputDebug(const wchar_t* text) = delete;

    static void BeginNamedEvent(const char* name, u32 color) = delete;
    static void EndNamedEvent(const char* name, u32 color) = delete;

    static void SetThreadDebugName(const char* name) = delete; // sets current thread name for debuggers
    static void GuaranteeStackSizeForStackOverflowRecovery() = delete; // backup memory in current thread for stack overflow crash
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PLATFORM_DEBUG
