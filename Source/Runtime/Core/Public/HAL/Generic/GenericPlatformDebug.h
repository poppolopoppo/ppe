#pragma once

#include "HAL/TargetPlatform.h"

#define USE_PPE_PLATFORM_DEBUG (!USE_PPE_FINAL_RELEASE || USE_PPE_MEMORY_DEBUGGING)

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
    static void CheckMemory() = delete;
    static void GuaranteeStackSizeForStackOverflowRecovery() = delete; // backup memory in current thread for stack overflow crash

public: // profiling
    struct FNamedScope {
        FNamedScope(const char* name) = delete;
        ~FNamedScope() = delete;
    };

    static void SetThreadDebugName(const char* name) = delete; // sets current thread name for debuggers

    static void BeginNamedEvent(u32 uid, const char* name) = delete;
    static void EndNamedEvent(u32 uid) = delete;

    using FHeapHandle = int;

    STATIC_CONST_INTEGRAL(FHeapHandle, HEAP_Alloca, 0);
    STATIC_CONST_INTEGRAL(FHeapHandle, HEAP_Malloc, 1);
    STATIC_CONST_INTEGRAL(FHeapHandle, HEAP_Linear, 2);

    static void AllocateEvent(FHeapHandle heap, void* ptr, size_t sz) = delete;
    static void ReallocateEvent(FHeapHandle heap, void* newp, size_t newsz, void* oldp) = delete;
    static void DeallocateEvent(FHeapHandle heap, void* ptr) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#   define PPE_DEBUG_NAMEDSCOPE(_NAME) \
    const PPE::FPlatformDebug::FNamedScope ANONYMIZE(namedEvent){ (_NAME) }

#   define PPE_DEBUG_ALLOCATEEVENT(_HEAP, _PTR, _SZ) \
    PPE::FPlatformDebug::AllocateEvent((PPE::FPlatformDebug::CONCAT(HEAP_, _HEAP)), (_PTR), (_SZ))
#   define PPE_DEBUG_REALLOCATEEVENT(_HEAP, _NEWP, _SZ, _OLDP) \
    PPE::FPlatformDebug::ReallocateEvent((PPE::FPlatformDebug::CONCAT(HEAP_, _HEAP)), (_NEWP), (_SZ), (_OLDP))
#   define PPE_DEBUG_DEALLOCATEEVENT(_HEAP, _PTR) \
    PPE::FPlatformDebug::DeallocateEvent((PPE::FPlatformDebug::CONCAT(HEAP_, _HEAP)), (_PTR))

#else
#   define PPE_DEBUG_BREAK() NOOP()
#   define PPE_DECLSPEC_ALLOCATOR()

#   define PPE_DEBUG_NAMEDSCOPE(_NAME) NOOP()

#   define PPE_DEBUG_ALLOCATEEVENT(_HEAP, _SZ) NOOP()
#   define PPE_DEBUG_REALLOCATEEVENT(_HEAP, _NEWP, _SZ, _OLDP) NOOP()
#   define PPE_DEBUG_DEALLOCATEEVENT(_HEAP, _PTR) NOOP()

#endif //!USE_PLATFORM_DEBUG
