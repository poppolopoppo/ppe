#pragma once

#include "Remoting_fwd.h"

#include "Modular/ModuleInterface.h"

#include "Diagnostic/Logger_fwd.h"
#include "Memory/UniquePtr.h"
#include "Misc/EventHandle.h"
#include "RTTI/Module.h"

namespace PPE {
namespace Remoting {
EXTERN_LOG_CATEGORY(PPE_REMOTING_API, Remoting)
RTTI_MODULE_DECL(PPE_REMOTING_API, Remoting);
} //!namespace Remoting
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRemotingModule final : public IModuleInterface {
public:
    static const FModuleInfo StaticInfo;

    explicit FRemotingModule() NOEXCEPT;
    ~FRemotingModule() override;

    const IRemotingService& Service() const { return *_remotingService; }

    virtual void Start(FModularDomain& domain) override;
    virtual void Shutdown(FModularDomain& domain) override;

    virtual void DutyCycle(FModularDomain& domain) override;
    virtual void ReleaseMemory(FModularDomain& domain) NOEXCEPT override;

private:
    URemotingService _remotingService;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
