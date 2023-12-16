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

    virtual void RequestExit() NOEXCEPT override;

    void ApplicationLoop();

private:
    FTimeline _timeline;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
