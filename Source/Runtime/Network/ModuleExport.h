#pragma once

#include "Network_fwd.h"

#include "Module.h"

#ifndef PPE_STATICMODULES_STARTUP
#   error "Runtime/Network/ModuleExport.h can't be included first !"
#endif

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_NETWORK_API FNetworkModule : public FModule {
public:
    FNetworkModule();
    virtual ~FNetworkModule();

protected:
    virtual void Start(FModuleManager& manager) override final;
    virtual void Shutdown() override final;
    virtual void ReleaseMemory() override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE

using FNetworkModule = PPE::Network::FNetworkModule;
PPE_STATICMODULE_STARTUP_DEF(Network);

#undef PPE_STATICMODULES_STARTUP
#define PPE_STATICMODULES_STARTUP PPE_STATICMODULE_STARTUP_NAME(Network)
