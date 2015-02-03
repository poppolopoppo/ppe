#include "stdafx.h"

#include "MouseButton.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const char *MouseButtonToCStr(MouseButton value) {
    switch (value)
    {
    case Core::Engine::MouseButton::Button0:
        return "Button0";
    case Core::Engine::MouseButton::Button1:
        return "Button1";
    case Core::Engine::MouseButton::Button2:
        return "Button2";
    case Core::Engine::MouseButton::Wheel:
        return "Wheel";
    case Core::Engine::MouseButton::Thumb0:
        return "Thumb0";
    case Core::Engine::MouseButton::Thumb1:
        return "Thumb1";
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
