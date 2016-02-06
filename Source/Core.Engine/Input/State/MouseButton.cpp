#include "stdafx.h"

#include "MouseButton.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringSlice MouseButtonToCStr(MouseButton value) {
    switch (value)
    {
    case Core::Engine::MouseButton::Button0:
        return MakeStringSlice("Button0");
    case Core::Engine::MouseButton::Button1:
        return MakeStringSlice("Button1");
    case Core::Engine::MouseButton::Button2:
        return MakeStringSlice("Button2");
    case Core::Engine::MouseButton::Wheel:
        return MakeStringSlice("Wheel");
    case Core::Engine::MouseButton::Thumb0:
        return MakeStringSlice("Thumb0");
    case Core::Engine::MouseButton::Thumb1:
        return MakeStringSlice("Thumb1");
    }
    AssertNotImplemented();
    return StringSlice();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
