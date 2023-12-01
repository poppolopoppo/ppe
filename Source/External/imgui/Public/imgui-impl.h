#pragma once

#include "imgui-external.h"

#ifdef __clang__
#   pragma clang system_header
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wreorder-ctor"
#   pragma clang diagnostic ignored "-Wsign-compare"
#   pragma clang diagnostic ignored "-Wunused-function"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#   pragma clang diagnostic ignored "-Wunused-variable"
#endif

#ifdef __gcc__
#   pragma GCC system_header
#endif

PRAGMA_MSVC_WARNING_PUSH()

PRAGMA_MSVC_WARNING_DISABLE(4189) // tab': local variable is initialized but not referenced
PRAGMA_MSVC_WARNING_DISABLE(4505) // 'GetInputSourceName': unreferenced function with internal linkage has been removed
PRAGMA_MSVC_WARNING_DISABLE(4702) // unreachable code

#include "External/imgui/imgui.git/imgui.cpp"
#include "External/imgui/imgui.git/imgui_draw.cpp"
#include "External/imgui/imgui.git/imgui_tables.cpp"
#include "External/imgui/imgui.git/imgui_widgets.cpp"
#include "External/imgui/imgui.git/imgui_demo.cpp"

#include "External/imgui/implot.git/implot.cpp"
#include "External/imgui/implot.git/implot_items.cpp"
#include "External/imgui/implot.git/implot_demo.cpp"

PRAGMA_MSVC_WARNING_POP()

#ifdef __clang__
#   pragma clang diagnostic pop
#endif
