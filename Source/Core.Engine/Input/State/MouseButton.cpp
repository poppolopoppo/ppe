#include "stdafx.h"

#include "MouseButton.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringView MouseButtonToCStr(MouseButton value) {
    switch (value)
    {
    case Core::Engine::MouseButton::Button0:
        return MakeStringView("Button0");
    case Core::Engine::MouseButton::Button1:
        return MakeStringView("Button1");
    case Core::Engine::MouseButton::Button2:
        return MakeStringView("Button2");
    case Core::Engine::MouseButton::Wheel:
        return MakeStringView("Wheel");
    case Core::Engine::MouseButton::Thumb0:
        return MakeStringView("Thumb0");
    case Core::Engine::MouseButton::Thumb1:
        return MakeStringView("Thumb1");
    }
    AssertNotImplemented();
    return StringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
