#include "stdafx.h"

#include "Modular/ModularDomain.h"

#include "Modular/ModuleEnums.h"
#include "Modular/ModuleInterface.h"
#include "Modular/ModuleRegistration.h"

#include "Diagnostic/Logger.h"

namespace PPE {
LOG_CATEGORY(, Modular)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
FModularDomain* GAppDomainRef_ = nullptr;
//----------------------------------------------------------------------------
CONSTEXPR EModulePhase GApplicationPhases_[] = {
    EModulePhase::Bare,
    EModulePhase::System,
    EModulePhase::Framework,
    EModulePhase::Application,
    EModulePhase::User,
};
STATIC_ASSERT(lengthof(GApplicationPhases_) == size_t(EModulePhase::_Max));
//----------------------------------------------------------------------------
static CONSTEXPR int StatusPhaseMask_(EModulePhase phase) {
    const int packed = 1<<(int)phase;
    Assert(packed);
    return packed;
}
//----------------------------------------------------------------------------
static int StatusStartPhase_(int phaseStatus, EModulePhase phase) {
    const int phaseMask = StatusPhaseMask_(phase);
    Assert((phaseMask - 1) == phaseStatus); // check precedence
    return (phaseStatus | phaseMask);
}
//----------------------------------------------------------------------------
static int StatusShutdownPhase_(int phaseStatus, EModulePhase phase) {
    const int phaseMask = StatusPhaseMask_(phase);
    Assert((phaseMask | (phaseMask - 1)) == phaseStatus); // check precedence
    return (phaseStatus & ~phaseMask);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FModularDomain::FModularDomain(const FStringView& name, EModuleUsage usage) NOEXCEPT
:   _name(name)
,   _usage(usage)
,   _parent(nullptr)
,   _phaseStatus(0) {
    Assert_NoAssume(not _name.empty());
}
//----------------------------------------------------------------------------
FModularDomain::FModularDomain(const FStringView& name, EModuleUsage usage, FModularDomain& parent) NOEXCEPT
:   _name(name)
,   _usage(usage)
,   _parent(&parent)
,   _phaseStatus(0)
,   _services(&parent._services) {
    Assert_NoAssume(not _name.empty());
}
//----------------------------------------------------------------------------
FModularDomain::~FModularDomain() {
    AssertRelease(0 == _phaseStatus); // some phases are still started !
    Assert_NoAssume(_modules.empty()); // some modules are still loaded !
}
//----------------------------------------------------------------------------
bool FModularDomain::IsModuleLoaded(const FStringView& name) const NOEXCEPT {
    Assert_NoAssume(not name.empty());

    const auto it = _modules.Find(name);
    return (_modules.end() != it);
}
//----------------------------------------------------------------------------
IModuleInterface& FModularDomain::Module(const FStringView& name) const NOEXCEPT {
    IModuleInterface* const pmod = ModuleIFP(name);
    AssertReleaseMessage(L"failed to find module", pmod);
    return (*pmod);
}
//----------------------------------------------------------------------------
IModuleInterface* FModularDomain::ModuleIFP(const FStringView& name) const NOEXCEPT {
    Assert_NoAssume(not name.empty());

    const auto it = _modules.Find(name);
    return (_modules.end() != it
        ? it->second.get()
        : nullptr );
}
//----------------------------------------------------------------------------
IModuleInterface& FModularDomain::LoadModule(const FStringView& name) {
    IModuleInterface* const pmod = LoadModuleIFP(name);
    AssertReleaseMessage(L"failed to load module", pmod);
    return (*pmod);
}
//----------------------------------------------------------------------------
IModuleInterface* FModularDomain::LoadModuleIFP(const FStringView& name) {
    Assert_NoAssume(not name.empty());

    // check for a module already loaded:
    if (IModuleInterface* const present = ModuleIFP(name))
        return present;

    // check for a statically linked module:
    FModuleInfo info;
    if (not FModuleStaticRegistration::Get().Find(&info, name)) {
        LOG(Modular, Error, L"failed to find module <{0}>", name);
        return nullptr;
    }

    return LoadModule_(info);
}
//----------------------------------------------------------------------------
void FModularDomain::LoadDependencyList(const FStringView& dependencyList) {
    FStringView dependencyName, it = dependencyList;
    while (Split(it, ',', dependencyName)) {
        LOG(Modular, Info, L" -> load module dependency <{0}>", dependencyName);
        LoadModule(dependencyName);
    }
}
//----------------------------------------------------------------------------
void FModularDomain::UnloadModule(const FStringView& name) {
    VerifyRelease(UnloadModuleIFP(name));
}
//----------------------------------------------------------------------------
bool FModularDomain::UnloadModuleIFP(const FStringView& name) {
    Assert_NoAssume(not name.empty());

    const auto it = _modules.Find(name);
    if (_modules.end() == it)
        return false;

    AssertReleaseMessage(
        L"unloading a module when phase is still running",
        not HasPhaseStarted(it->second->Phase()) );

    _modules.Erase(it);
    return true;
}
//----------------------------------------------------------------------------
bool FModularDomain::HasPhaseStarted(EModulePhase phase) const NOEXCEPT {
    return (_phaseStatus & StatusPhaseMask_(phase));
}
//----------------------------------------------------------------------------
void FModularDomain::StartPhase(EModulePhase phase) {
    LOG(Modular, Emphasis, L"start application domain <{0}> phase {1}", _name, phase);

    _OnPrePhaseStart(*this, phase);

    _phaseStatus = StatusStartPhase_(_phaseStatus, phase);

    foreachitem(it, _modules) { // normal order for start, reversed for shutdown
        if (it->second->Phase() == phase)
            StartModule_(*it->second);
    }

    _OnPostPhaseStart(*this, phase);
}
//----------------------------------------------------------------------------
void FModularDomain::ShutdownPhase(EModulePhase phase) {
    LOG(Modular, Emphasis, L"shutdown application domain <{0}> phase {1}", _name, phase);

    _OnPrePhaseShutdown(*this, phase);

    _phaseStatus = StatusShutdownPhase_(_phaseStatus, phase);

    reverseforeachitem(it, _modules) { // reversed for shutdown, normal order for start
        if (it->second->Phase() == phase)
            ShutdownModule_(*it->second);
    }

    _OnPostPhaseShutdown(*this, phase);
}
//----------------------------------------------------------------------------
FModularDomain& FModularDomain::Get() NOEXCEPT {
    Assert(GAppDomainRef_);
    return (*GAppDomainRef_);
}
//----------------------------------------------------------------------------
void FModularDomain::Start(FModularDomain& domain) {
    Assert(nullptr == GAppDomainRef_);
    GAppDomainRef_ = &domain;

    foreachitem(phase, GApplicationPhases_)
        domain.StartPhase(*phase);
}
//----------------------------------------------------------------------------
void FModularDomain::Shutdown(FModularDomain& domain) {
    Assert(&domain == GAppDomainRef_);

    reverseforeachitem(phase, GApplicationPhases_)
        domain.ShutdownPhase(*phase);

    GAppDomainRef_ = nullptr;
}
//----------------------------------------------------------------------------
void FModularDomain::DutyCycle() {
    _OnPreDutyCycle(*this);

    reverseforeachitem(it, _modules)
        it->second->DutyCycle(*this);

    if (_parent)
        _parent->DutyCycle();

    _OnPostReleaseMemory(*this);
}
//----------------------------------------------------------------------------
void FModularDomain::ReleaseMemory() NOEXCEPT {
    _OnPreReleaseMemory(*this);

    reverseforeachitem(it, _modules)
        it->second->ReleaseMemory(*this);

    if (_parent)
        _parent->ReleaseMemory();

    _OnPostReleaseMemory(*this);
}
//----------------------------------------------------------------------------
IModuleInterface* FModularDomain::LoadModule_(const FModuleInfo& info) {
    if (not (info.Usage ^ _usage)) {
        LOG(Modular, Warning, L"ignored module <{0}> because of usage: {1} != {2}",
            info.Name, info.Usage, _usage );
        return nullptr;
    }

    LoadDependencyList(info.DependencyList);

    TUniquePtr<IModuleInterface> pmod{
        info.Initializer()
    };
    CLOG(not pmod, Modular, Fatal, L"failed to initialize module <{0}>", info.Name);

    Assert_NoAssume(pmod->Name() == info.Name);
    Assert_NoAssume(pmod->Phase() == info.Phase);

    IModuleInterface* const result = pmod.get();
    _modules.Emplace_AssertUnique(FStringView{ info.Name }, std::move(pmod));

    return result;
}
//----------------------------------------------------------------------------
void FModularDomain::StartModule_(IModuleInterface& mod) {
    _OnPreModuleStart(*this, mod);

    mod.Start(*this);

    _OnPostModuleStart(*this, mod);
}
//----------------------------------------------------------------------------
void FModularDomain::ShutdownModule_(IModuleInterface& mod) {
    _OnPreModuleShutdown(*this, mod);

    mod.Shutdown(*this);

    _OnPostModuleShutdown(*this, mod);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Free function helpers to avoid include headers
//----------------------------------------------------------------------------
void DutyCycleForModules() NOEXCEPT {
    FModularDomain::Get().DutyCycle();
}
//----------------------------------------------------------------------------
void ReleaseMemoryInModules() NOEXCEPT {
    FModularDomain::Get().ReleaseMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
