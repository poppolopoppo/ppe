#pragma once

#include "Core_fwd.h"

#include "Modular/Modular_fwd.h"
#include "Modular/ModuleEnums.h"

#include "IO/StringView.h"
#include "Time/Timestamp.h"
#include "Memory/UniquePtr.h"

#include <initializer_list>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FModuleDependencyList = TMemoryView<const FStringView>;
using FModuleStaticInitializer = TUniquePtr<IModuleInterface> (*)() NOEXCEPT;
//----------------------------------------------------------------------------
struct FBuildVersion {
    FStringLiteral Branch;
    FStringLiteral Revision;
    FStringLiteral Family;
    FStringLiteral Compiler;
    FTimestamp Timestamp;
};
//----------------------------------------------------------------------------
struct FModuleInfo {
    FStringLiteral Name;
    EModulePhase Phase{};
    EModuleUsage Usage{};
    EModuleSource Source{};
    FModuleLoadOrder LoadOrder{};
    FModuleStaticInitializer Initializer{};
    FModuleDependencyList DependencyList;
    FBuildVersion BuildVersion;

    FModuleInfo() = default;

    CONSTEXPR FModuleInfo(
        FStringLiteral name,
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
    ,   Initializer(initializer)
    ,   DependencyList(dependencyList)
    ,   BuildVersion(buildVersion) {
        Assert_NoAssume(not Name.empty());
        Assert_NoAssume(Initializer);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
