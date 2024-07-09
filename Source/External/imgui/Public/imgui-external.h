#pragma once

#include "HAL/PlatformMacros.h"

#include "Maths/ScalarVector.h"
#include "Meta/Assert.h"
#include "Meta/StronglyTyped.h"

#ifdef __clang__
#    pragma clang system_header
#endif

#ifdef __gcc__
#    pragma GCC system_header
#endif

#ifndef EXPORT_PPE_EXTERNAL_IMGUI
    PRAGMA_MSVC_WARNING_PUSH()

#   ifdef __clang__
#       pragma clang diagnostic push
#   endif

#else

#   include "HAL/PlatformIncludes.h"

#endif

#ifndef IMGUI_API
#   ifndef EXPORT_PPE_EXTENSIONS_APPLICATIONUI
#       define IMGUI_API DLL_IMPORT
#   else
#       define IMGUI_API DLL_EXPORT
#   endif
#endif

PRAGMA_MSVC_WARNING_DISABLE(4189) // tab': local variable is initialized but not referenced
PRAGMA_MSVC_WARNING_DISABLE(4505) // 'GetInputSourceName': unreferenced function with internal linkage has been removed
PRAGMA_MSVC_WARNING_DISABLE(4668) // undefined preprocessor macro, replacing with '0'
PRAGMA_MSVC_WARNING_DISABLE(4702) // unreachable code

#ifdef __clang__
#   pragma clang diagnostic ignored "-Wreorder-ctor"
#   pragma clang diagnostic ignored "-Wsign-compare"
#   pragma clang diagnostic ignored "-Wunused-function"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#   pragma clang diagnostic ignored "-Wunused-variable"
#endif

#define IM_ASSERT(_EXPR) Assert(_EXPR)

// Use 'Metrics/Debugger->Tools->Item Picker' to break into the call-stack of a specific item.
#define IM_DEBUG_BREAK() PPE_DEBUG_BREAK()

// Define additional constructors and implicit cast operators in imconfig.h to convert back and forth between your math types and ImVec2.
#define IM_VEC2_CLASS_EXTRA \
    inline CONSTEXPR ImVec2(const ::PPE::float2& f) : x(f.x), y(f.y) {} \
    template <typename _Expr> inline CONSTEXPR ImVec2(const ::PPE::details::TScalarVectorExpr<float, 2, _Expr>& f) : x(f[0]), y(f[1]) {} \
    inline CONSTEXPR operator ::PPE::float2 () const { return { x, y }; } \
    NODISCARD inline CONSTEXPR ::PPE::float2 ToFloat2() const { return { x, y }; }
#define IM_VEC4_CLASS_EXTRA \
    inline CONSTEXPR ImVec4(const ::PPE::float4& f) : x(f.x), y(f.y), z(f.z), w(f.w) {} \
    template <typename _Expr> inline CONSTEXPR ImVec4(const ::PPE::details::TScalarVectorExpr<float, 4, _Expr>& f) : x(f[0]), y(f[1]), z(f[2]), w(f[3]) {} \
    inline CONSTEXPR operator ::PPE::float4 () const { return { x, y, z, w }; } \
    NODISCARD inline CONSTEXPR ::PPE::float4 ToFloat4() const { return { x, y, z, w }; }

// You can use '#define IMGUI_DEFINE_MATH_OPERATORS' to import our operators, provided as a courtesy.
// #define IMGUI_DEFINE_MATH_OPERATORS

// ImTexture: user data for renderer backend to identify a texture [Compile-time configurable type]
namespace PPE {
PPE_STRONGLYTYPED_NUMERIC_DEF(u64, FImTexturePackedID);
}
#define ImTextureID ::PPE::FImTexturePackedID

// #define IMGUI_DISABLE_DEMO_WINDOWS
#if USE_PPE_PROFILING || USE_PPE_FINAL_RELEASE
#   define IMGUI_DISABLE_DEBUG_TOOLS
#elif USE_PPE_DEBUG
#   define IMGUI_DEBUG_PARANOID
#endif

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DISABLE_OBSOLETE_KEYIO

#define IMGUI_USE_STB_SPRINTF

#define IMPLOT_API IMGUI_API

#define IMGUI_STB_TRUETYPE_FILENAME             "External/stb/stb.git/stb_truetype.h"
#define IMGUI_STB_RECT_PACK_FILENAME            "External/stb/stb.git/stb_rect_pack.h"
#define IMGUI_STB_SPRINTF_FILENAME              "External/stb/stb.git/stb_sprintf.h"

#pragma include_alias("imconfig.h",             "External/imgui/imgui.git/imconfig.h")
#pragma include_alias("imgui.h",                "External/imgui/imgui.git/imgui.h")
#pragma include_alias("imgui_internal.h",       "External/imgui/imgui.git/imgui_internal.h")

