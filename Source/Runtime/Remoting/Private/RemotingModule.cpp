#include "stdafx.h"

#include "RemotingModule.h"

#include "RemotingServer.h"
#include "Remoting/RTTIEndpoint.h"

#include "ApplicationModule.h"
#include "Diagnostic/Logger.h"
#include "Modular/ModuleRegistration.h"

#include "BuildModules.generated.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Remoting {
//----------------------------------------------------------------------------
LOG_CATEGORY(PPE_REMOTING_API, Remoting)
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
void FRemotingModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

    using namespace Remoting;

    _srv.reset<FRemotingServer>();
    _srv->Add(MakeUnique<FRTTIEndpoint>());
    _srv->Start();

    _tickHandle = FApplicationModule::Get(domain).OnApplicationTick().Add([this](Application::FApplicationBase&, FTimespan dt) {
        _srv->Tick(dt);
    });
}
//----------------------------------------------------------------------------
void FRemotingModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

    using namespace Remoting;

    FApplicationModule::Get(domain).OnApplicationTick().Remove(_tickHandle);

    _srv->Shutdown();
    _srv.reset();
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
