#pragma once

#include "Core/Core.h"

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
class CORE_NETWORK_API FNetworkModule {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    FNetworkModule() { Start(); }
    ~FNetworkModule() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
