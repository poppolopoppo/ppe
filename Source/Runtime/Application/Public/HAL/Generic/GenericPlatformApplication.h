#pragma once

#include "Application.h"

#include "HAL/TargetPlatform.h"

#include "IO/String.h"
#include "Misc/ServiceContainer.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Application {
class IDisplayAdapterService;
class IInputService;
class INotificationService;
class IWindowService;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FGenericPlatformApplication {
public: // must be defined for every platform
    virtual ~FGenericPlatformApplication();

    const FString& Name() const { return _name; }
    const FServiceContainer& Services() const { return _services; }

    const IDisplayAdapterService& DisplayAdapter() const;
    const IInputService& Input() const;
    const INotificationService& Notification() const;
    const IWindowService& Window() const;

    virtual void Start() {}
    virtual void PumpMessages() {}
    virtual void ProcessDeferredEvents(FTimespan dt) {}
    virtual void Tick(FTimespan dt) {}
    virtual void Shutdown() {}

protected:
    FGenericPlatformApplication() {} // this is a base virtual class

    FServiceContainer& Services() { return _services; }

private:
    FString _name;
    FServiceContainer _services;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
