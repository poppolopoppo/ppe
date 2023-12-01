#pragma once

#include "Application_fwd.h"

#include "Application/ApplicationService.h"

#include "IO/String.h"
#include "Modular/ModularDomain.h"
#include "Modular/ModularServices.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FGenericApplication : public IApplicationService {
public: // must be defined for every platform

    virtual ~FGenericApplication() override;

    virtual void Start();
    virtual void Shutdown();

    virtual const FModularServices& Services() const NOEXCEPT final override { return _services; }

    virtual bool PumpMessages() NOEXCEPT override;
    virtual void ReleaseMemory() NOEXCEPT override;
    virtual void Tick(FTimespan dt);

public: // generic
    const FModularDomain& Domain() const { return _domain; }
    const FString& Name() const { return _name; }
    const FSeconds& Elapsed() const { return _elapsed; }
    bool HasFocus() const { return _hasFocus; }

protected:
    explicit FGenericApplication(FModularDomain& domain, FString&& name);

    FModularServices& Services_() { return _services; }

    void SetFocus(bool value) { _hasFocus = value; }

private:
    FModularDomain& _domain;
    FString _name;
    FModularServices _services;
    FSeconds _elapsed;
    bool _hasFocus{ true };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
