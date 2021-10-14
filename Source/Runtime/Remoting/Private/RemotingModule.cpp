#include "stdafx.h"

#include "RemotingModule.h"

#include "RemotingServer.h"
#include "Remoting/RTTIEndpoint.h"

#include "ApplicationModule.h"
#include "Diagnostic/Logger.h"
#include "Modular/ModuleRegistration.h"
#include "RTTI/Module-impl.h"
#include "RTTI/Service.h"

#include "BuildModules.generated.h"
#include "RemotingService.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Remoting {
//----------------------------------------------------------------------------
LOG_CATEGORY(PPE_REMOTING_API, Remoting)
//----------------------------------------------------------------------------
RTTI_MODULE_DEF(PPE_REMOTING_API, Remoting, Remoting);
//----------------------------------------------------------------------------
} //!namespace Remoting
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FRemotingModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FRemotingModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::Application,
        EModuleUsage::Developer,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        Generated::DependencyList )
};
//----------------------------------------------------------------------------
FRemotingModule::FRemotingModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
FRemotingModule::~FRemotingModule() {
    // IRemotingService isn't defined in the header
}
//----------------------------------------------------------------------------
void FRemotingModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

    using namespace Remoting;

    RTTI_MODULE(Remoting).Start();

    auto& services = domain.Services();

    services.Get<IRTTIService>().RegisterModule(this, RTTI_MODULE(Remoting));

    IRemotingService::MakeDefault(&_remotingService, domain);
    services.Add<IRemotingService>(_remotingService.get());
}
//----------------------------------------------------------------------------
void FRemotingModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

    using namespace Remoting;

    auto& services = domain.Services();

    services.Remove<IRemotingService>();
    _remotingService.reset();

    services.Get<IRTTIService>().UnregisterModule(this, RTTI_MODULE(Remoting));

    RTTI_MODULE(Remoting).Shutdown();
}
//----------------------------------------------------------------------------
void FRemotingModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FRemotingModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

    using namespace Remoting;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
