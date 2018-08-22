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

    static FGenericApplication& Get() = delete; // must implement a singleton !

    virtual ~FGenericApplication();

    const FWString& Name() const { return _name; }
    const FServiceContainer& Services() const { return _services; }

    virtual void Start();
    virtual void PumpMessages();
    virtual void Tick(FTimespan dt);
    virtual void Shutdown();

protected:
    explicit FGenericApplication(FWString&& name);

    FServiceContainer& Services() { return _services; }

private:
    FWString _name;
    FServiceContainer _services;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
