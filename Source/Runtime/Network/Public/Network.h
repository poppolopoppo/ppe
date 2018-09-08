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