#pragma include_alias("implot.h",               "External/imgui/implot.git/implot.h")
#pragma include_alias("implot_internal.h",      "External/imgui/implot.git/implot_internal.h")

#ifndef EXPORT_PPE_EXTERNAL_IMGUI

#   pragma include_alias("imstb_rectpack.h",    "External/imgui/imgui.git/imstb_rectpack.h")
#   pragma include_alias("imstb_textedit.h",    "External/imgui/imgui.git/imstb_textedit.h")
#   pragma include_alias("imstb_truetype.h",    "External/imgui/imgui.git/imstb_truetype.h")

#   include "External/imgui/imgui.git/imgui.h"
// #   include "External/imgui/imgui.git/imgui_internal.h"

// fix header unit with inline constexpr variant of ImGui math operators (which are static)
inline CONSTEXPR ImVec2  operator*(const ImVec2& lhs, const float rhs)     { return ImVec2(lhs.x * rhs, lhs.y * rhs); }
inline CONSTEXPR ImVec2  operator/(const ImVec2& lhs, const float rhs)     { return ImVec2(lhs.x / rhs, lhs.y / rhs); }
inline CONSTEXPR ImVec2  operator+(const ImVec2& lhs, const ImVec2& rhs)   { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
inline CONSTEXPR ImVec2  operator-(const ImVec2& lhs, const ImVec2& rhs)   { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
inline CONSTEXPR ImVec2  operator*(const ImVec2& lhs, const ImVec2& rhs)   { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
inline CONSTEXPR ImVec2  operator/(const ImVec2& lhs, const ImVec2& rhs)   { return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y); }
inline CONSTEXPR ImVec2  operator-(const ImVec2& lhs)                      { return ImVec2(-lhs.x, -lhs.y); }
inline CONSTEXPR ImVec2& operator*=(ImVec2& lhs, const float rhs)          { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
inline CONSTEXPR ImVec2& operator/=(ImVec2& lhs, const float rhs)          { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
inline CONSTEXPR ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs)        { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
inline CONSTEXPR ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs)        { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
inline CONSTEXPR ImVec2& operator*=(ImVec2& lhs, const ImVec2& rhs)        { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
inline CONSTEXPR ImVec2& operator/=(ImVec2& lhs, const ImVec2& rhs)        { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }
inline CONSTEXPR bool    operator==(const ImVec2& lhs, const ImVec2& rhs)  { return lhs.x == rhs.x && lhs.y == rhs.y; }
inline CONSTEXPR bool    operator!=(const ImVec2& lhs, const ImVec2& rhs)  { return lhs.x != rhs.x || lhs.y != rhs.y; }
inline CONSTEXPR ImVec4  operator+(const ImVec4& lhs, const ImVec4& rhs)   { return ImVec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
inline CONSTEXPR ImVec4  operator-(const ImVec4& lhs, const ImVec4& rhs)   { return ImVec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); }
inline CONSTEXPR ImVec4  operator*(const ImVec4& lhs, const ImVec4& rhs)   { return ImVec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }
inline CONSTEXPR bool    operator==(const ImVec4& lhs, const ImVec4& rhs)  { return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w; }
inline CONSTEXPR bool    operator!=(const ImVec4& lhs, const ImVec4& rhs)  { return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z || lhs.w != rhs.w; }

#   include "External/imgui/implot.git/implot.h"
// #   include "External/imgui/implot.git/implot_internal.h"

#   include "External/imgui/IconFontCppHeaders.git/IconsCodicons.h"
//#   include "External/imgui/IconFontCppHeaders.git/IconsFontaudio.h"
//#   include "External/imgui/IconFontCppHeaders.git/IconsFontAwesome4.h"
//#   include "External/imgui/IconFontCppHeaders.git/IconsFontAwesome5.h"
//#   include "External/imgui/IconFontCppHeaders.git/IconsFontAwesome5Brands.h"
//#   include "External/imgui/IconFontCppHeaders.git/IconsFontAwesome5Pro.h"
//#   include "External/imgui/IconFontCppHeaders.git/IconsFontAwesome5ProBrands.h"
//#   include "External/imgui/IconFontCppHeaders.git/IconsFontAwesome6.h"
//#   include "External/imgui/IconFontCppHeaders.git/IconsFontAwesome6Brands.h"
#   include "External/imgui/IconFontCppHeaders.git/IconsForkAwesome.h"
#   include "External/imgui/IconFontCppHeaders.git/IconsKenney.h"
//#   include "External/imgui/IconFontCppHeaders.git/IconsMaterialDesign.h"
//#   include "External/imgui/IconFontCppHeaders.git/IconsMaterialDesignIcons.h"

#   ifdef __clang__
#       pragma clang diagnostic pop
#   endif

    PRAGMA_MSVC_WARNING_POP()

#endif //!EXPORT_PPE_EXTERNAL_IMGUI
