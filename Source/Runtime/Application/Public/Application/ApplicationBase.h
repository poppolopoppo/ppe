#pragma once

#include "Application_fwd.h"

#include "HAL/PlatformApplication.h"
#include "Misc/Event.h"
#include "Modular/Modular_fwd.h"
#include "Time/Timeline.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Dummy wrapper for fwd declaration (FPlatformApplication is a using)
class PPE_APPLICATION_API FApplicationBase : public FPlatformApplication {
public:
    explicit FApplicationBase(FModularDomain& domain, FString&& name);
    virtual ~FApplicationBase() NOEXCEPT override;

    virtual void Start() override;
    virtual void Tick(FTimespan dt) override;
    virtual void Shutdown() override;

    const FTimeline& Timeline() const { return _timeline; }

    void SetTickRate(FTimespan period);
    FTimespan TickRate() const { return _tickRate; }

    void SetLowerTickRateInBackground(bool enabled);
    bool LowerTickRateInBackground() const { return _lowerTickRateInBackground; }

    void RequestExit();
    bool HasRequestedExit() const { return _requestedExit; }

    void ApplicationLoop();

private:
    FTimeline _timeline;
    FTimespan _tickRate;

    bool _requestedExit : 1;
    bool _lowerTickRateInBackground : 1;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
