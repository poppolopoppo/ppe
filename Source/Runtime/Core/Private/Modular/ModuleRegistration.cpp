// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Modular/ModuleRegistration.h"

#include "Diagnostic/Logger.h"

namespace PPE {
EXTERN_LOG_CATEGORY(PPE_CORE_API, Modular)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FModuleStaticRegistration& FModuleStaticRegistration::Get() NOEXCEPT {
    ONE_TIME_DEFAULT_INITIALIZE(FModuleStaticRegistration, GStaticRegistry);
    return GStaticRegistry;
}
//----------------------------------------------------------------------------
FModuleStaticRegistration::FModuleStaticRegistration() = default;
//----------------------------------------------------------------------------
FModuleStaticRegistration::~FModuleStaticRegistration() {
    WRITESCOPELOCK(_rwlock);
    Assert_NoAssume(_modules.empty());
    _modules.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
bool FModuleStaticRegistration::Find(FModuleInfo* pinfo, const FStringView& name) const NOEXCEPT {
    Assert(pinfo);
    Assert_NoAssume(not name.empty());

    READSCOPELOCK(_rwlock);

    const auto it = _modules.find(name);
    if (Likely(_modules.end() != it)) {
        *pinfo = it->second; // thread-safe copy
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
void FModuleStaticRegistration::RegisterAnchor(FModuleStaticAnchor anchor) {
    Assert(anchor);

    RegisterModule(ModuleInfo(anchor));
}
//----------------------------------------------------------------------------
void FModuleStaticRegistration::UnregisterAnchor(FModuleStaticAnchor anchor) {
    Assert(anchor);

    UnregisterModule(ModuleInfo(anchor));
}
//----------------------------------------------------------------------------
void FModuleStaticRegistration::RegisterModule(const FModuleInfo& info) {
    Assert_NoAssume(not info.Name.empty());
    Assert_NoAssume(info.Initializer);

    PPE_LOG(Modular, Info, "register static module <{0}> ({1})", info.Name, info.Phase);

    WRITESCOPELOCK(_rwlock);

    _modules.Insert_AssertUnique(info.Name.MakeView(), info);
}
//----------------------------------------------------------------------------
void FModuleStaticRegistration::UnregisterModule(const FModuleInfo& info) {
    Assert_NoAssume(not info.Name.empty());
    Assert_NoAssume(info.Initializer);

    PPE_LOG(Modular, Info, "unregister static module <{0}> ({1})", info.Name, info.Phase);

    WRITESCOPELOCK(_rwlock);

    _modules.Remove_AssertExists(info.Name.MakeView());
    _modules.shrink_to_fit(); // release mem asap to avoid static destroy fiasco
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FModuleDynamicRegistration& FModuleDynamicRegistration::Get() NOEXCEPT {
    ONE_TIME_DEFAULT_INITIALIZE(FModuleDynamicRegistration, GDynamicRegistry);
    return GDynamicRegistry;
}
//----------------------------------------------------------------------------
FModuleDynamicRegistration::FModuleDynamicRegistration() = default;
//----------------------------------------------------------------------------
FModuleDynamicRegistration::~FModuleDynamicRegistration() {
    const Meta::FLockGuard scopeLock(_barrier);
    Assert_NoAssume(_modules.empty());
    _modules.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
bool FModuleDynamicRegistration::Load(FModuleInfo* pinfo, const FStringView& name) NOEXCEPT {
    Assert(pinfo);
    Assert_NoAssume(not name.empty());

    const Meta::FLockGuard scopeLock(_barrier);

    const auto it = _modules.find(name);
    if (Unlikely(_modules.end() == it))
        return false;

    FDynamicModule& dynamic = it->second;

    // may need to load the library if first hit:
    if (not (0 == dynamic.RefCount
            ? dynamic.Library.AttachOrLoad(dynamic.Path)
            : dynamic.Library.IsValid()) ) {
        PPE_LOG(Modular, Error, "failed to load dynamic module <{0}> from '{1}'",
            name, MakeCStringView(dynamic.Path) );
        return false;
    }

    dynamic.RefCount++; // only increment refcount if the library was loaded

    void* const fAnchor = dynamic.Library.FunctionAddr(dynamic.Anchor);
    if (not fAnchor) {
        PPE_LOG(Modular, Error, "failed to find anchor '{0}' inside module <{1}>",
            MakeCStringView(dynamic.Anchor), name );
        return false;
    }

    *pinfo = ModuleInfo(reinterpret_cast<FModuleStaticAnchor>(fAnchor));
    AssertRelease(name == pinfo->Name.MakeView());

    return true;
}
//----------------------------------------------------------------------------
void FModuleDynamicRegistration::Unload(const FStringView& name) NOEXCEPT {
    VerifyRelease(UnloadIFP(name));
}
//----------------------------------------------------------------------------
bool FModuleDynamicRegistration::UnloadIFP(const FStringView& name) NOEXCEPT {
    Assert_NoAssume(not name.empty());

    const Meta::FLockGuard scopeLock(_barrier);

    const auto it = _modules.find(name);
    if (_modules.end() == it)
        return false;

    FDynamicModule& dynamic = it->second;
    Verify(--dynamic.RefCount >= 0);

    // may need to unload the library if last hit:
    if (0 == dynamic.RefCount)
        dynamic.Library.Unload();

    return true;
}
//----------------------------------------------------------------------------
void FModuleDynamicRegistration::RegisterLibrary(
    FStringLiteral name,
    const char *anchor,
    const wchar_t *path ) {
    Assert_NoAssume(not name.empty());
    Assert(anchor);
    Assert(path);

    PPE_LOG(Modular, Info, "register dynamic module <{0}> ({1})", name, MakeCStringView(path));

    const Meta::FLockGuard scopeLock(_barrier);
    _modules.Emplace_AssertUnique(name.MakeView(), anchor, path);
}
//----------------------------------------------------------------------------
void FModuleDynamicRegistration::UnregisterLibrary(const FStringView& name) {
    Assert_NoAssume(not name.empty());

    PPE_LOG(Modular, Info, "unregister dynamic module <{0}>", name);

    const Meta::FLockGuard scopeLock(_barrier);

    _modules.Remove_AssertExists(name);
    _modules.shrink_to_fit(); // release mem asap to avoid static destroy fiasco
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
