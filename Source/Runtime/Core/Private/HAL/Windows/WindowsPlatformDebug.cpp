// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Windows/WindowsPlatformDebug.h"

#if USE_PPE_PLATFORM_DEBUG && defined(PLATFORM_WINDOWS)

#   include "HAL/PlatformProfiler.h"
#   include "HAL/Windows/TraceLogging.h"
#   include "HAL/Windows/VSToolsWrapper.h"

#if USE_PPE_PLATFORM_DEBUG_MEM_POISONS
#   include <sanitizer/asan_interface.h>
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FWindowsPlatformDebug::GuaranteeStackSizeForStackOverflowRecovery() {
    ULONG stackSizeInBytes = 0;
    if (::SetThreadStackGuarantee(&stackSizeInBytes)) {
        stackSizeInBytes += PAGE_SIZE; // 4 kb
        if (::SetThreadStackGuarantee(&stackSizeInBytes))
            return;
    }
    AssertNotReached();
}
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
// Windows trace logging
//----------------------------------------------------------------------------
void FWindowsPlatformDebug::TraceVerbose(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) {
#if USE_PPE_WINDOWS_TRACELOGGING
    FWindowsTraceLogging::TraceVerbose(category, timestamp, filename, line, text);
#else
    Unused(category);
    Unused(timestamp);
    Unused(filename);
    Unused(line);
    Unused(text);
#endif
}
//----------------------------------------------------------------------------
void FWindowsPlatformDebug::TraceInformation(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) {
#if USE_PPE_WINDOWS_TRACELOGGING
    FWindowsTraceLogging::TraceInformation(category, timestamp, filename, line, text);
#else
    Unused(category);
    Unused(timestamp);
    Unused(filename);
    Unused(line);
    Unused(text);
#endif
}
//----------------------------------------------------------------------------
void FWindowsPlatformDebug::TraceWarning(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) {
#if USE_PPE_WINDOWS_TRACELOGGING
    FWindowsTraceLogging::TraceWarning(category, timestamp, filename, line, text);
#else
    Unused(category);
    Unused(timestamp);
    Unused(filename);
    Unused(line);
    Unused(text);
#endif
}
//----------------------------------------------------------------------------
void FWindowsPlatformDebug::TraceError(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) {
#if USE_PPE_WINDOWS_TRACELOGGING
    FWindowsTraceLogging::TraceError(category, timestamp, filename, line, text);
#else
    Unused(category);
    Unused(timestamp);
    Unused(filename);
    Unused(line);
    Unused(text);
#endif
}
//----------------------------------------------------------------------------
void FWindowsPlatformDebug::TraceFatal(const wchar_t* category, i64 timestamp, const wchar_t* filename, size_t line, const wchar_t* text) {
#if USE_PPE_WINDOWS_TRACELOGGING
    FWindowsTraceLogging::TraceFatal(category, timestamp, filename, line, text);
#else
    Unused(category);
    Unused(timestamp);
    Unused(filename);
    Unused(line);
    Unused(text);
#endif
}
//----------------------------------------------------------------------------
// #TODO: Windows trace activity
//----------------------------------------------------------------------------
void FWindowsPlatformDebug::BeginNamedEvent(u32 uid, const char* name) {
#if USE_PPE_PLATFORM_PROFILER
    FPlatformProfiler::MarkAndComment(uid, name);
#else
    Unused(uid);
    Unused(name);
#endif
}
//----------------------------------------------------------------------------
void FWindowsPlatformDebug::EndNamedEvent(u32 uid) {
#if USE_PPE_PLATFORM_PROFILER
    FPlatformProfiler::Mark(uid);
#else
    Unused(uid);
#endif
}
//----------------------------------------------------------------------------
// Memory tracking
//----------------------------------------------------------------------------
#if USE_PPE_VSTOOLS_WRAPPER
//----------------------------------------------------------------------------
FWindowsPlatformDebug::FHeapHandle& FWindowsPlatformDebug::HEAP_Alloca = FVSToolsWrapper::HEAP_Alloca;
FWindowsPlatformDebug::FHeapHandle& FWindowsPlatformDebug::HEAP_Malloc = FVSToolsWrapper::HEAP_Malloc;
FWindowsPlatformDebug::FHeapHandle& FWindowsPlatformDebug::HEAP_Linear = FVSToolsWrapper::HEAP_Linear;
//----------------------------------------------------------------------------
void FWindowsPlatformDebug::AllocateEvent(FHeapHandle heap, void* ptr, size_t sz) {
    FVSToolsWrapper::API.AllocateEvent(heap, ptr, checked_cast<unsigned long>(sz));
}
//----------------------------------------------------------------------------
void FWindowsPlatformDebug::ReallocateEvent(FHeapHandle heap, void* newp, size_t newsz, void* oldp) {
    FVSToolsWrapper::API.ReallocateEvent(heap, newp, checked_cast<unsigned long>(newsz), oldp);
}
//----------------------------------------------------------------------------
void FWindowsPlatformDebug::DeallocateEvent(FHeapHandle heap, void* ptr) {
    FVSToolsWrapper::API.DeallocateEvent(heap, ptr);
}
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
static FWindowsPlatformDebug::FHeapHandle GHeap_Dummy = nullptr;
//----------------------------------------------------------------------------
FWindowsPlatformDebug::FHeapHandle& FWindowsPlatformDebug::HEAP_Alloca = GHeap_Dummy;
FWindowsPlatformDebug::FHeapHandle& FWindowsPlatformDebug::HEAP_Malloc = GHeap_Dummy;
FWindowsPlatformDebug::FHeapHandle& FWindowsPlatformDebug::HEAP_Linear = GHeap_Dummy;
//---------------------------------------------------------------------------
void FWindowsPlatformDebug::AllocateEvent(FHeapHandle , void* , size_t ) {}
void FWindowsPlatformDebug::ReallocateEvent(FHeapHandle , void* , size_t , void* ) {}
void FWindowsPlatformDebug::DeallocateEvent(FHeapHandle , void* ) {}
//---------------------------------------------------------------------------
#endif //!USE_PPE_VSTOOLS_WRAPPER
//----------------------------------------------------------------------------
#if USE_PPE_PLATFORM_DEBUG_MEM_POISONS
void FWindowsPlatformDebug::PoisonMemory(void* ptr, size_t sz) {
    ASAN_POISON_MEMORY_REGION(ptr, sz);
}
void FWindowsPlatformDebug::UnpoisonMemory(void* ptr, size_t sz) {
    ASAN_UNPOISON_MEMORY_REGION(ptr, sz);
}
#else
void FWindowsPlatformDebug::PoisonMemory(void*, size_t) {}
void FWindowsPlatformDebug::UnpoisonMemory(void*, size_t) {}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_PLATFORM_DEBUG && defined(PLATFORM_WINDOWS)
