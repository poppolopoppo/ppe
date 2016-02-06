#include "stdafx.h"

#include "DeviceStatus.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringSlice DeviceStatusToCStr(DeviceStatus status) {
    switch (status)
    {
    case Core::Graphics::DeviceStatus::Invalid:
        return MakeStringSlice("Invalid");
    case Core::Graphics::DeviceStatus::Normal:
        return MakeStringSlice("Normal");
    case Core::Graphics::DeviceStatus::Create:
        return MakeStringSlice("Create");
    case Core::Graphics::DeviceStatus::Destroy:
        return MakeStringSlice("Destroy");
    case Core::Graphics::DeviceStatus::Reset:
        return MakeStringSlice("Reset");
    case Core::Graphics::DeviceStatus::Lost:
        return MakeStringSlice("Lost");
    }
    AssertNotImplemented();
    return StringSlice();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
