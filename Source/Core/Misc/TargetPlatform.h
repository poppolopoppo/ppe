#pragma once

#include "Core/Core.h"

#include "Core/IO/StringView.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class Endianness {
    LittleEndian    = 0,
    BigEndian       = 1,
};
//----------------------------------------------------------------------------
enum class TargetPlatform {
    PC              = 1<<0,
    PS4             = 1<<1,
    XONE            = 1<<2,
    MAC             = 1<<3,

#if defined(OS_WINDOWS)
    Current         = PC,
#else
#   error "unsupported os"
#endif
};
//----------------------------------------------------------------------------
MemoryView<const TargetPlatform> EachTargetPlatform();
StringView TargetPlatformToCStr(TargetPlatform platform);
Endianness TargetPlatformEndianness(TargetPlatform platform);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    TargetPlatform platform) {
    return TargetPlatformToCStr(platform);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
