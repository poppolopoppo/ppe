#pragma once

#include "Application_fwd.h"

#include "Modular/Modular_fwd.h"
#include "Time/Time_fwd.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API IApplicationService {
public:
    virtual ~IApplicationService() = default;

    virtual const FModularServices& Services() const NOEXCEPT = 0;

    virtual const FModularDomain& Domain() const NOEXCEPT = 0;
    virtual const FString& Name() const NOEXCEPT = 0;
    virtual FTimespan DeltaTime() const NOEXCEPT = 0;
    virtual FTimespan ElapsedTime() const NOEXCEPT = 0;
    virtual bool HasFocus() const NOEXCEPT = 0;

    virtual void SetLowerTickRateInBackground(bool enabled) NOEXCEPT = 0;
    virtual bool LowerTickRateInBackground() const NOEXCEPT = 0;

    virtual void SetTickRate(FTimespan period) NOEXCEPT = 0;
    virtual FTimespan TickRate() const NOEXCEPT = 0;

    virtual void RequestExit() NOEXCEPT = 0;
    virtual bool HasRequestedExit() const NOEXCEPT = 0;

    virtual bool PumpMessages() NOEXCEPT = 0;
    virtual void ReleaseMemory() = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
