#include "stdafx.h"

#include "MouseButton.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const char *MouseButtonToCStr(MouseButton value) {
    switch (value)
    {
    case Core::Application::MouseButton::Button0:
        return "Button0";
    case Core::Application::MouseButton::Button1:
        return "Button1";
    case Core::Application::MouseButton::Button2:
        return "Button2";
    case Core::Application::MouseButton::Wheel:
        return "Wheel";
    case Core::Application::MouseButton::Thumb0:
        return "Thumb0";
    case Core::Application::MouseButton::Thumb1:
        return "Thumb1";
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
