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

public: // debugger
    static void DebugBreak() { _CrtDbgBreak(); }
    static void DebugBreakAttach() { ::DebugBreak(); }

    static bool IsDebuggerPresent() { return (!!::IsDebuggerPresent()); }

    static void OutputDebug(const char* text) { ::OutputDebugStringA(text); }
    static void OutputDebug(const wchar_t* wtext) { ::OutputDebugStringW(wtext); }

public: // memory
    static bool CheckMemory() { return (!!_CrtCheckMemory()); }
    static void GuaranteeStackSizeForStackOverflowRecovery();

public: // profiling
    struct FNamedScope {
        FNamedScope(const char* name) NOEXCEPT {
            BeginNamedEvent(static_cast<u32>(hash_ptr(this)), name);
        }
        ~FNamedScope() NOEXCEPT {
            EndNamedEvent(static_cast<u32>(hash_ptr(this)));
        }
    };

    static void SetThreadDebugName(const char* name); // sets current thread name for debuggers

    static void TraceVerbose(const std::thread::id& tid, const char* category, i64 timestamp, const char* filename, size_t line, const char* text);
    static void TraceInformation(const std::thread::id& tid, const char* category, i64 timestamp, const char* filename, size_t line, const char* text);
    static void TraceWarning(const std::thread::id& tid, const char* category, i64 timestamp, const char* filename, size_t line, const char* text);
    static void TraceError(const std::thread::id& tid, const char* category, i64 timestamp, const char* filename, size_t line, const char* text);
    static void TraceFatal(const std::thread::id& tid, const char* category, i64 timestamp, const char* filename, size_t line, const char* text);

    static void TraceVerbose(const std::thread::id& tid, const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text);
    static void TraceInformation(const std::thread::id& tid, const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text);
    static void TraceWarning(const std::thread::id& tid, const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text);
    static void TraceError(const std::thread::id& tid, const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text);
    static void TraceFatal(const std::thread::id& tid, const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text);

    static void BeginNamedEvent(u32 uid, const char* name);
    static void EndNamedEvent(u32 uid);

    using FHeapHandle = void*;

    static FHeapHandle& HEAP_Alloca;
    static FHeapHandle& HEAP_Malloc;
    static FHeapHandle& HEAP_Linear;

    static void AllocateEvent(FHeapHandle heap, void* ptr, size_t sz);
    static void ReallocateEvent(FHeapHandle heap, void* newp, size_t newsz, void* oldp);
    static void DeallocateEvent(FHeapHandle heap, void* ptr);

    static void PoisonMemory(void* ptr, size_t sz);
    static void UnpoisonMemory(void* ptr, size_t sz);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_PLATFORM_DEBUG && defined(PLATFORM_WINDOWS)
