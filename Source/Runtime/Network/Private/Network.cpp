#include "stdafx.h"

#include "Network.h"
#include "Network_fwd.h"

#include "Name.h"
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
void FNetworkModule::Start() {
    PPE_MODULE_START(Network);

    POOL_TAG(Network)::Start();

    FName::Start();
    FHttpConstNames::Start();
    FAddress::Start();
    FSocket::Start();
}
//----------------------------------------------------------------------------
void FNetworkModule::Shutdown() {
    PPE_MODULE_SHUTDOWN(Network);

    FlushDNSCache();

    FSocket::Shutdown();
    FAddress::Shutdown();
    FHttpConstNames::Shutdown();
    FName::Shutdown();

    POOL_TAG(Network)::Shutdown();
}
//----------------------------------------------------------------------------
void FNetworkModule::ClearAll_UnusedMemory() {
    PPE_MODULE_CLEARALL(Network);

    FlushDNSCache();
    POOL_TAG(Network)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
