#include "stdafx.h"

#include "DeviceStatus.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringView DeviceStatusToCStr(DeviceStatus status) {
    switch (status)
    {
    case Core::Graphics::DeviceStatus::Invalid:
        return MakeStringView("Invalid");
    case Core::Graphics::DeviceStatus::Normal:
        return MakeStringView("Normal");
    case Core::Graphics::DeviceStatus::Create:
        return MakeStringView("Create");
    case Core::Graphics::DeviceStatus::Destroy:
        return MakeStringView("Destroy");
    case Core::Graphics::DeviceStatus::Reset:
        return MakeStringView("Reset");
    case Core::Graphics::DeviceStatus::Lost:
        return MakeStringView("Lost");
    }
    AssertNotImplemented();
    return StringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
