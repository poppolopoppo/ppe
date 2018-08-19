#pragma once

#include "Application_fwd.h"

#include "IO/String.h"
#include "Misc/ServiceContainer.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FGenericApplication {
public: // must be defined for every platform
    virtual ~FGenericApplication();

    const FString& Name() const { return _name; }
    const FServiceContainer& Services() const { return _services; }

    virtual void Start() {}
    virtual void PumpMessages() {}
    virtual void ProcessDeferredEvents(FTimespan dt) {}
    virtual void Tick(FTimespan dt) {}
    virtual void Shutdown() {}

protected:
    FGenericApplication() {} // this is a base virtual class

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
