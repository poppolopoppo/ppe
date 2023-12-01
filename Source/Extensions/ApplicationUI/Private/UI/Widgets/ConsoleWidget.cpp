// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "UI/Widgets/ConsoleWidget.h"

#include "UI/Imgui.h"

#include "Diagnostic/CurrentProcess.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATIONUI_API, UI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FConsoleWidget::FConsoleWidget() NOEXCEPT
{}
//----------------------------------------------------------------------------
FConsoleWidget::~FConsoleWidget() NOEXCEPT
{}
//----------------------------------------------------------------------------
bool FConsoleWidget::Show() {
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
