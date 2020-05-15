#include "stdafx.h"

#include "Modular/ModuleRegistration.h"

#include "Diagnostic/Logger.h"

namespace PPE {
EXTERN_LOG_CATEGORY(PPE_CORE_API, Modular)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FModuleRegistration::FModuleRegistration() {

}
//----------------------------------------------------------------------------
FModuleRegistration::~FModuleRegistration() {
    WRITESCOPELOCK(_barrier);
    Assert_NoAssume(_modules.empty());
    _modules.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
FModuleInfoContainer FModuleRegistration::All() const NOEXCEPT {
    READSCOPELOCK(_barrier);

    return _modules; // thread-safe copy
}
//----------------------------------------------------------------------------
bool FModuleRegistration::Find(FModuleInfo* pinfo, const FStringView& name) const NOEXCEPT {
    Assert(pinfo);
    Assert_NoAssume(not name.empty());

    READSCOPELOCK(_barrier);

    const auto it = std::find_if(
        _modules.begin(), _modules.end(),
        [name](const FModuleInfo& info) NOEXCEPT -> bool {
           return (info.Name == name);
        });

    if (Likely(_modules.end() != it)) {
        *pinfo = *it; // thread-safe copy
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
void FModuleRegistration::RegisterModule(const FModuleInfo& info) {
    WRITESCOPELOCK(_barrier);

    LOG(Modular, Info, L"register module <{0}> ({1})", info.Name, info.Phase);

    AssertReleaseMessage_NoAssume(
        L"a module with the same name was already registered",
        std::find_if(_modules.begin(), _modules.end(), [&info](const FModuleInfo& it) {
            return (it.Name == info.Name);
        }) == _modules.end() );


    _modules.emplace_back(info);
}
//----------------------------------------------------------------------------
void FModuleRegistration::UnregisterModule(const FModuleInfo& info) {
    WRITESCOPELOCK(_barrier);

    LOG(Modular, Info, L"unregister module <{0}> ({1})", info.Name, info.Phase);

    const auto it = std::find_if(
        _modules.begin(), _modules.end(),
        [&info](const FModuleInfo& it) NOEXCEPT -> bool {
            return (it.Name == info.Name);
        });

    AssertReleaseMessage_NoAssume(
        L"trying to unregister an unregistered module",
        _modules.end() != it );

    _modules.erase(it);
    _modules.shrink_to_fit(); // release mem asap to avoid static destroy fiasco
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FModuleRegistration& ModuleStaticRegistry() NOEXCEPT {
    ONE_TIME_DEFAULT_INITIALIZE(FModuleRegistration, GStaticRegistry);
    return GStaticRegistry;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
