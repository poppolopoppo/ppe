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
    case PPE::Engine::EMouseButton::Button0:
        return MakeStringView("Button0");
    case PPE::Engine::EMouseButton::Button1:
        return MakeStringView("Button1");
    case PPE::Engine::EMouseButton::Button2:
        return MakeStringView("Button2");
    case PPE::Engine::EMouseButton::Wheel:
        return MakeStringView("Wheel");
    case PPE::Engine::EMouseButton::Thumb0:
        return MakeStringView("Thumb0");
    case PPE::Engine::EMouseButton::Thumb1:
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
