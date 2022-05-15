#pragma once

#include "Application_fwd.h"

#ifdef EXPORT_PPE_EXTENSIONS_APPLICATIONUI
#   define PPE_APPLICATIONUI_API DLL_EXPORT
#else
#   define PPE_APPLICATIONUI_API DLL_IMPORT
#endif

struct ImGuiContext;

namespace PPE {
class FApplicationUIModule;
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using PImguiContext = ImGuiContext*;
class FImguiService;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
