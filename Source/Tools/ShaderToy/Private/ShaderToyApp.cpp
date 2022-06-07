#include "stdafx.h"

#include "ShaderToyApp.h"

#include "ApplicationModule.h"
#include "RHIModule.h"
#include "UI/Imgui.h"
#include "Window/WindowService.h"

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

    ImGui::ShowDemoWindow();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
