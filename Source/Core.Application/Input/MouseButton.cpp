#include "stdafx.h"

#include "MouseButton.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringView MouseButtonToCStr(MouseButton value) {
    switch (value)
    {
    case Core::Application::MouseButton::Button0:
        return MakeStringView("Button0");
    case Core::Application::MouseButton::Button1:
        return MakeStringView("Button1");
    case Core::Application::MouseButton::Button2:
        return MakeStringView("Button2");
    case Core::Application::MouseButton::Wheel:
        return MakeStringView("Wheel");
    case Core::Application::MouseButton::Thumb0:
        return MakeStringView("Thumb0");
    case Core::Application::MouseButton::Thumb1:
        return MakeStringView("Thumb1");
    }
    AssertNotImplemented();
    return StringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
