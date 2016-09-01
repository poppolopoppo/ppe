#pragma once

#include "Core.Network/Network.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class NetworkStartup {
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
