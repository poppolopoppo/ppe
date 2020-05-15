#pragma once

#include "Core_fwd.h"

#include "Modular/Modular_fwd.h"
#include "Modular/ModuleInfo.h"

#include "Container/Vector.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FModuleInfoContainer = VECTOR(Modular, FModuleInfo);
//----------------------------------------------------------------------------
class PPE_CORE_API FModuleRegistration : Meta::FNonCopyableNorMovable {
public:
    FModuleRegistration();
    ~FModuleRegistration();

    FModuleInfoContainer All() const NOEXCEPT;

    bool Find(FModuleInfo* pinfo, const FStringView& name) const NOEXCEPT;

    void RegisterModule(const FModuleInfo& info);
    void UnregisterModule(const FModuleInfo& info);

private:
    FReadWriteLock _barrier;
    FModuleInfoContainer _modules;
};
//----------------------------------------------------------------------------
PPE_CORE_API FModuleRegistration& ModuleStaticRegistry() NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FModuleStaticRegistration {

    template <typename _StaticModule>
    NODISCARD static CONSTEXPR FModuleInfo MakeInfo(
        const FStringView& name,
        EModulePhase phase,
        EModuleUsage usage,
        EModuleSource source,
        FModuleLoadOrder loadOrder,
        const FStringView& dependencyList ) NOEXCEPT {
        FModuleInfo staticModuleInfo{
            name, phase, usage, source, loadOrder, dependencyList,
            CurrentBuildVersion(),
            []() NOEXCEPT -> TUniquePtr<IModuleInterface> {
                return MakeUnique<_StaticModule>();
            }
        };
        return staticModuleInfo;
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
