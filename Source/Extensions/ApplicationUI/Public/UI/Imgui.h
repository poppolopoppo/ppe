#pragma once

#ifdef IMGUI_API
#   error "tototo IMGUI_API"
#endif

#include "ApplicationUI_fwd.h"

#ifdef IMGUI_API
#   error "should be included before any reference to imgui.h" IMGUI_API
#endif

#define IMGUI_API PPE_APPLICATIONUI_API
#include "External/imgui/Public/imgui-external.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
