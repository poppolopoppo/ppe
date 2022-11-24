// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Input/MouseButton.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView MouseButtonToCStr(EMouseButton value) {
    switch (value)
    {
    case PPE::Application::EMouseButton::Button0:
        return MakeStringView("MouseButton0");
    case PPE::Application::EMouseButton::Button1:
        return MakeStringView("MouseButton1");
    case PPE::Application::EMouseButton::Button2:
        return MakeStringView("MouseButton2");
    case PPE::Application::EMouseButton::Thumb0:
        return MakeStringView("MouseThumb0");
    case PPE::Application::EMouseButton::Thumb1:
        return MakeStringView("MouseThumb1");
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
