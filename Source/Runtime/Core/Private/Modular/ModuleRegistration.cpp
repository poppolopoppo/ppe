#include "stdafx.h"

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
    WRITESCOPELOCK(_barrier);
    Assert_NoAssume(_modules.empty());
    _modules.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
bool FModuleStaticRegistration::Find(FModuleInfo* pinfo, const FStringView& name) const NOEXCEPT {
    Assert(pinfo);
    Assert_NoAssume(not name.empty());

    READSCOPELOCK(_barrier);

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
    RegisterModule(*static_cast<const FModuleInfo*>(anchor()));
}
//----------------------------------------------------------------------------
void FModuleStaticRegistration::UnregisterAnchor(FModuleStaticAnchor anchor) {
    Assert(anchor);
    UnregisterModule(*static_cast<const FModuleInfo*>(anchor()));
}
//----------------------------------------------------------------------------
void FModuleStaticRegistration::RegisterModule(const FModuleInfo& info) {
    WRITESCOPELOCK(_barrier);

    LOG(Modular, Info, L"register module <{0}> ({1})", info.Name, info.Phase);

    _modules.Insert_AssertUnique(info.Name, info);
}
//----------------------------------------------------------------------------
void FModuleStaticRegistration::UnregisterModule(const FModuleInfo& info) {
    WRITESCOPELOCK(_barrier);

    LOG(Modular, Info, L"unregister module <{0}> ({1})", info.Name, info.Phase);

    _modules.Remove_AssertExists(info.Name);
    _modules.shrink_to_fit(); // release mem asap to avoid static destroy fiasco
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
