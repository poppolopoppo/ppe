#pragma once

#include "Core_fwd.h"

#include "Modular/Modular_fwd.h"
#include "Modular/ModuleInfo.h"

#include "Container/FlatMap.h"
#include "IO/StringView.h"
#include "Misc/DynamicLibrary.h"
#include "Thread/ReadWriteLock.h"

#include <mutex>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern "C" typedef const void* (*FModuleStaticAnchor)();
//----------------------------------------------------------------------------
inline const FModuleInfo& ModuleInfo(FModuleStaticAnchor anchor) NOEXCEPT {
    return (*static_cast<const FModuleInfo*>(anchor()));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
        FStringLiteral name,
        EModulePhase phase,
        EModuleUsage usage,
        EModuleSource source,
        FModuleLoadOrder loadOrder,
        FModuleDependencyList&& dependencyList,
        FBuildVersion&& buildVersion ) NOEXCEPT {
        FModuleInfo info{
            name, phase, usage, source, loadOrder,
            std::move(dependencyList),
            std::move(buildVersion),
            []() NOEXCEPT -> TUniquePtr<IModuleInterface> {
                return MakeUnique<_StaticModule>();
            }
        };
        return info;
    }

private:
    FReadWriteLock _rwlock;
    FLATMAP_INSITU(Modular, FStringView, FModuleInfo, 8) _modules;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FModuleDynamicRegistration : Meta::FNonCopyableNorMovable {
public:
    FModuleDynamicRegistration();
    ~FModuleDynamicRegistration();

    bool Load(FModuleInfo* pinfo, const FStringView& name) NOEXCEPT;
    void Unload(const FStringView& name) NOEXCEPT;
    bool UnloadIFP(const FStringView& name) NOEXCEPT;

    void RegisterLibrary(
        FStringLiteral name,
        const char* anchor,
        const wchar_t* path );
    void UnregisterLibrary(const FStringView& name);

    static FModuleDynamicRegistration& Get() NOEXCEPT;

private:
    struct FDynamicModule {
        int RefCount;
        const char* Anchor;
        const wchar_t* Path;
        FDynamicLibrary Library;

        FDynamicModule(const char* anchor, const wchar_t* path) NOEXCEPT
        :   RefCount(0)
        ,   Anchor(anchor)
        ,   Path(path)
        {}
    };

    std::mutex _barrier;
    FLATMAP_INSITU(Modular, FStringView, FDynamicModule, 8) _modules;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
