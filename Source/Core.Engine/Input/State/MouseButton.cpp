#include "stdafx.h"

#include "MouseButton.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView MouseButtonToCStr(EMouseButton value) {
    switch (value)
    {
    case Core::Engine::EMouseButton::Button0:
        return MakeStringView("Button0");
    case Core::Engine::EMouseButton::Button1:
        return MakeStringView("Button1");
    case Core::Engine::EMouseButton::Button2:
        return MakeStringView("Button2");
    case Core::Engine::EMouseButton::Wheel:
        return MakeStringView("Wheel");
    case Core::Engine::EMouseButton::Thumb0:
        return MakeStringView("Thumb0");
    case Core::Engine::EMouseButton::Thumb1:
        return MakeStringView("Thumb1");
    }
    AssertNotImplemented();
    return FStringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
