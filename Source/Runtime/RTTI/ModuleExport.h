#pragma once

#include "RTTI_fwd.h"

#include "Module.h"

#ifndef PPE_STATICMODULES_STARTUP
#   error "Runtime/RTTI/targetmodule.h can't be included first !"
#endif

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RTTI_API FRTTIModule : public FModule {
public:
    FRTTIModule();
    virtual ~FRTTIModule();

protected:
    virtual void Start(FModuleManager& manager) override final;
    virtual void Shutdown() override final;
    virtual void ReleaseMemory() override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

using FRTTIModule = PPE::RTTI::FRTTIModule;
PPE_STATICMODULE_STARTUP_DEF(RTTI);

#undef PPE_STATICMODULES_STARTUP
#define PPE_STATICMODULES_STARTUP PPE_STATICMODULE_STARTUP_NAME(RTTI)
