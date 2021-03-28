#pragma once

#include "HAL/Generic/GenericPlatformDebug.h"

#if USE_PPE_PLATFORM_DEBUG && defined(PLATFORM_LINUX)

#include "HAL/Linux/LinuxPlatformIncludes.h"

#if __has_builtin(__builtin_debugtrap)
#   define PPE_DEBUG_BREAK() __builtin_debugtrap()
#else
#   include <signal.h>
#   if defined(SIGTRAP)
#       define PPE_DEBUG_BREAK() raise(SIGTRAP)
#   else
#       define PPE_DEBUG_BREAK() __asm__ volatile("int $0x03")
#   endif
#endif

#define PPE_DECLSPEC_ALLOCATOR() // not supported on Linux

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FLinuxPlatformDebug : FGenericPlatformDebug {
public:
    STATIC_CONST_INTEGRAL(bool, HasDebugger, true);

public: // debugger
    static void DebugBreak() { PPE_DEBUG_BREAK(); }
    static void DebugBreakAttach() { PPE_DEBUG_BREAK(); }

    static bool IsDebuggerPresent();

    static void OutputDebug(const char* text);
    static void OutputDebug(const wchar_t* wtext);

public: // memory
    static void CheckMemory() {}
    static void GuaranteeStackSizeForStackOverflowRecovery();

public: // profiling
    struct FNamedScope {
        FNamedScope(const char* name) {
            BeginNamedEvent((u32)uintptr_t(this), name);
        }
        ~FNamedScope() {
            EndNamedEvent((u32)uintptr_t(this));
        }
    };

    static void SetThreadDebugName(const char* name); // sets current thread name for debuggers

    static void TraceVerbose(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text);
    static void TraceInformation(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text);
    static void TraceWarning(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text);
    static void TraceError(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text);
    static void TraceFatal(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text);

    static void BeginNamedEvent(u32 uid, const char* name);
    static void EndNamedEvent(u32 uid);

    using typename FGenericPlatformDebug::FHeapHandle;

    using FGenericPlatformDebug::HEAP_Alloca;
    using FGenericPlatformDebug::HEAP_Malloc;
    using FGenericPlatformDebug::HEAP_Linear;

    static void AllocateEvent(FHeapHandle , void* , size_t ) {}
    static void ReallocateEvent(FHeapHandle , void* , size_t , void* ) {}
    static void DeallocateEvent(FHeapHandle , void* ) {}

    static void PoisonMemory(void* , size_t ) {}
    static void UnpoisonMemory(void* , size_t ) {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_PLATFORM_DEBUG && defined(PLATFORM_LINUX)
