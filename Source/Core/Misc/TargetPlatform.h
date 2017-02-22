#pragma once

#include "Core/Core.h"

#include "Core/IO/StringView.h"
#include "Core/Memory/MemoryView.h"

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

#if     defined(OS_WINDOWS)
    Current         = PC,
#elif   defined(OS_LINUX)
    Current         = LINUX,
#else
#   error "unsupported os"
#endif
};
//----------------------------------------------------------------------------
TMemoryView<const ETargetPlatform> EachTargetPlatform();
FStringView TargetPlatformToCStr(ETargetPlatform platform);
EEndianness TargetPlatformEndianness(ETargetPlatform platform);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    ETargetPlatform platform) {
    return TargetPlatformToCStr(platform);
}
//----------------------------------------------------------------------------
struct FPlatform {
#ifndef FINAL_RELEASE
static void CheckMemory();
static void DebugBreak();
static void DebugBreakAttach();
static bool IsDebuggerAttached();
#endif
static void OutputDebug(const char* text);
static void OutputDebug(const wchar_t* text);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
