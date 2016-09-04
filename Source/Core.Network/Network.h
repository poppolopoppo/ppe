#pragma once

#include "Core.Network/Network.h"

#ifdef EXPORT_CORE_NETWORK
#   define CORE_NETWORK_API DLL_EXPORT
#else
#   define CORE_NETWORK_API DLL_IMPORT
#endif

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_NETWORK_API NetworkStartup {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    NetworkStartup() { Start(); }
    ~NetworkStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
