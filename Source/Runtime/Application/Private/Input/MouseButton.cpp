#include "stdafx.h"

#include "MouseButton.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView MouseButtonToCStr(EMouseButton value) {
    switch (value)
    {
    case Core::Application::EMouseButton::Button0:
        return MakeStringView("Button0");
    case Core::Application::EMouseButton::Button1:
        return MakeStringView("Button1");
    case Core::Application::EMouseButton::Button2:
        return MakeStringView("Button2");
    case Core::Application::EMouseButton::Wheel:
        return MakeStringView("Wheel");
    case Core::Application::EMouseButton::Thumb0:
        return MakeStringView("Thumb0");
    case Core::Application::EMouseButton::Thumb1:
        return MakeStringView("Thumb1");
    }
    AssertNotImplemented();
    return FStringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
