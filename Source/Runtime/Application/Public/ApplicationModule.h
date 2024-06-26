#pragma once

#include "Application_fwd.h"

#include "Modular/ModuleInterface.h"

#include "Diagnostic/Logger_fwd.h"
#include "Misc/Event.h"
#include "Misc/Function.h"
#include "Modular/ModularServices.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATION_API, Application)
} //!namespace Application
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FApplicationModule final : public IModuleInterface {
public:
    static const FModuleInfo StaticInfo;

    explicit FApplicationModule() NOEXCEPT;

    virtual void Start(FModularDomain& domain) override;
    virtual void Shutdown(FModularDomain& domain) override;

    virtual void DutyCycle(FModularDomain& domain) override;
    virtual void ReleaseMemory(FModularDomain& domain) NOEXCEPT override;

public:
    friend class Application::FApplicationBase;

    static FApplicationModule& Get(const FModularDomain& domain);

    using FApplicationEvent = TFunction<void(Application::FApplicationBase&) NOEXCEPT>;

    PUBLIC_EVENT(OnApplicationCreate, FApplicationEvent);
    PUBLIC_EVENT(OnApplicationDestroy, FApplicationEvent);

    using FApplicationServicesEvent = TFunction<void(Application::FApplicationBase&, FModularServices&) NOEXCEPT>;

    PUBLIC_EVENT(OnApplicationStart, FApplicationServicesEvent);
    PUBLIC_EVENT(OnApplicationRun, FApplicationServicesEvent);
    PUBLIC_EVENT(OnApplicationShutdown, FApplicationServicesEvent);

    using FApplicationTick = TFunction<void(Application::FApplicationBase&, FTimespan) NOEXCEPT>;

    PUBLIC_EVENT(OnApplicationTick, FApplicationTick);

    PUBLIC_EVENT(OnApplicationRequestExit, FApplicationEvent);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
