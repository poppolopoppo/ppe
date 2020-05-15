#pragma once

#include "Serialize_fwd.h"

#include "Modular/ModuleInterface.h"

#include "Diagnostic/Logger_fwd.h"

namespace PPE {
namespace Serialize {
EXTERN_LOG_CATEGORY(PPE_SERIALIZE_API, Serialize)
} //!namespace Serialize
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FSerializeModule final : public IModuleInterface {
public:
    static const FModuleInfo StaticInfo;

    explicit FSerializeModule() NOEXCEPT;

    virtual void Start(FModularDomain& domain) override;
    virtual void Shutdown(FModularDomain& domain) override;

    virtual void DutyCycle(FModularDomain& domain) override;
    virtual void ReleaseMemory(FModularDomain& domain) NOEXCEPT override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
