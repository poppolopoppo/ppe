#include "stdafx.h"

#include "TargetPlatform.h"

#ifdef PLATFORM_WINDOWS
#   include "Misc/Platform_Windows.h"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static constexpr ETargetPlatform GTargetPlatforms[] = {
    ETargetPlatform::PC,
    ETargetPlatform::PS4,
    ETargetPlatform::XONE,
    ETargetPlatform::MAC,
    ETargetPlatform::LINUX
};
//----------------------------------------------------------------------------
#if     defined(PLATFORM_WINDOWS)
struct FWindowsSystemInfo_ : FPlatform::FSystemInfo {
    FWindowsSystemInfo_() {
        ::SYSTEM_INFO st;
        ::GetSystemInfo(&st);

        AllocationGranularity = checked_cast<size_t>(st.dwAllocationGranularity);
        PageSize = checked_cast<size_t>(st.dwPageSize);
        ProcessorsCount = checked_cast<size_t>(st.dwNumberOfProcessors);

        AssertRelease(PAGE_SIZE == AllocationGranularity);
    }
};
static const FWindowsSystemInfo_ GSystemInfo;
#else
#   error "unsupported platform"
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TMemoryView<const ETargetPlatform> EachTargetPlatform() {
    return MakeView(GTargetPlatforms);
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
const FPlatform::FSystemInfo& FPlatform::SystemInfo = GSystemInfo;
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void FPlatform::CheckMemory() {
#ifdef PLATFORM_WINDOWS
    _CrtCheckMemory();
#else
#   error "no support"
#endif
}
#endif
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void FPlatform::DebugBreak() {
#ifdef PLATFORM_WINDOWS
    ::DebugBreak();
#else
#   error "no support"
#endif
}
#endif
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void FPlatform::DebugBreakAttach() {
#ifdef PLATFORM_WINDOWS
    if (::IsDebuggerPresent()) {
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
#ifdef PLATFORM_WINDOWS
    return ::IsDebuggerPresent() ? true : false;
#else
#   error "no support"
#endif
}
#endif
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void FPlatform::OutputDebug(const char* text) {
#ifdef PLATFORM_WINDOWS
    return ::OutputDebugStringA(text);
#else
#   error "no support"
#endif
}
#endif
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void FPlatform::OutputDebug(const wchar_t* text) {
#ifdef PLATFORM_WINDOWS
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
