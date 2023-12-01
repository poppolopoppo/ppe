// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Modular/ModuleInterface.h"

#include "Modular/ModularDomain.h"
#include "Modular/ModuleInfo.h"

#include "Diagnostic/Logger.h"

namespace PPE {
EXTERN_LOG_CATEGORY(PPE_CORE_API, Modular)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
IModuleInterface::IModuleInterface(const FModuleInfo& info) NOEXCEPT
:   _name(info.Name)
,   _phase(info.Phase) {
    Assert_NoAssume(not _name.empty());
}
//----------------------------------------------------------------------------
IModuleInterface::~IModuleInterface() NOEXCEPT {}
//----------------------------------------------------------------------------
void IModuleInterface::Start(FModularDomain& domain) {
    Unused(domain);
    PPE_SLOG(Modular, Info, "start application module", {{"module", _name}, {"domain", domain.Name()}});
}
//----------------------------------------------------------------------------
void IModuleInterface::Shutdown(FModularDomain& domain) {
    Unused(domain);
    PPE_SLOG(Modular, Info, "shutdown application module", {{"module", _name}, {"domain", domain.Name()}});
}
//----------------------------------------------------------------------------
void IModuleInterface::PostStart(FModularDomain& domain) {
    Unused(domain);
    PPE_SLOG(Modular, Info, "post-start application module", {{"module", _name}, {"domain", domain.Name()}});
}
//----------------------------------------------------------------------------
void IModuleInterface::PreShutdown(FModularDomain& domain) {
    Unused(domain);
    PPE_SLOG(Modular, Info, "pre-shutdown application module", {{"module", _name}, {"domain", domain.Name()}});
}
//----------------------------------------------------------------------------
void IModuleInterface::DutyCycle(FModularDomain& domain) {
    Unused(domain);
    PPE_SLOG(Modular, Info, "duty-cycle in application module", {{"module", _name}, {"domain", domain.Name()}});
}
//----------------------------------------------------------------------------
void IModuleInterface::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    Unused(domain);
    PPE_SLOG(Modular, Info, "release memory in application module", {{"module", _name}, {"domain", domain.Name()}});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
