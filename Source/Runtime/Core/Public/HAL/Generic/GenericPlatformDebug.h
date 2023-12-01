#pragma once

#include "HAL/TargetPlatform.h"

#define USE_PPE_PLATFORM_DEBUG_CPU_MARKERS (USE_PPE_PLATFORM_DEBUG && 1) // %_NOCOMMIT%
#define USE_PPE_PLATFORM_DEBUG_MEM_MARKERS (USE_PPE_PLATFORM_DEBUG && !USE_PPE_PROFILING && 1) // %_NOCOMMIT%
#define USE_PPE_PLATFORM_DEBUG_MEM_POISONS (USE_PPE_SANITIZER && 1) // %_NOCOMMIT%

#if USE_PPE_PLATFORM_DEBUG

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FGenericPlatformDebug {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasDebugger, false);

public: // debugger
    static void DebugBreak() = delete;
    static void DebugBreakAttach() = delete;

    static bool IsDebuggerPresent() = delete;

    static void OutputDebug(const char* text) = delete;
    static void OutputDebug(const wchar_t* wtext) = delete;

public: // memory
    static bool CheckMemory() = delete;
    static void GuaranteeStackSizeForStackOverflowRecovery() = delete; // backup memory in current thread for stack overflow crash

public: // profiling
    struct FNamedScope {
        FNamedScope(const char* name) = delete;
        ~FNamedScope() = delete;
    };

    static void SetThreadDebugName(const char* name) = delete; // sets current thread name for debuggers

    static void TraceVerbose(const std::thread::id& tid, const char* category, i64 timestamp, const char* filename, size_t line, const char* text) = delete;
    static void TraceInformation(const std::thread::id& tid, const char* category, i64 timestamp, const char* filename, size_t line, const char* text) = delete;
    static void TraceWarning(const std::thread::id& tid, const char* category, i64 timestamp, const char* filename, size_t line, const char* text) = delete;
    static void TraceError(const std::thread::id& tid, const char* category, i64 timestamp, const char* filename, size_t line, const char* text) = delete;
    static void TraceFatal(const std::thread::id& tid, const char* category, i64 timestamp, const char* filename, size_t line, const char* text) = delete;

    static void TraceVerbose(const std::thread::id& tid, const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) = delete;
    static void TraceInformation(const std::thread::id& tid, const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) = delete;
    static void TraceWarning(const std::thread::id& tid, const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) = delete;
    static void TraceError(const std::thread::id& tid, const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) = delete;
    static void TraceFatal(const std::thread::id& tid, const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) = delete;

    static void BeginNamedEvent(u32 uid, const char* name) = delete;
    static void EndNamedEvent(u32 uid) = delete;

    using FHeapHandle = int;

    STATIC_CONST_INTEGRAL(FHeapHandle, HEAP_Alloca, 0);
    STATIC_CONST_INTEGRAL(FHeapHandle, HEAP_Malloc, 1);
    STATIC_CONST_INTEGRAL(FHeapHandle, HEAP_Linear, 2);

    static void AllocateEvent(FHeapHandle heap, void* ptr, size_t sz) = delete;
    static void ReallocateEvent(FHeapHandle heap, void* newp, size_t newsz, void* oldp) = delete;
    static void DeallocateEvent(FHeapHandle heap, void* ptr) = delete;

    static void PoisonMemory(void* ptr, size_t sz) = delete;
    static void UnpoisonMemory(void* ptr, size_t sz) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PLATFORM_DEBUG

#if USE_PPE_PLATFORM_DEBUG_CPU_MARKERS
#   define PPE_DEBUG_NAMEDSCOPE(_NAME) \
    const PPE::FPlatformDebug::FNamedScope ANONYMIZE(namedEvent){ (_NAME) }
#else
#   define PPE_DEBUG_NAMEDSCOPE(_NAME) NOOP()
#endif

#if USE_PPE_PLATFORM_DEBUG_MEM_MARKERS
#   define PPE_DEBUG_ALLOCATEEVENT(_HEAP, _PTR, _SZ) \
    PPE::FPlatformDebug::AllocateEvent((PPE::FPlatformDebug::CONCAT(HEAP_, _HEAP)), (_PTR), (_SZ))
#   define PPE_DEBUG_REALLOCATEEVENT(_HEAP, _NEWP, _SZ, _OLDP) \
    PPE::FPlatformDebug::ReallocateEvent((PPE::FPlatformDebug::CONCAT(HEAP_, _HEAP)), (_NEWP), (_SZ), (_OLDP))
#   define PPE_DEBUG_DEALLOCATEEVENT(_HEAP, _PTR) \
    PPE::FPlatformDebug::DeallocateEvent((PPE::FPlatformDebug::CONCAT(HEAP_, _HEAP)), (_PTR))
#else
#   define PPE_DEBUG_ALLOCATEEVENT(_HEAP, _PTR, _SZ) NOOP()
#   define PPE_DEBUG_REALLOCATEEVENT(_HEAP, _NEWP, _SZ, _OLDP) NOOP()
#   define PPE_DEBUG_DEALLOCATEEVENT(_HEAP, _PTR) NOOP()
#endif

#if USE_PPE_PLATFORM_DEBUG_MEM_POISONS
#   define PPE_DEBUG_POISONMEMORY(_PTR, _SZ) PPE::FPlatformDebug::PoisonMemory(_PTR, _SZ)
#   define PPE_DEBUG_UNPOISONMEMORY(_PTR, _SZ) PPE::FPlatformDebug::UnpoisonMemory(_PTR, _SZ)
#else
#   define PPE_DEBUG_POISONMEMORY(_PTR, _SZ) NOOP()
#   define PPE_DEBUG_UNPOISONMEMORY(_PTR, _SZ) NOOP()
#endif
