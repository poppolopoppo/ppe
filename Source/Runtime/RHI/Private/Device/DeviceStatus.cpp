#include "stdafx.h"

#include "DeviceStatus.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView DeviceStatusToCStr(EDeviceStatus status) {
    switch (status)
    {
    case PPE::Graphics::EDeviceStatus::Invalid:
        return MakeStringView("Invalid");
    case PPE::Graphics::EDeviceStatus::Normal:
        return MakeStringView("Normal");
    case PPE::Graphics::EDeviceStatus::Create:
        return MakeStringView("Create");
    case PPE::Graphics::EDeviceStatus::Destroy:
        return MakeStringView("Destroy");
    case PPE::Graphics::EDeviceStatus::Reset:
        return MakeStringView("Reset");
    case PPE::Graphics::EDeviceStatus::Lost:
        return MakeStringView("Lost");
    }
    AssertNotImplemented();
    return FStringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
