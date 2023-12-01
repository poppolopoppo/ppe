#pragma once

#include "ApplicationUI_fwd.h"

#include "Diagnostic/Logger_fwd.h"
#include "Misc/EventHandle.h"
#include "Modular/ModularDomain.h"
#include "Modular/ModuleInterface.h"

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATIONUI_API, UI)
} //!namespace Application
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATIONUI_API FApplicationUIModule final : public IModuleInterface {
public:
    static const FModuleInfo StaticInfo;

    explicit FApplicationUIModule() NOEXCEPT;

    virtual void Start(FModularDomain& domain) override;
    virtual void Shutdown(FModularDomain& domain) override;

    virtual void DutyCycle(FModularDomain& domain) override;
    virtual void ReleaseMemory(FModularDomain& domain) NOEXCEPT override;

public:
    static FApplicationUIModule& Get(const FModularDomain& domain);

private:
    void OnApplicationStart_(Application::FApplicationBase& app, FModularServices& services) NOEXCEPT;
    void OnApplicationShutdown_(Application::FApplicationBase& app, FModularServices& services) NOEXCEPT;

    TUniquePtr<Application::FImGuiService> _ui;
    FEventHandle _onApplicationStart;
    FEventHandle _onApplicationShutdown;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
