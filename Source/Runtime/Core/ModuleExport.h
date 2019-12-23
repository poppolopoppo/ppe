#pragma once

#include "Core_fwd.h"

#include "Module.h"

#ifndef PPE_STATICMODULES_STARTUP
#   error "Runtime/Core/ModuleExport.h can't be included first !"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FCoreModule is the entry and exit point encapsulating every call to PPE::.
// Constructed with the same lifetime than the program (or application if segregated).
//----------------------------------------------------------------------------
class PPE_CORE_API FCoreModule : public FModule {
public:
    FCoreModule();
    virtual ~FCoreModule();

protected:
    virtual void Start() override final;
    virtual void Shutdown() override final;

    virtual void ReleaseMemory() override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

using FCoreModule = PPE::FCoreModule;
PPE_STATICMODULE_STARTUP_DEF(Core);

#undef PPE_STATICMODULES_STARTUP
#define PPE_STATICMODULES_STARTUP PPE_STATICMODULE_STARTUP_NAME(Core)
