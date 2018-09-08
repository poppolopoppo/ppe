#include "stdafx.h"

#include "Network.h"
#include "Network_fwd.h"

#include "NetworkName.h"
#include "Http/ConstNames.h"
#include "Socket/Socket.h"

#include "Allocator/PoolAllocatorTag-impl.h"
#include "Diagnostic/Logger.h"

PRAGMA_INITSEG_LIB

namespace PPE {
namespace Network {
POOL_TAG_DEF(Network);
LOG_CATEGORY(PPE_NETWORK_API, Network)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FNetworkModule::FNetworkModule()
:   FModule("Runtime/Network")
{}
//----------------------------------------------------------------------------
FNetworkModule::~FNetworkModule()
{}
//----------------------------------------------------------------------------
void FNetworkModule::Start(FModuleManager& manager) {
    FModule::Start(manager);

    POOL_TAG(Network)::Start();

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

    POOL_TAG(Network)::Shutdown();
}
//----------------------------------------------------------------------------
void FNetworkModule::ReleaseMemory() {
    FModule::ReleaseMemory();

    FlushDNSCache();

    POOL_TAG(Network)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
