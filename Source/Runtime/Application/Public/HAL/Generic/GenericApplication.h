#pragma once

#include "Application_fwd.h"

#include "Application/ApplicationService.h"

#include "IO/String.h"
#include "Modular/ModularDomain.h"
#include "Modular/ModularServices.h"
#include "Time/Timeline.h"
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
    virtual void Run();
    virtual void Shutdown();

public: // application service
    virtual const FModularServices& Services() const NOEXCEPT final override { return _services; }

    virtual const FModularDomain& Domain() const NOEXCEPT override final { return _domain; }
    virtual const FString& Name() const NOEXCEPT override final { return _name; }
    virtual bool HasFocus() const NOEXCEPT override final { return _hasFocus; }

    virtual const FTimeline& RealTime() const NOEXCEPT override final { return _realTime; }
    virtual const FTimeline& ApplicationTime() const NOEXCEPT override final { return _applicationTime; }

    virtual void SetApplicationTimeDilation(float speed) NOEXCEPT override final;
    virtual float ApplicationTimeDilation() const NOEXCEPT  override final { return _applicationTimeDilation; }

    virtual void SetLowerTickRateInBackground(bool enabled) NOEXCEPT override final;
    virtual bool LowerTickRateInBackground() const NOEXCEPT override final { return _lowerTickRateInBackground; }

    virtual void SetTickRate(FTimespan period) NOEXCEPT override final;
    virtual FTimespan TickRate() const NOEXCEPT override final { return _tickRate; }

    virtual void RequestExit() NOEXCEPT override;
    virtual bool HasRequestedExit() const NOEXCEPT override final { return _requestedExit; }

    virtual bool PumpMessages() NOEXCEPT override;
    virtual void ReleaseMemory() NOEXCEPT override;

protected:
    explicit FGenericApplication(FModularDomain& domain, FString&& name);

    FModularDomain& Domain_() { return _domain; }
    FModularServices& Services_() { return _services; }

    void SetFocus(bool value) { _hasFocus = value; }

    virtual void Tick(FTimespan dt);

private:
    FModularDomain& _domain;
    FString _name;
    FModularServices _services;

    FTimeline _realTime;
    FTimeline _applicationTime;
    FTimespan _tickRate;

    float _applicationTimeDilation{ 1.0 };

    bool _hasFocus : 1;
    bool _requestedExit : 1;
    bool _lowerTickRateInBackground : 1;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
