// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "UI/Widgets/FrameRateOverlayWidget.h"

#include "UI/ImGui.h"

#include "Application/ApplicationWindow.h"

#include "Color/Color.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATIONUI_API, UI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FFrameRateOverlayWidget::FFrameRateOverlayWidget(TPtrRef<const FApplicationWindow> application) NOEXCEPT
:   Application(application)
{}
//----------------------------------------------------------------------------
FFrameRateOverlayWidget::~FFrameRateOverlayWidget()
{}
//----------------------------------------------------------------------------
bool FFrameRateOverlayWidget::Show() {
    ImGuiWindowFlags overlayWindowFlags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    if (static_cast<i8>(Location) >= 0) {
        const float PAD = 10.0f;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        const ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        const ImVec2 work_size = viewport->WorkSize;

        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (static_cast<i8>(Location) & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
        window_pos.y = (static_cast<i8>(Location) & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
        window_pos_pivot.x = (static_cast<i8>(Location) & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (static_cast<i8>(Location) & 2) ? 1.0f : 0.0f;

        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowViewport(viewport->ID);

        overlayWindowFlags |= ImGuiWindowFlags_NoMove;
    }
    else if (Location == EOverlayLocation::Center) {
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        overlayWindowFlags |= ImGuiWindowFlags_NoMove;
    }

    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

    DEFERRED{ ImGui::End(); };
    if (not ImGui::Begin("##FrameRateOverlay", &WindowVisible, overlayWindowFlags))
        return false;

    if (ImGui::BeginTable("##FrameRateOverlay/Table", 2,
        ImGuiTableFlags_NoBordersInBody,
        float2(100, 0))) {

        auto printCounter = [](FStringLiteral text, FStringLiteral fmt, double value, const float4& color){
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(color, "%s", text.c_str());

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(color, fmt.c_str(), value);
        };

        const FSeconds dt = Application->RealTime().Elapsed();

        printCounter("FPS", "%.2f", 1.0 / dt.Value(), FLinearColor::Yellow());
        printCounter("DT", "%.3fms", FMilliseconds(dt).Value(), FLinearColor::LightYellow());
        printCounter("Tick", "%.3fms", FMilliseconds(Application->TickTime().Average).Value(), FLinearColor::Cyan());
        printCounter("Pump", "%.3fms", FMilliseconds(Application->MessageTime().Average).Value(), FLinearColor::LightCyan());

        ImGui::EndTable();
    }

    if (ImGui::BeginPopupContextWindow()) {
        if (ImGui::MenuItem("Custom", nullptr, Location == EOverlayLocation::Custom)) Location = EOverlayLocation::Custom;
        if (ImGui::MenuItem("Center", nullptr, Location == EOverlayLocation::Center)) Location = EOverlayLocation::Center;
        if (ImGui::MenuItem("Top-left", nullptr, Location == EOverlayLocation::TopLeft)) Location = EOverlayLocation::TopLeft;
        if (ImGui::MenuItem("Top-right", nullptr, Location == EOverlayLocation::TopRight)) Location = EOverlayLocation::TopRight;
        if (ImGui::MenuItem("Bottom-left", nullptr, Location == EOverlayLocation::BottomLeft)) Location = EOverlayLocation::BottomLeft;
        if (ImGui::MenuItem("Bottom-right", nullptr, Location == EOverlayLocation::BottomRight)) Location = EOverlayLocation::BottomRight;

        if (ImGui::MenuItem("Close")) WindowVisible = false;

        ImGui::EndPopup();
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
