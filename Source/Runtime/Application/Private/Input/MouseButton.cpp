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
    case PPE::Application::EMouseButton::Button0:
        return MakeStringView("Button0");
    case PPE::Application::EMouseButton::Button1:
        return MakeStringView("Button1");
    case PPE::Application::EMouseButton::Button2:
        return MakeStringView("Button2");
    case PPE::Application::EMouseButton::Wheel:
        return MakeStringView("Wheel");
    case PPE::Application::EMouseButton::Thumb0:
        return MakeStringView("Thumb0");
    case PPE::Application::EMouseButton::Thumb1:
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