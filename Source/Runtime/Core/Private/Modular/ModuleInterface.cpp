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
    LOG(Modular, Info, L"start application module <{0}> in domain <{1}>", _name, domain.Name());
}
//----------------------------------------------------------------------------
void IModuleInterface::Shutdown(FModularDomain& domain) {
    Unused(domain);
    LOG(Modular, Info, L"shutdown application module <{0}> in domain <{1}>", _name, domain.Name());
}
//----------------------------------------------------------------------------
void IModuleInterface::PostStart(FModularDomain& domain) {
    Unused(domain);
    LOG(Modular, Info, L"post-start application module <{0}> in domain <{1}>", _name, domain.Name());
}
//----------------------------------------------------------------------------
void IModuleInterface::PreShutdown(FModularDomain& domain) {
    Unused(domain);
    LOG(Modular, Info, L"pre-shutdown application module <{0}> in domain <{1}>", _name, domain.Name());
}
//----------------------------------------------------------------------------
void IModuleInterface::DutyCycle(FModularDomain& domain) {
    Unused(domain);
    LOG(Modular, Info, L"duty-cycle in application module <{0}> in domain <{1}>", _name, domain.Name());
}
//----------------------------------------------------------------------------
void IModuleInterface::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    Unused(domain);
    LOG(Modular, Info, L"release memory in application module <{0}> in domain <{1}>", _name, domain.Name());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
