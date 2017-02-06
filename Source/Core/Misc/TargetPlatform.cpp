#include "stdafx.h"

#include "TargetPlatform.h"

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
} //!namespace Core
