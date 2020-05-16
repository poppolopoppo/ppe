#pragma once

#include "HAL/Generic/GenericPlatformDebug.h"

#if USE_PPE_PLATFORM_DEBUG && defined(PLATFORM_WINDOWS)

#include "HAL/Windows/WindowsPlatformIncludes.h"

#define PPE_DEBUG_BREAK() __debugbreak() // more comfy to break in current frame
#define PPE_DEBUG_CRASH() ::FatalExit(-1) // transfers execution control to the debugger
#define PPE_DECLSPEC_ALLOCATOR() __declspec(allocator)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformDebug : FGenericPlatformDebug {
public:
    STATIC_CONST_INTEGRAL(bool, HasDebugger, true);

public: // debugger
    static void DebugBreak() { _CrtDbgBreak(); }
    static void DebugBreakAttach() { ::DebugBreak(); }

    static bool IsDebuggerPresent() { return (!!::IsDebuggerPresent()); }

    static void OutputDebug(const char* text) { ::OutputDebugStringA(text); }
    static void OutputDebug(const wchar_t* wtext) { ::OutputDebugStringW(wtext); }

public: // memory
    static void CheckMemory() { _CrtCheckMemory(); }
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

    using FHeapHandle = void*;

    static FHeapHandle& HEAP_Alloca;
    static FHeapHandle& HEAP_Malloc;
    static FHeapHandle& HEAP_Linear;

    static void AllocateEvent(FHeapHandle heap, void* ptr, size_t sz);
    static void ReallocateEvent(FHeapHandle heap, void* newp, size_t newsz, void* oldp);
    static void DeallocateEvent(FHeapHandle heap, void* ptr);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_PLATFORM_DEBUG && defined(PLATFORM_WINDOWS)
