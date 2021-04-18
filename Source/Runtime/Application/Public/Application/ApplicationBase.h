#pragma once

#include "Application_fwd.h"

#include "HAL/PlatformApplication.h"
#include "Misc/Event.h"
#include "Modular/Modular_fwd.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Dummy wrapper for fwd declaration (FPlatformApplication is a using)
class PPE_APPLICATION_API FApplicationBase : public FPlatformApplication {
public:
    explicit FApplicationBase(const FModularDomain& domain, FString&& name);
    virtual ~FApplicationBase() NOEXCEPT;

    virtual void Start() override;
    virtual void Tick(FTimespan dt) override;
    virtual void Shutdown() override;

    void ApplicationLoop();

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
