#include "stdafx.h"

#include "TargetPlatform.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const TargetPlatform gTargetPlatforms[] = {
    TargetPlatform::PC,
    TargetPlatform::PS4,
    TargetPlatform::XONE,
    TargetPlatform::MAC
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MemoryView<const TargetPlatform> EachTargetPlatform() {
    return MakeView(gTargetPlatforms);
}
//----------------------------------------------------------------------------
StringSlice TargetPlatformToCStr(TargetPlatform platform) {
    switch (platform)
    {
    case Core::TargetPlatform::PC:
        return StringSlice("PC");
    case Core::TargetPlatform::PS4:
        return StringSlice("PS4");
    case Core::TargetPlatform::XONE:
        return StringSlice("XONE");
    case Core::TargetPlatform::MAC:
        return StringSlice("MAC");
    default:
        AssertNotImplemented();
    }
    return StringSlice();
}
//----------------------------------------------------------------------------
Endianness TargetPlatformEndianness(TargetPlatform platform) {
    switch (platform)
    {
    case Core::TargetPlatform::PC:
        return Endianness::LittleEndian;
    case Core::TargetPlatform::PS4:
        return Endianness::LittleEndian;
    case Core::TargetPlatform::XONE:
        return Endianness::LittleEndian;
    case Core::TargetPlatform::MAC:
        return Endianness::LittleEndian;
    default:
        AssertNotImplemented();
    }
    return Endianness::LittleEndian;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core