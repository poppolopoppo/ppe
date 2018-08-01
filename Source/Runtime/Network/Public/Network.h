#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_NETWORK
#   define PPE_NETWORK_API DLL_EXPORT
#else
#   define PPE_NETWORK_API DLL_IMPORT
#endif

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_NETWORK_API FNetworkModule {
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
} //!namespace PPE
