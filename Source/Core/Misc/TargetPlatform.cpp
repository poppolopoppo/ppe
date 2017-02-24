#include "stdafx.h"

#include "TargetPlatform.h"

#ifdef OS_WINDOWS
#   include <windows.h>
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static constexpr ETargetPlatform gTargetPlatforms[] = {
    ETargetPlatform::PC,
    ETargetPlatform::PS4,
    ETargetPlatform::XONE,
    ETargetPlatform::MAC,
    ETargetPlatform::LINUX
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TMemoryView<const ETargetPlatform> EachTargetPlatform() {
    return MakeView(gTargetPlatforms);
}
//----------------------------------------------------------------------------
FStringView TargetPlatformToCStr(ETargetPlatform platform) {
    switch (platform)
    {
    case Core::ETargetPlatform::PC:
        return FStringView("PC");
    case Core::ETargetPlatform::PS4:
        return FStringView("PS4");
    case Core::ETargetPlatform::XONE:
        return FStringView("XONE");
    case Core::ETargetPlatform::MAC:
        return FStringView("MAC");
    default:
        AssertNotImplemented();
    }
    return FStringView();
}
//----------------------------------------------------------------------------
EEndianness TargetPlatformEndianness(ETargetPlatform platform) {
    switch (platform)
    {
    case Core::ETargetPlatform::PC:
        return EEndianness::LittleEndian;
    case Core::ETargetPlatform::PS4:
        return EEndianness::LittleEndian;
    case Core::ETargetPlatform::XONE:
        return EEndianness::LittleEndian;
    case Core::ETargetPlatform::MAC:
        return EEndianness::LittleEndian;
    default:
        AssertNotImplemented();
    }
    return EEndianness::LittleEndian;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void FPlatform::CheckMemory() {
#ifdef OS_WINDOWS
    _CrtCheckMemory();
#else
#   error "no support"
#endif
}
#endif
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void FPlatform::DebugBreak() {
#ifdef OS_WINDOWS
    ::DebugBreak();
#else
#   error "no support"
#endif
}
#endif
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void FPlatform::DebugBreakAttach() {
#ifdef OS_WINDOWS
    if (::IsDebuggerPresent())
    {
        ::DebugBreak();
    }
#else
#   error "no support"
#endif
}
#endif
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
bool FPlatform::IsDebuggerAttached() {
#ifdef OS_WINDOWS
    return ::IsDebuggerPresent() ? true : false;
#else
#   error "no support"
#endif
}
#endif
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void FPlatform::OutputDebug(const char* text) {
#ifdef OS_WINDOWS
    return ::OutputDebugStringA(text);
#else
#   error "no support"
#endif
}
#endif
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void FPlatform::OutputDebug(const wchar_t* text) {
#ifdef OS_WINDOWS
    return ::OutputDebugStringW(text);
#else
#   error "no support"
#endif
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
