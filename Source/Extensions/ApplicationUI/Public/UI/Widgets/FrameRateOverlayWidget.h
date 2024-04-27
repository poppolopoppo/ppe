#pragma once

#include "UI/ImGui.h"

#include "IO/String.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFrameRateOverlayWidget {
public:
    enum class EOverlayLocation : i8 {
        Custom = -1,
        Center = -2,
        TopLeft = 0,
        TopRight = 1,
        BottomLeft = 2,
        BottomRight = 3,
    };

    TPtrRef<const FApplicationWindow> Application;

    FString Title{ ICON_CI_GRAPH_LINE " Frame-rate" };

    EOverlayLocation Location{ EOverlayLocation::TopRight };

    bool WindowVisible{ true };

    explicit PPE_APPLICATIONUI_API FFrameRateOverlayWidget(TPtrRef<const FApplicationWindow> application) NOEXCEPT;
    PPE_APPLICATIONUI_API ~FFrameRateOverlayWidget();

    NODISCARD PPE_APPLICATIONUI_API bool Show();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
