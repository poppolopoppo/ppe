#include "stdafx.h"

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
    UNUSED(domain);
    LOG(Modular, Info, L"Start application module <{0}> in domain <{1}>", _name, domain.Name());
}
//----------------------------------------------------------------------------
void IModuleInterface::Shutdown(FModularDomain& domain) {
    UNUSED(domain);
    LOG(Modular, Info, L"Shutdown application module <{0}> in domain <{1}>", _name, domain.Name());
}
//----------------------------------------------------------------------------
void IModuleInterface::PostStart(FModularDomain& domain) {
    UNUSED(domain);
    LOG(Modular, Info, L"PostStart application module <{0}> in domain <{1}>", _name, domain.Name());
}
//----------------------------------------------------------------------------
void IModuleInterface::PreShutdown(FModularDomain& domain) {
    UNUSED(domain);
    LOG(Modular, Info, L"PreShutdown application module <{0}> in domain <{1}>", _name, domain.Name());
}
//----------------------------------------------------------------------------
void IModuleInterface::DutyCycle(FModularDomain& domain) {
    UNUSED(domain);
    LOG(Modular, Info, L"Duty cycle in application module <{0}> in domain <{1}>", _name, domain.Name());
}
//----------------------------------------------------------------------------
void IModuleInterface::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    UNUSED(domain);
    LOG(Modular, Info, L"Release memory in application module <{0}> in domain <{1}>", _name, domain.Name());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
