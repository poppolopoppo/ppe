#pragma once

#include "Core_fwd.h"

#include "Modular/Modular_fwd.h"

#include "IO/StringView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API IModuleInterface {
public:
    explicit IModuleInterface(const FModuleInfo& info) NOEXCEPT;
    virtual ~IModuleInterface() NOEXCEPT;

    IModuleInterface(const IModuleInterface& ) = delete;
    IModuleInterface& operator =(const IModuleInterface& ) = delete;

    const FStringView& Name() const { return _name; }
    EModulePhase Phase() const { return _phase; }

    virtual void Start(FModularDomain& domain);
    virtual void Shutdown(FModularDomain& domain);

    virtual void PostStart(FModularDomain& domain);
    virtual void PreShutdown(FModularDomain& domain);

    virtual void DutyCycle(FModularDomain& domain);
    virtual void ReleaseMemory(FModularDomain& domain) NOEXCEPT;

private:
    const FStringView _name;
    const EModulePhase _phase;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
