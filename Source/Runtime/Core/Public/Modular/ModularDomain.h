#pragma once

#include "Core_fwd.h"

#include "Modular/Modular_fwd.h"
#include "Modular/ModularServices.h"
#include "Modular/ModuleEnums.h"

#include "Misc/Event.h"
#include "Misc/Function.h"

#include "Container/AssociativeVector.h"
#include "IO/StringView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FModularDomain : Meta::FNonCopyableNorMovable {
public:
    FModularDomain(const FStringView& name, EModuleUsage usage) NOEXCEPT;
    FModularDomain(const FStringView& name, EModuleUsage usage, FModularDomain& parent) NOEXCEPT;
    ~FModularDomain();

    const FStringView& Name() const { return _name; }
    EModuleUsage Usage() const { return _usage; }

    const FModularDomain* Parent() const { return _parent; }

    FModularServices& Services() { return _services; }
    const FModularServices& Services() const { return _services; }

    bool IsModuleLoaded(const FStringView& name) const NOEXCEPT;

    IModuleInterface& Module(const FStringView& name) const NOEXCEPT;
    IModuleInterface* ModuleIFP(const FStringView& name) const NOEXCEPT;

    IModuleInterface& LoadModule(const FStringView& name);
    IModuleInterface* LoadModuleIFP(const FStringView& name);

    void LoadDependencyList(const FStringView& dependencyList); // comma separated module name list

    void UnloadModule(const FStringView& name);
    bool UnloadModuleIFP(const FStringView& name);

    bool HasPhaseStarted(EModulePhase) const NOEXCEPT;

    void StartPhase(EModulePhase phase);
    void ShutdownPhase(EModulePhase phase);

    void DutyCycle();
    void ReleaseMemory() NOEXCEPT;

    static FModularDomain& Get() NOEXCEPT;

    static void Start(FModularDomain& domain);
    static void Shutdown(FModularDomain& domain);

public: // events
    using FModuleDelegate = TFunction<void(const FModularDomain&, IModuleInterface&)>;

    PUBLIC_EVENT(OnLoadModule, FModuleDelegate)
    PUBLIC_EVENT(OnUnloadModule, FModuleDelegate)

    PUBLIC_EVENT(OnPreModuleStart, FModuleDelegate)
    PUBLIC_EVENT(OnPostModuleStart, FModuleDelegate)

    PUBLIC_EVENT(OnPreModuleShutdown, FModuleDelegate)
    PUBLIC_EVENT(OnPostModuleShutdown, FModuleDelegate)

    using FPhaseDelegate = TFunction<void(const FModularDomain&, EModulePhase)>;

    PUBLIC_EVENT(OnPrePhaseStart, FPhaseDelegate)
    PUBLIC_EVENT(OnPostPhaseStart, FPhaseDelegate)

    PUBLIC_EVENT(OnPrePhaseShutdown, FPhaseDelegate)
    PUBLIC_EVENT(OnPostPhaseShutdown, FPhaseDelegate)

    using FDomainDelegate = TFunction<void(const FModularDomain&)>;

    PUBLIC_EVENT(OnPreDutyCycle, FDomainDelegate)
    PUBLIC_EVENT(OnPostDutyCycle, FDomainDelegate)

    PUBLIC_EVENT(OnPreReleaseMemory, FDomainDelegate)
    PUBLIC_EVENT(OnPostReleaseMemory, FDomainDelegate)

private:
    const FStringView _name;
    const EModuleUsage _usage;
    FModularDomain* const _parent;

    int _phaseStatus;

    FModularServices _services;

    ASSOCIATIVE_VECTOR(Modular, FStringView, TUniquePtr<IModuleInterface>) _modules;

    IModuleInterface* LoadModule_(const FModuleInfo& info);

    void StartModule_(IModuleInterface& mod);
    void ShutdownModule_(IModuleInterface& mod);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
