#pragma once

#include "RTTI_fwd.h"

#include "Container/AssociativeVector.h"
#include "Memory/PtrRef.h"
#include "Memory/UniquePtr.h"
#include "Modular/Modular_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RTTI_API IRTTIService {
protected:
    IRTTIService() = default;

public: // virtual
    virtual ~IRTTIService() = default;

    virtual TPtrRef<const RTTI::FMetaModule> Module(TPtrRef<const IModuleInterface> modular) const NOEXCEPT = 0;

    virtual void RegisterModule(TPtrRef<const IModuleInterface> modular, TPtrRef<const RTTI::FMetaModule> rtti) = 0;
    virtual void UnregisterModule(TPtrRef<const IModuleInterface> modular, TPtrRef<const RTTI::FMetaModule> rtti) = 0;

public:
    static void MakeDefault(TUniquePtr<IRTTIService>* service);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
