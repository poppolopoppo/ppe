﻿#include "stdafx.h"

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
FModuleStaticRegistration::FModuleStaticRegistration()
{}
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

    const auto it = _modules.Find(name);
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

    LOG(Modular, Info, L"register static module <{0}> ({1})", info.Name, info.Phase);

    WRITESCOPELOCK(_rwlock);

    _modules.Insert_AssertUnique(info.Name, info);
}
//----------------------------------------------------------------------------
void FModuleStaticRegistration::UnregisterModule(const FModuleInfo& info) {
    Assert_NoAssume(not info.Name.empty());
    Assert_NoAssume(info.Initializer);

    LOG(Modular, Info, L"unregister static module <{0}> ({1})", info.Name, info.Phase);

    WRITESCOPELOCK(_rwlock);

    _modules.Remove_AssertExists(info.Name);
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
FModuleDynamicRegistration::FModuleDynamicRegistration() {

}
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

    const auto it = _modules.Find(name);
    if (Unlikely(_modules.end() == it))
        return false;

    FDynamicModule& dynamic = it->second;

    // may need to load the library if first hit:
    if (not (0 == dynamic.RefCount
            ? dynamic.Library.AttachOrLoad(dynamic.Path)
            : dynamic.Library.IsValid()) ) {
        LOG(Modular, Error, L"failed to load dynamic module <{0}> from '{1}'",
            name, MakeCStringView(dynamic.Path) );
        return false;
    }

    dynamic.RefCount++; // only increment refcount if the library was loaded

    void* const fAnchor = dynamic.Library.FunctionAddr(dynamic.Anchor);
    if (not fAnchor) {
        LOG(Modular, Error, L"failed to find anchor '{0}' inside module <{1}>",
            MakeCStringView(dynamic.Anchor), name );
        return false;
    }

    *pinfo = ModuleInfo(static_cast<FModuleStaticAnchor>(fAnchor));
    AssertRelease(name == pinfo->Name);

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

    const auto it = _modules.Find(name);
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
    const FStringView& name,
    const char *anchor,
    const wchar_t *path ) {
    Assert_NoAssume(not name.empty());
    Assert(anchor);
    Assert(path);

    LOG(Modular, Info, L"register dynamic module <{0}> ({1})", name, MakeCStringView(path));

    const Meta::FLockGuard scopeLock(_barrier);
    _modules.Emplace_AssertUnique(name, anchor, path);
}
//----------------------------------------------------------------------------
void FModuleDynamicRegistration::UnregisterLibrary(const FStringView& name) {
    Assert_NoAssume(not name.empty());

    LOG(Modular, Info, L"unregister dynamic module <{0}>", name);

    const Meta::FLockGuard scopeLock(_barrier);

    _modules.Remove_AssertExists(name);
    _modules.shrink_to_fit(); // release mem asap to avoid static destroy fiasco
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE