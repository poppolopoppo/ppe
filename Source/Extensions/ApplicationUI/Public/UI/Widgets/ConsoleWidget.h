#pragma once

#include "UI/ImGui.h"

#include "IO/String.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FConsoleWidget {
public:
    FString Title{ ICON_CI_OUTPUT " Console" };

    PPE_APPLICATIONUI_API FConsoleWidget() NOEXCEPT;
    PPE_APPLICATIONUI_API ~FConsoleWidget();

    NODISCARD PPE_APPLICATIONUI_API bool Show();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
