#pragma once

#include "Application.h"

#include "IO/StringView.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EMouseButton : u8 {
    Button0 = 0,    // Left (assuming left handed)
    Button1,        // Right
    Button2,        // Middle

    Wheel,          // Scroll

    Thumb0,         // Previous
    Thumb1          // Next
};
//----------------------------------------------------------------------------
FStringView MouseButtonToCStr(EMouseButton value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
