// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "ShaderToyApp.h"

#include "ApplicationModule.h"

#include "RHIModule.h"
#include "Color/Color.h"
#include "UI/Imgui.h"
#include "UI/Widgets/ConsoleWidget.h"
#include "UI/Widgets/FileDialogWidget.h"
#include "UI/Widgets/LogViewerWidget.h"
#include "UI/Widgets/MemoryUsageWidget.h"
#include "Window/WindowService.h"

#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "Maths/Threefy.h"

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

    ApplicationLoop();
}
//----------------------------------------------------------------------------
void FShaderToyApp::Shutdown() {
    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
void FShaderToyApp::Render(FTimespan dt) {
    parent_type::Render(dt);

    //_editor->Render("GLSL");

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
			auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
			auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dockspace_id);

			// we now dock our windows into the docking node we made above
			ImGui::DockBuilderDockWindow("Down", dock_id_down);
			ImGui::DockBuilderDockWindow("Left", dock_id_left);
			ImGui::DockBuilderFinish(dockspace_id);
		}
	}

	ImGui::End();

	ImGui::Begin("Left");
	ImGui::Text("Hello, left!");




	ImGui::End();

	ImGui::Begin("Down");
	ImGui::Text("Hello, down !");
	ImGui::End();

	ImGui::ShowDemoWindow();
	ImGui::ShowUserGuide();

	ImPlot::ShowDemoWindow();
	ImPlot::ShowUserGuide();

	FFilename selectedFile;
	static Application::FFileDialogWidget fileDialog;
	if (fileDialog.Show()) {
		if (not fileDialog.SelectedFiles.empty())
			selectedFile = fileDialog.SelectedFiles.front();
	}

	static Application::FMemoryUsageWidget memoryUsage;
	if (memoryUsage.Show()) {
		NOOP();
	}

#if USE_PPE_LOGGER
	static Application::FLogViewerWidget  logViewer;
	if (logViewer.Show()) {
		NOOP();
	}
#endif

	static Application::FConsoleWidget console;
	if (console.Show()) {
		NOOP();
	}

	static bool showFrameRateOverlay = true;
	auto frameRateOverlay = [](const FApplicationWindow& app, bool* p_open){
		static int location = 1;
	    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	    if (location >= 0)
	    {
	        const float PAD = 10.0f;
	        const ImGuiViewport* viewport = ImGui::GetMainViewport();
	        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
	        ImVec2 work_size = viewport->WorkSize;
	        ImVec2 window_pos, window_pos_pivot;
	        window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
	        window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
	        window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
	        window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
	        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	        ImGui::SetNextWindowViewport(viewport->ID);
	        window_flags |= ImGuiWindowFlags_NoMove;
	    }
	    else if (location == -2)
	    {
	        // Center window
	        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	        window_flags |= ImGuiWindowFlags_NoMove;
	    }
	    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
	    if (ImGui::Begin("FPS overlay", p_open, window_flags))
	    {
			FSeconds dt = app.DeltaTime();

			ImGui::TextColored(float4(FLinearColor::Yellow()), "FPS: %.2f", 1.0 / dt.Value());
	        ImGui::TextColored(float4(FLinearColor::LightSalmon()), "DT: %.3fms", FMilliseconds(dt).Value());
			ImGui::TextColored(float4(FLinearColor::LightCyan()), "Pump: %.3fms", FMilliseconds(app.MessageTime().Average).Value());
			ImGui::TextColored(float4(FLinearColor::LightYellow()), "Tick: %.3fms", FMilliseconds(app.TickTime().Average).Value());

	        if (ImGui::BeginPopupContextWindow())
	        {
	            if (ImGui::MenuItem("Custom",       NULL, location == -1)) location = -1;
	            if (ImGui::MenuItem("Center",       NULL, location == -2)) location = -2;
	            if (ImGui::MenuItem("Top-left",     NULL, location == 0)) location = 0;
	            if (ImGui::MenuItem("Top-right",    NULL, location == 1)) location = 1;
	            if (ImGui::MenuItem("Bottom-left",  NULL, location == 2)) location = 2;
	            if (ImGui::MenuItem("Bottom-right", NULL, location == 3)) location = 3;
	            if (p_open && ImGui::MenuItem("Close")) *p_open = false;
	            ImGui::EndPopup();
	        }
	    }
	    ImGui::End();
	};
	frameRateOverlay(*this, &showFrameRateOverlay);

	/*using namespace RHI;
	const auto& fg = RHI().FrameGraph();
	RHI::FImageDesc desc{ EImageDim_3D, uint3(256), EPixelFormat::R8u, EImageUsage::Sampled, 0_layer, 1_mipmap };
	auto Voxels = fg->CreateImage(desc);*/
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
