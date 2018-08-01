#include "stdafx.h"

#include "DeviceStatus.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView DeviceStatusToCStr(EDeviceStatus status) {
    switch (status)
    {
    case Core::Graphics::EDeviceStatus::Invalid:
        return MakeStringView("Invalid");
    case Core::Graphics::EDeviceStatus::Normal:
        return MakeStringView("Normal");
    case Core::Graphics::EDeviceStatus::Create:
        return MakeStringView("Create");
    case Core::Graphics::EDeviceStatus::Destroy:
        return MakeStringView("Destroy");
    case Core::Graphics::EDeviceStatus::Reset:
        return MakeStringView("Reset");
    case Core::Graphics::EDeviceStatus::Lost:
        return MakeStringView("Lost");
    }
    AssertNotImplemented();
    return FStringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
