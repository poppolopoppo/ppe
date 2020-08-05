#pragma once

#include "Core_fwd.h"

#include "Modular/Modular_fwd.h"
#include "Modular/ModuleEnums.h"

#include "Diagnostic/BuildVersion.h"
#include "Memory/UniquePtr.h"

#include <initializer_list>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FModuleDependencyList = TMemoryView<const FStringView>;
using FModuleStaticInitializer = TUniquePtr<IModuleInterface> (*)() NOEXCEPT;
//----------------------------------------------------------------------------
struct FModuleInfo {
    FStringView Name;
    EModulePhase Phase;
    EModuleUsage Usage;
    EModuleSource Source;
    FModuleLoadOrder LoadOrder;
    FModuleDependencyList DependencyList;
    FBuildVersion BuildVersion;
    FModuleStaticInitializer Initializer;

    FModuleInfo() = default;

    CONSTEXPR FModuleInfo(
        const FStringView& name,
        EModulePhase phase,
        EModuleUsage usage,
        EModuleSource source,
        FModuleLoadOrder loadOrder,
        FModuleDependencyList dependencyList,
        FBuildVersion buildVersion,
        FModuleStaticInitializer initializer ) NOEXCEPT
    :   Name(name)
    ,   Phase(phase)
    ,   Usage(usage)
    ,   Source(source)
    ,   LoadOrder(loadOrder)
    ,   DependencyList(dependencyList)
    ,   BuildVersion(buildVersion)
    ,   Initializer(initializer) {
        Assert_NoAssume(not Name.empty());
        Assert_NoAssume(Initializer);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
