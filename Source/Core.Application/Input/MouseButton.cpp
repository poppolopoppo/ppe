#include "stdafx.h"

#include "MouseButton.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringSlice MouseButtonToCStr(MouseButton value) {
    switch (value)
    {
    case Core::Application::MouseButton::Button0:
        return MakeStringSlice("Button0");
    case Core::Application::MouseButton::Button1:
        return MakeStringSlice("Button1");
    case Core::Application::MouseButton::Button2:
        return MakeStringSlice("Button2");
    case Core::Application::MouseButton::Wheel:
        return MakeStringSlice("Wheel");
    case Core::Application::MouseButton::Thumb0:
        return MakeStringSlice("Thumb0");
    case Core::Application::MouseButton::Thumb1:
        return MakeStringSlice("Thumb1");
    }
    AssertNotImplemented();
    return StringSlice();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
