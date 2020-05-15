#include "stdafx.h"

#include "NetworkModule.h"

#include "Network.h"

#include "Diagnostic/Logger.h"
#include "NetworkName.h"
#include "Http/ConstNames.h"
#include "Socket/Socket.h"

#include "Modular/ModuleRegistration.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Network {
//----------------------------------------------------------------------------
LOG_CATEGORY(PPE_NETWORK_API, Network)
//----------------------------------------------------------------------------
} //!namespace Network
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FNetworkModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FNetworkModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::Bare,
        EModuleUsage::Runtime,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        STRINGIZE(BUILD_TARGET_DEPS) )
};
//----------------------------------------------------------------------------
FNetworkModule::FNetworkModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
void FNetworkModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

    using namespace Network;

    FName::Start();
    FHttpConstNames::Start();
    FAddress::Start();
    FSocket::Start();
}
//----------------------------------------------------------------------------
void FNetworkModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

    using namespace Network;

    FlushDNSCache();

    FSocket::Shutdown();
    FAddress::Shutdown();
    FHttpConstNames::Shutdown();
    FName::Shutdown();
}
//----------------------------------------------------------------------------
void FNetworkModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FNetworkModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

    using namespace Network;

    FlushDNSCache();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
