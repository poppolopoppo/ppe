#include "stdafx.h"

#include "UI/Imgui.h"

PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4505) // unreferenced function with internal linkage removed
PRAGMA_MSVC_WARNING_DISABLE(4702) // unreachable code

#ifdef __clang__
#   pragma clang diagnostic push,
#   pragma clang diagnostic ignored "-Wsign-compare"
#   pragma clang diagnostic ignored "-Wreorder-ctor"
#   pragma clang diagnostic ignored "-Wunused-function"
#   pragma clang diagnostic ignored "-Wunused-variable"
#endif

#include "External/imgui/imgui.git/imgui.cpp"
#include "External/imgui/imgui.git/imgui_draw.cpp"
#include "External/imgui/imgui.git/imgui_tables.cpp"
#include "External/imgui/imgui.git/imgui_widgets.cpp"
#include "External/imgui/imgui.git/imgui_demo.cpp"

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

PRAGMA_MSVC_WARNING_POP()

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
