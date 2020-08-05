#pragma once

#include "Application_fwd.h"

#include "IO/String.h"
#include "Modular/ModularDomain.h"
#include "Modular/ModularServices.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FGenericApplication {
public: // must be defined for every platform

    virtual ~FGenericApplication() NOEXCEPT;

    virtual void Start();
    virtual bool PumpMessages() NOEXCEPT;
    virtual void Tick(FTimespan dt);
    virtual void Shutdown();

public: // generic
    const FModularDomain& Domain() const { return _domain; }
    const FWString& Name() const { return _name; }
    const FModularServices& Services() const { return _services; }
    const FSeconds& Elapsed() const { return _elapsed; }

protected:
    explicit FGenericApplication(const FModularDomain& domain, FWString&& name);

    FModularServices& Services() { return _services; }

private:
    const FModularDomain& _domain;
    FWString _name;
    FModularServices _services;
    FSeconds _elapsed;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
