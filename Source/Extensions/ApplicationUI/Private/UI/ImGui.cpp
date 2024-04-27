// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "UI/ImGui.h"

#ifdef _WIN32
//  #TODO: remove workaround for static inline definition with header units
#   if _USE_32BIT_TIME_T
#       define mktime(_Tm) _mktime32(_Tm)
#       define _mkgmtime(_Tm) _mkgmtime32(_Tm)
#       define gmtime_s(_Tm, _Time) _gmtime32_s(_Tm, _Time)
#       define localtime_s(_Tm, _Time) _localtime32_s(_Tm, _Time)
#   else
#       define mktime(_Tm) _mktime64(_Tm)
#       define _mkgmtime(_Tm) _mkgmtime64(_Tm)
#       define gmtime_s(_Tm, _Time) _gmtime64_s(_Tm, _Time)
#       define localtime_s(_Tm, _Time) _localtime64_s(_Tm, _Time)
#   endif
#endif

#include "imgui-impl.h"

#include "Maths/MathHelpers.h"

#ifdef _WIN32
#   undef mktime
#   undef _mkgmtime
#   undef gmtime_s
#   undef localtime_s
#endif

namespace ImGui {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using namespace PPE;
//----------------------------------------------------------------------------
ImFont* GImGuiService_SmallFont{ nullptr };
ImFont* SmallFont() NOEXCEPT {
    return GImGuiService_SmallFont;
}
ImFont* GImGuiService_LargeFont{ nullptr };
ImFont* LargeFont() NOEXCEPT {
    return GImGuiService_LargeFont;
}
ImFont* GImGuiService_MonospaceFont{ nullptr };
ImFont* MonospaceFont() NOEXCEPT {
    return GImGuiService_MonospaceFont;
}
//----------------------------------------------------------------------------
bool Spinner(const char* label, float radius, float thickness, const ImU32& color) {

    /*ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;*/

    const ImGuiStyle& style = ImGui::GetStyle();
    const float2 pos = ImGui::GetCursorScreenPos();
    const float2 size((radius )*2, (radius + style.FramePadding.y)*2);

    ImGui::Dummy(size);

    // Render
    ImDrawList& drawList = *ImGui::GetWindowDrawList();

    drawList.PathClear();

    const double t = ImGui::GetTime();

    int num_segments = int(PI*2.f*radius/thickness);
    int start = RoundToInt(abs(std::sin(float(t*1.8))*(num_segments-5)));

    const float a_min = PI*2.f * ((float)start) / (float)num_segments;
    const float a_max = PI*2.f * ((float)num_segments-3) / (float)num_segments;

    const float2 center(pos.x+radius, pos.y+radius+style.FramePadding.y);

    drawList.AddRectFilled(drawList.GetClipRectMin(), drawList.GetClipRectMax(), ImGui::GetColorU32(ImGuiCol_FrameBg));

    for (int i = 0; i < num_segments; i++) {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        drawList.PathLineTo({
            center.x + std::cos(a+float(t*8)) * radius,
            center.y + std::sin(a+float(t*8)) * radius});
    }

    drawList.PathStroke(color, 0, thickness);

    const float2 textSize = ImGui::CalcTextSize(label);
    const float2 textPos = center + float2(-textSize.x / 2, radius + style.FramePadding.y + textSize.y);

    drawList.AddRectFilled(textPos - style.WindowPadding, textPos + textSize + style.WindowPadding, color, 5.f);
    drawList.AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), label);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ImGui
