#pragma once

#include "Core_fwd.h"

#include "Modular/Modular_fwd.h"
#include "Modular/ModuleInfo.h"

#include "Container/FlatMap.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern "C" typedef const void* (*FModuleStaticAnchor)();
//----------------------------------------------------------------------------
class PPE_CORE_API FModuleStaticRegistration : Meta::FNonCopyableNorMovable {
public:
    FModuleStaticRegistration();
    ~FModuleStaticRegistration();

    bool Find(FModuleInfo* pinfo, const FStringView& name) const NOEXCEPT;

    void RegisterAnchor(FModuleStaticAnchor anchor);
    void UnregisterAnchor(FModuleStaticAnchor anchor);

    void RegisterModule(const FModuleInfo& info);
    void UnregisterModule(const FModuleInfo& info);

    static FModuleStaticRegistration& Get() NOEXCEPT;

    template <typename _StaticModule>
    NODISCARD static CONSTEXPR FModuleInfo MakeInfo(
        const FStringView& name,
        EModulePhase phase,
        EModuleUsage usage,
        EModuleSource source,
        FModuleLoadOrder loadOrder,
        const FStringView& dependencyList ) NOEXCEPT {
        FModuleInfo info{
            name, phase, usage, source, loadOrder, dependencyList,
            CurrentBuildVersion(),
            []() NOEXCEPT -> TUniquePtr<IModuleInterface> {
                return MakeUnique<_StaticModule>();
            }
        };
        return info;
    }

private:
    FReadWriteLock _barrier;
    FLATMAP_INSITU(Modular, FStringView, FModuleInfo, 8) _modules;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
