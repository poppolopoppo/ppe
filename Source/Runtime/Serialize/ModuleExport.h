#pragma once

#include "Serialize_fwd.h"

#include "Module.h"

#ifndef PPE_STATICMODULES_STARTUP
#   error "Runtime/Serialize/targetmodule.h can't be included first !"
#endif

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FSerializeModule is the entry and exit point encapsulating every call to PPE::Serialize::.
// Constructed with the same lifetime than the program (or application if segregated).
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FSerializeModule : public FModule {
public:
    FSerializeModule();
    virtual ~FSerializeModule();

protected:
    virtual void Start() override final;
    virtual void Shutdown() override final;
    virtual void ReleaseMemory() override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE

using FSerializeModule = PPE::Serialize::FSerializeModule;
PPE_STATICMODULE_STARTUP_DEF(Serialize);

#undef PPE_STATICMODULES_STARTUP
#define PPE_STATICMODULES_STARTUP PPE_STATICMODULE_STARTUP_NAME(Serialize)
