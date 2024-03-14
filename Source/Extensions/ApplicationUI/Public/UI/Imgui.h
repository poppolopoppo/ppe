#pragma once

#include "ApplicationUI_fwd.h"

#ifdef IMGUI_API
#   error "should be included before any reference to imgui.h" IMGUI_API
#endif

#define IMGUI_API PPE_APPLICATIONUI_API
#include "External/imgui/Public/imgui-external.h"

namespace ImGui {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_APPLICATIONUI_API ImFont* SmallFont() NOEXCEPT;
NODISCARD PPE_APPLICATIONUI_API ImFont* LargeFont() NOEXCEPT;
NODISCARD PPE_APPLICATIONUI_API ImFont* MonospaceFont() NOEXCEPT;
//----------------------------------------------------------------------------
PPE_APPLICATIONUI_API bool Spinner(const char* label, float radius, float thickness, const ImU32& color);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ImGui
