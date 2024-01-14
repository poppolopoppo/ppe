// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "ApplicationUIModule.h"

#include "Application/ApplicationBase.h"

#include "UI/ImguiService.h"

#include "Allocator/TrackingMalloc.h"
#include "Diagnostic/Logger.h"
#include "Modular/ModularDomain.h"
#include "Modular/ModuleRegistration.h"

#include "BuildModules.generated.h"
#include "Diagnostic/BuildVersion.h"

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

    PPE_LOG(UI, Info, "create imgui context");

    _ui.create();

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
void FApplicationUIModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FApplicationUIModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

}
//----------------------------------------------------------------------------
void FApplicationUIModule::OnApplicationStart_(Application::FApplicationBase& app, FModularServices& services) NOEXCEPT {
    Assert(_ui);

    using namespace Application;

    IInputService& input = services.Get<IInputService>();
    IRHIService& rhi = services.Get<IRHIService>();

    if (not _ui->Construct(app, input, rhi)) {
        PPE_LOG(UI, Error, "failed to initialize imgui service");
        return;
    }

    services.Add<IUIService>(_ui.get());
}
//----------------------------------------------------------------------------
void FApplicationUIModule::OnApplicationShutdown_(Application::FApplicationBase& app, FModularServices& services) NOEXCEPT {
    Assert(_ui);

    using namespace Application;

    if (_ui) {
        IInputService& input = services.Get<IInputService>();
        IRHIService& rhi = services.Get<IRHIService>();

        services.Remove<IUIService>();

        _ui->TearDown(app, input, rhi);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
