#pragma once

#include "Application_fwd.h"

#include "Misc/Event.h"
#include "Modular/Modular_fwd.h"
#include "Time/Time_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API IApplicationService {
public:
    virtual ~IApplicationService() = default;

    virtual const FModularServices& Services() const NOEXCEPT = 0;

    virtual const FModularDomain& Domain() const NOEXCEPT = 0;
    virtual const FString& Name() const NOEXCEPT = 0;
    virtual bool HasFocus() const NOEXCEPT = 0;

    virtual const FTimeline& RealTime() const NOEXCEPT = 0;
    virtual const FTimeline& ApplicationTime() const NOEXCEPT = 0;

    virtual float ApplicationTimeDilation() const NOEXCEPT = 0;
    virtual void SetApplicationTimeDilation(float speed) NOEXCEPT = 0;

    virtual void SetLowerTickRateInBackground(bool enabled) NOEXCEPT = 0;
    virtual bool LowerTickRateInBackground() const NOEXCEPT = 0;

    virtual void SetTickRate(FTimespan period) NOEXCEPT = 0;
    virtual FTimespan TickRate() const NOEXCEPT = 0;

    virtual void RequestExit() NOEXCEPT = 0;
    virtual bool HasRequestedExit() const NOEXCEPT = 0;

    virtual bool PumpMessages() NOEXCEPT = 0;
    virtual void ReleaseMemory() = 0;

public: // for all services
    using FApplicationEvent = TFunction<void(const IApplicationService&)>;

    THREADSAFE_EVENT(OnApplicationBeginLoop, FApplicationEvent);
    THREADSAFE_EVENT(OnApplicationEndLoop, FApplicationEvent);

    THREADSAFE_EVENT(OnApplicationBeginTick, FApplicationEvent);
    THREADSAFE_EVENT(OnApplicationEndTick, FApplicationEvent);

    using FApplicationTickEvent = TFunction<bool(const IApplicationService&, FTimespan dt)>;

    THREADSAFE_EVENT(OnApplicationTick, FApplicationTickEvent);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
