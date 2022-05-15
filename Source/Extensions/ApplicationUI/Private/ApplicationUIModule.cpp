#include "stdafx.h"

#include "ApplicationUIModule.h"

#include "UI/ImguiService.h"

#include "Diagnostic/Logger.h"
#include "Modular/ModularDomain.h"
#include "Modular/ModuleRegistration.h"

#include "BuildModules.generated.h"
#include "Diagnostic/BuildVersion.h"

#include "imgui-external.h"
#include "External/imgui/imgui.git/imgui_internal.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Application {
//----------------------------------------------------------------------------
LOG_CATEGORY(PPE_APPLICATIONUI_API, UI)
//----------------------------------------------------------------------------
} //!namespace Application
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FApplicationUIModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FApplicationUIModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::System,
        EModuleUsage::Runtime,
        EModuleSource::Extensions,
        BUILD_TARGET_ORDINAL,
        Generated::DependencyList,
        CurrentBuildVersion() )
};
//----------------------------------------------------------------------------
FApplicationUIModule& FApplicationUIModule::Get(const FModularDomain& domain) {
    return domain.ModuleChecked<FApplicationUIModule>(STRINGIZE(BUILD_TARGET_NAME));
}
//----------------------------------------------------------------------------
FApplicationUIModule::FApplicationUIModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
void FApplicationUIModule::Start(FModularDomain& domain) {
    Assert(not _ui);
    IModuleInterface::Start(domain);

    using namespace Application;

    LOG(UI, Info, L"create imgui context");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    _ui.create(PImguiContext{ GImGui });

    auto& appModule = FApplicationModule::Get(domain);
    _onApplicationStart = appModule.OnApplicationStart().Bind<&FApplicationUIModule::OnApplicationStart_>(this);
    _onApplicationShutdown = appModule.OnApplicationShutdown().Bind<&FApplicationUIModule::OnApplicationShutdown_>(this);
}
//----------------------------------------------------------------------------
void FApplicationUIModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

    auto& appModule = FApplicationModule::Get(domain);
    appModule.OnApplicationStart().Remove(_onApplicationStart);
    appModule.OnApplicationShutdown().Remove(_onApplicationShutdown);

    _ui.reset();
}
//----------------------------------------------------------------------------
void FApplicationUIModule::OnApplicationStart_(Application::FApplicationBase&, FModularServices& services) NOEXCEPT {
    Assert(_ui);

    using namespace Application;

    IInputService& input = services.Get<IInputService>();
    IRHIService& rhi = services.Get<IRHIService>();

    if (not _ui->Construct(input, rhi)) {
        LOG(UI, Error, L"failed to initialize imgui service");
        _ui.reset();
        return;
    }

    services.Add<IUIService>(_ui.get());
}
//----------------------------------------------------------------------------
void FApplicationUIModule::OnApplicationShutdown_(Application::FApplicationBase&, FModularServices& services) NOEXCEPT {
    Assert(_ui);

    using namespace Application;

    if (_ui) {
        IInputService& input = services.Get<IInputService>();
        IRHIService& rhi = services.Get<IRHIService>();

        services.Remove<IUIService>();

        _ui->TearDown(input, rhi);
    }
}
//----------------------------------------------------------------------------
void FApplicationUIModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FApplicationUIModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
