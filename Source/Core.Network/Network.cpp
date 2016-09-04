#include "stdafx.h"

#include "Network.h"
#include "Network_fwd.h"

#include "Socket.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

#ifdef OS_WINDOWS
#   pragma warning(disable: 4073) // initialiseurs placés dans la zone d'initialisation d'une bibliothèque
#   pragma init_seg(lib)
#else
#   error "missing compiler specific command"
#endif

#ifdef OS_WINDOWS
//  Link with WinSock2 library
#   pragma comment(lib, "ws2_32.lib")
#endif

namespace Core {
namespace Network {
POOL_TAG_DEF(Network);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void NetworkStartup::Start() {
    CORE_MODULE_START(Network);

    POOL_TAG(Network)::Start();

    Socket::Start();
}
//----------------------------------------------------------------------------
void NetworkStartup::Shutdown() {
    CORE_MODULE_SHUTDOWN(Network);

    Socket::Shutdown();

    POOL_TAG(Network)::Shutdown();
}
//----------------------------------------------------------------------------
void NetworkStartup::ClearAll_UnusedMemory() {
    CORE_MODULE_CLEARALL(Network);

    POOL_TAG(Network)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
