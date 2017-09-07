#include "stdafx.h"

#include "Network.h"
#include "Network_fwd.h"

#include "Name.h"
#include "Http/ConstNames.h"
#include "Socket/Socket.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

PRAGMA_INITSEG_LIB

#ifdef PLATFORM_WINDOWS
//  Link with WinSock2 library
#   pragma comment(lib, "ws2_32.lib")
#endif

namespace Core {
namespace Network {
POOL_TAG_DEF(Network);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FNetworkModule::Start() {
    CORE_MODULE_START(Network);

    POOL_TAG(Network)::Start();

    FName::Start();
    FHttpConstNames::Start();
    FSocket::Start();
}
//----------------------------------------------------------------------------
void FNetworkModule::Shutdown() {
    CORE_MODULE_SHUTDOWN(Network);

    FSocket::Shutdown();
    FHttpConstNames::Shutdown();
    FName::Shutdown();

    POOL_TAG(Network)::Shutdown();
}
//----------------------------------------------------------------------------
void FNetworkModule::ClearAll_UnusedMemory() {
    CORE_MODULE_CLEARALL(Network);

    POOL_TAG(Network)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
