#pragma once

#include "HAL/PlatformMacros.h"

#include "Maths/ScalarVector.h"
#include "Meta/Assert.h"

#ifdef CPP_CLANG
#    pragma clang system_header
#endif

#ifdef CPP_GCC
#    pragma GCC system_header
#endif

#ifndef EXPORT_PPE_EXTERNAL_IMGUI
    PRAGMA_MSVC_WARNING_PUSH()

#   ifdef __clang__
#       pragma clang diagnostic push,
#   endif

#endif

#ifndef IMGUI_API
#   ifndef EXPORT_PPE_EXTENSIONS_APPLICATIONUI
#       define IMGUI_API DLL_IMPORT
#   else
#       define IMGUI_API DLL_EXPORT
#   endif
#endif

PRAGMA_MSVC_WARNING_DISABLE(4505) // 'GetInputSourceName': unreferenced function with internal linkage has been removed
PRAGMA_MSVC_WARNING_DISABLE(4702) // unreachable code

#ifdef CPP_CLANG
#   pragma clang diagnostic ignored "-Wunused-function"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#   pragma clang diagnostic ignored "-Wunused-variable"
#endif

#define IM_ASSERT(_EXPR) Assert(_EXPR)
#define IM_DEBUG_BREAK() PPE_DEBUG_BREAK()

#define IMGUI_DEFINE_MATH_OPERATORS

#define IM_VEC2_CLASS_EXTRA \
    CONSTEXPR ImVec2(const PPE::float2& f) : x(f.x), y(f.y) {} \
    CONSTEXPR operator PPE::float2 () const { return { x, y }; }
#define IM_VEC4_CLASS_EXTRA \
    CONSTEXPR ImVec4(const PPE::float4& f) : x(f.x), y(f.y), z(f.z), w(f.w) {} \
    CONSTEXPR operator PPE::float4 () const { return { x, y, z, w }; }

// #define IMGUI_DISABLE_DEMO_WINDOWS
#if USE_PPE_PROFILING || !USE_PPE_FINAL_RELEASE
#   define IMGUI_DISABLE_METRICS_WINDOW
#   if USE_PPE_DEBUG
#       define IMGUI_DEBUG_PARANOID
#   endif
#endif

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DISABLE_OBSOLETE_KEYIO

#define IMGUI_USE_STB_SPRINTF

#define IMGUI_STB_TRUETYPE_FILENAME   "External/stb/stb.git/stb_truetype.h"
#define IMGUI_STB_RECT_PACK_FILENAME  "External/stb/stb.git/stb_rect_pack.h"
#define IMGUI_STB_SPRINTF_FILENAME    "External/stb/stb.git/stb_sprintf.h"

#pragma include_alias("imconfig.h", "External/imgui/imgui.git/imconfig.h")
#pragma include_alias("imgui.h", "External/imgui/imgui.git/imgui.h")
#pragma include_alias("imgui_internal.h", "External/imgui/imgui.git/imgui_internal.h")

#ifndef EXPORT_PPE_EXTERNAL_IMGUI

#   pragma include_alias("imstb_rectpack.h", "External/imgui/imgui.git/imstb_rectpack.h")
#   pragma include_alias("imstb_textedit.h", "External/imgui/imgui.git/imstb_textedit.h")
#   pragma include_alias("imstb_truetype.h", "External/imgui/imgui.git/imstb_truetype.h")

#   include "External/imgui/imgui.git/imgui.h"

#   ifdef __clang__
#       pragma clang diagnostic pop
#   endif

    PRAGMA_MSVC_WARNING_POP()

#endif //!EXPORT_PPE_EXTERNAL_IMGUI
