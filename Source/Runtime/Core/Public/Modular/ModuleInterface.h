#pragma once

#include "Core_fwd.h"

#include "Modular/Modular_fwd.h"

#include "IO/StringView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API IModuleInterface : public Meta::FNonCopyableNorMovable {
public:
    explicit IModuleInterface(const FModuleInfo& info) NOEXCEPT;

    virtual ~IModuleInterface();

    const FStringView& Name() const { return _name; }
    EModulePhase Phase() const { return _phase; }

    virtual void Start(FModularDomain& domain);
    virtual void Shutdown(FModularDomain& domain);

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