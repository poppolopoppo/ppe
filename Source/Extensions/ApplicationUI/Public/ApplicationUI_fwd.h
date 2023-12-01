#pragma once

#include "Application_fwd.h"

#ifdef EXPORT_PPE_EXTENSIONS_APPLICATIONUI
#   define PPE_APPLICATIONUI_API DLL_EXPORT
#else
#   define PPE_APPLICATIONUI_API DLL_IMPORT
#endif

struct ImGuiContext;
struct ImPlotContext;

namespace PPE {
class FApplicationUIModule;
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using PImGuiContext = ImGuiContext*;
using PImPlotContext = ImPlotContext*;
class FImGuiService;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
