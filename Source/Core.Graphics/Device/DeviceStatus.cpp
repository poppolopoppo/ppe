#include "stdafx.h"

#include "DeviceStatus.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const char *DeviceStatusToCStr(DeviceStatus status) {
    switch (status)
    {
    case Core::Graphics::DeviceStatus::Invalid:
        return "Invalid";
    case Core::Graphics::DeviceStatus::Normal:
        return "Normal";
    case Core::Graphics::DeviceStatus::Create:
        return "Create";
    case Core::Graphics::DeviceStatus::Destroy:
        return "Destroy";
    case Core::Graphics::DeviceStatus::Reset:
        return "Reset";
    case Core::Graphics::DeviceStatus::Lost:
        return "Lost";
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
