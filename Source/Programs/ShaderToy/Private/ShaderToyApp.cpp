// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "ShaderToyApp.h"

#include "ApplicationModule.h"

#include "RHIModule.h"

#include "UI/Imgui.h"
#include "External/imgui/Public/imgui-internal.h"

#include "UI/Widgets/ConsoleWidget.h"
#include "UI/Widgets/FileDialogWidget.h"
#include "UI/Widgets/LogViewerWidget.h"
#include "UI/Widgets/MemoryUsageWidget.h"
#include "UI/Widgets/FrameRateOverlayWidget.h"

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "Maths/Threefy.h"
#include "Memory/UniquePtr.h"

namespace PPE {
LOG_CATEGORY(, ShaderToy)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FShaderToyApp::FShaderToyApp(FModularDomain& domain)
:   parent_type(domain, "Tools/ShaderToy", true) {

    FRHIModule& rhiModule = FRHIModule::Get(domain);
    rhiModule.SetStagingBufferSize(8_MiB);
}
//----------------------------------------------------------------------------
FShaderToyApp::~FShaderToyApp() = default;
//----------------------------------------------------------------------------
void FShaderToyApp::Start() {
    parent_type::Start();

    /*auto lang = TextEditor::LanguageDefinition::GLSL();

    _editor.create();
    _editor->SetLanguageDefinition(lang);*/

    _OnApplicationTick.Emplace([fileDialog(MakeUnique<Application::FFileDialogWidget>())](const IApplicationService&, FTimespan) {
        FFilename selectedFile;
        if (fileDialog->Show()) {
            if (not fileDialog->SelectedFiles.empty())
                selectedFile = fileDialog->SelectedFiles.front();
        }
        Unused(selectedFile);
    });

    _OnApplicationTick.Emplace([memoryUsage(MakeUnique<Application::FMemoryUsageWidget>())](const IApplicationService&, FTimespan) {
        if (memoryUsage->Show()) {
            NOOP();
        }
    });

#if USE_PPE_LOGGER
    _OnApplicationTick.Emplace([logViewer(MakeUnique<Application::FLogViewerWidget>())](const IApplicationService&, FTimespan) {
        if (logViewer->Show()) {
            NOOP();
        }
    });
#endif

    _OnApplicationTick.Emplace([consoleWidget(MakeUnique<Application::FConsoleWidget>())](const IApplicationService&, FTimespan) {
        if (consoleWidget->Show()) {
            NOOP();
        }
    });

    _OnApplicationTick.Emplace([frameRateOverlay(MakeUnique<Application::FFrameRateOverlayWidget>(this))](const IApplicationService&, FTimespan) {
        if (frameRateOverlay->Show()) {
            NOOP();
        }
    });

    ApplicationLoop();
}
//----------------------------------------------------------------------------
void FShaderToyApp::Shutdown() {
    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
void FShaderToyApp::Update(FTimespan dt) {
    parent_type::Update(dt);

    Unused(dt);

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        static auto setupDockspaceOnce = true;
        if (setupDockspaceOnce) {
            setupDockspaceOnce = false;

            ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            // split the dockspace into 2 nodes -- DockBuilderSplitNode takes in the following args in the following order
            //   window ID to split, direction, fraction (between 0 and 1), the final two setting let's us choose which id we want (which ever one we DON'T set as NULL, will be returned by the function)
            //                                                              out_id_at_dir is the id of the node in the direction we specified earlier, out_id_at_opposite_dir is in the opposite direction
            /*auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
            auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dockspace_id);
            Unused(dock_id_down, dock_id_left);*/

            // we now dock our windows into the docking node we made above
            /*ImGui::DockBuilderDockWindow("Down", dock_id_down);
            ImGui::DockBuilderDockWindow("Left", dock_id_left);*/
            ImGui::DockBuilderFinish(dockspace_id);
        }
    }

    ImGui::End();

    //ImGui::Begin("Left");
    //ImGui::Text("Hello, left!");
    //ImGui::End();

    //ImGui::Begin("Down");
    //ImGui::Text("Hello, down !");
    //ImGui::End();

    /*ImGui::ShowDemoWindow();
    ImGui::ShowUserGuide();*/

    /*ImPlot::ShowDemoWindow();
    ImPlot::ShowUserGuide();*/
}
//----------------------------------------------------------------------------
void FShaderToyApp::Render(FTimespan dt) {
    parent_type::Render(dt);

    //_editor->Render("GLSL");


    /*using namespace RHI;
    const auto& fg = RHI().FrameGraph();
    RHI::FImageDesc desc{ EImageDim_3D, uint3(256), EPixelFormat::R8u, EImageUsage::Sampled, 0_layer, 1_mipmap };
    auto Voxels = fg->CreateImage(desc);*/
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
