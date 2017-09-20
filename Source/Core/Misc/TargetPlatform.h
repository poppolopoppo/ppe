#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/Policies.h"
#include "Core/IO/StringView.h"
#include "Core/Memory/MemoryView.h"

#include <intrin.h>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EEndianness {
    LittleEndian    = 0,
    BigEndian       = 1,

#if     defined(ARCH_X86)
    Current = LittleEndian,
#elif   defined(ARCH_X64)
    Current = LittleEndian,
#else
#   error "unsupported architecture"
#endif
};
//----------------------------------------------------------------------------
enum class ETargetPlatform {
    PC              = 1<<0,
    PS4             = 1<<1,
    XONE            = 1<<2,
    MAC             = 1<<3,
    LINUX           = 1<<4,

#if     defined(PLATFORM_WINDOWS)
    Current         = PC,
#elif   defined(PLATFORM_LINUX)
    Current         = LINUX,
#else
#   error "unsupported os"
#endif
};
//----------------------------------------------------------------------------
CORE_API TMemoryView<const ETargetPlatform> EachTargetPlatform();
CORE_API FStringView TargetPlatformToCStr(ETargetPlatform platform);
CORE_API EEndianness TargetPlatformEndianness(ETargetPlatform platform);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    ETargetPlatform platform) {
    return TargetPlatformToCStr(platform);
}
//----------------------------------------------------------------------------
CORE_API struct FPlatform {
    struct FSystemInfo {
        size_t AllocationGranularity;
        size_t PageSize;
        size_t ProcessorsCount;
    };
    static const FSystemInfo& SystemInfo;
    static void Sleep(size_t ms);
#ifndef FINAL_RELEASE
    static void CheckMemory();
    static void DebugBreak();
    static void DebugBreakAttach();
    static bool IsDebuggerAttached();
    static void OutputDebug(const char* text);
    static void OutputDebug(const wchar_t* text);
#endif
};
//----------------------------------------------------------------------------
struct FPlatformAtomics {
#ifdef PLATFORM_WINDOWS
    FORCE_INLINE static long Exchange(long volatile* target, long value) { return ::InterlockedExchange(target, value); }
    FORCE_INLINE static void* Exchange(void* volatile* target, void* value) { return ::InterlockedExchangePointer(target, value); }
    FORCE_INLINE static long CompareExchange(long volatile* target, long exchange, long comparand) { return ::InterlockedCompareExchange(target, exchange, comparand); }
    FORCE_INLINE static void* CompareExchange(void* volatile* target, void* exchange, void* comparand) { return ::InterlockedCompareExchangePointer(target, exchange, comparand); }
    FORCE_INLINE static void ShortSyncWait() { ::_mm_pause(); }
#else
#   error "unsupported platform"
#endif
};
//----------------------------------------------------------------------------
struct FPlatformIO {
    typedef int FHandle;

    static constexpr FHandle Stdin = 0;
    static constexpr FHandle Stdout = 1;
    static constexpr FHandle Stderr = 2;
    static constexpr FHandle InvalidHandle = int(-1);

    static bool Access(const wchar_t* entity, EExistPolicy exists);
    static FHandle Open(const wchar_t* filename, EOpenPolicy openMode, EAccessPolicy accessFlags);
    static bool Close(FHandle handle);
    static bool Eof(FHandle handle);
    static std::streamoff Tell(FHandle handle);
    static std::streamoff Seek(FHandle handle, std::streamoff offset, ESeekOrigin origin);
    static std::streamsize Read(FHandle handle, void* dst, std::streamsize sizeInBytes);
    static std::streamsize Write(FHandle handle, const void* src, std::streamsize sizeInBytes);
    static bool Commit(FHandle handle);
    static FHandle Dup(FHandle handle);
    static bool Dup2(FHandle handleSrc, FHandle handleDst);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
