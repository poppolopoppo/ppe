#include "stdafx.h"

#include "ModuleExport.h"

#include "Network.h"

#include "NetworkName.h"
#include "Http/ConstNames.h"
#include "Socket/Socket.h"

#include "Diagnostic/Logger.h"

#include "Module-impl.h"

namespace PPE {
namespace Network {
LOG_CATEGORY(PPE_NETWORK_API, Network)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FNetworkModule::FNetworkModule()
:   FModule("Runtime/Network")
{}
//----------------------------------------------------------------------------
FNetworkModule::~FNetworkModule() = default;
//----------------------------------------------------------------------------
void FNetworkModule::Start() {
    FModule::Start();

    FName::Start();
    FHttpConstNames::Start();
    FAddress::Start();
    FSocket::Start();
}
//----------------------------------------------------------------------------
void FNetworkModule::Shutdown() {
    FModule::Shutdown();

    FlushDNSCache();

    FSocket::Shutdown();
    FAddress::Shutdown();
    FHttpConstNames::Shutdown();
    FName::Shutdown();
}
//----------------------------------------------------------------------------
void FNetworkModule::ReleaseMemory() {
    FModule::ReleaseMemory();

    FlushDNSCache();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
