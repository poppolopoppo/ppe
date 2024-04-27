#pragma once

#include "Application.h"

#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EMouseAxis : u8 {
    Pointer = 0,
    ScrollWheelY,
    ScrollWheelX
};
//----------------------------------------------------------------------------
NODISCARD PPE_APPLICATION_API TMemoryView<const EMouseAxis> EachMouseAxises() NOEXCEPT;
NODISCARD PPE_APPLICATION_API FStringLiteral MouseAxisToCStr(EMouseAxis value) NOEXCEPT;
//----------------------------------------------------------------------------
enum class EMouseButton : u8 {
    Button0 = 0,    // Left (assuming left handed)
    Button1,        // Right
    Button2,        // Middle
    Thumb0,         // Previous
    Thumb1,         // Next

    Left = Button0,
    Right = Button1,
    Middle = Button2,
};
//----------------------------------------------------------------------------
NODISCARD PPE_APPLICATION_API TMemoryView<const EMouseButton> EachMouseButton() NOEXCEPT;
NODISCARD PPE_APPLICATION_API FStringLiteral MouseButtonToCStr(EMouseButton value) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, EMouseAxis value) {
    return oss << MouseAxisToCStr(value);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, EMouseButton value) {
    return oss << MouseButtonToCStr(value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
