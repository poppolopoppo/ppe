#pragma once

#include "Remoting_fwd.h"

#include "Modular/ModuleInterface.h"

#include "Diagnostic/Logger_fwd.h"
#include "Memory/UniquePtr.h"
#include "Misc/EventHandle.h"

namespace PPE {
namespace Remoting {
EXTERN_LOG_CATEGORY(PPE_REMOTING_API, Remoting)
} //!namespace Remoting
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRemotingModule final : public IModuleInterface {
public:
    static const FModuleInfo StaticInfo;

    explicit FRemotingModule() NOEXCEPT;

    virtual void Start(FModularDomain& domain) override;
    virtual void Shutdown(FModularDomain& domain) override;

    virtual void DutyCycle(FModularDomain& domain) override;
    virtual void ReleaseMemory(FModularDomain& domain) NOEXCEPT override;

private:
    FEventHandle _tickHandle;
    TUniquePtr<Remoting::FRemotingServer> _srv;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
