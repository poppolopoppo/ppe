#pragma once

#include "Application_fwd.h"

#include "Module.h"

#ifndef PPE_STATICMODULES_STARTUP
#   error "Runtime/Application/ModuleExport.h can't be included first !"
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FApplicationModule : public FModule {
public:
    FApplicationModule();
    virtual ~FApplicationModule();

protected:
    virtual void Start(FModuleManager& manager) override final;
    virtual void Shutdown() override final;
    virtual void ReleaseMemory() override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

using FApplicationModule = PPE::Application::FApplicationModule;
PPE_STATICMODULE_STARTUP_DEF(Application);

#undef PPE_STATICMODULES_STARTUP
#define PPE_STATICMODULES_STARTUP PPE_STATICMODULE_STARTUP_NAME(Application)
