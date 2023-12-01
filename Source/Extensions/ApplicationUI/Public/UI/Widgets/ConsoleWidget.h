#pragma once

#include "UI/Imgui.h"

#include "Container/FlatSet.h"
#include "Container/RingBuffer.h"
#include "Diagnostic/Logger.h"
#include "IO/String.h"
#include "Memory/UniquePtr.h"

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
