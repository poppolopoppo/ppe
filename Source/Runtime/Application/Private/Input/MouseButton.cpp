// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Input/MouseButton.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static constexpr EMouseAxis GEachMouseAxises[] = {
    EMouseAxis::Pointer,
    EMouseAxis::ScrollWheelY,
    EMouseAxis::ScrollWheelX,
};
//----------------------------------------------------------------------------
static constexpr EMouseButton GEachMouseButtons[] = {
    EMouseButton::Button0,
    EMouseButton::Button1,
    EMouseButton::Button2,
    EMouseButton::Thumb0,
    EMouseButton::Thumb1,
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TMemoryView<const EMouseAxis> EachMouseAxises() NOEXCEPT {
    return GEachMouseAxises;
}
//----------------------------------------------------------------------------
TMemoryView<const EMouseButton> EachMouseButton() NOEXCEPT {
    return GEachMouseButtons;
}
//----------------------------------------------------------------------------
FStringLiteral MouseAxisToCStr(EMouseAxis value) noexcept {
    switch (value) {
    case EMouseAxis::Pointer: return "MousePointer";
    case EMouseAxis::ScrollWheelY: return "MouseScrollWheelY";
    case EMouseAxis::ScrollWheelX: return "MouseScrollWheelX";
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
FStringLiteral MouseButtonToCStr(EMouseButton value) NOEXCEPT {
    switch (value)
    {
    case PPE::Application::EMouseButton::Button0:
        return "MouseButton0";
    case PPE::Application::EMouseButton::Button1:
        return "MouseButton1";
    case PPE::Application::EMouseButton::Button2:
        return "MouseButton2";
    case PPE::Application::EMouseButton::Thumb0:
        return "MouseThumb0";
    case PPE::Application::EMouseButton::Thumb1:
        return "MouseThumb1";
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
