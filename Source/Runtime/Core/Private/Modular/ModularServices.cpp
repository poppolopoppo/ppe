#include "stdafx.h"

#include "Modular/ModularServices.h"

#include "Diagnostic/Logger.h"

namespace PPE {
EXTERN_LOG_CATEGORY(, Modular)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FModularServices::FModularServices(const FStringView& name) NOEXCEPT
:   _name(name)
,   _parent(nullptr) {
    Assert_NoAssume(not _name.empty());
}
//----------------------------------------------------------------------------
FModularServices::FModularServices(const FStringView& name, const FModularServices* parent) NOEXCEPT
:   _name(name)
,   _parent(parent) {
    Assert_NoAssume(not _name.empty());
    Assert_NoAssume(parent);
}
//----------------------------------------------------------------------------
FModularServices::~FModularServices() = default;
//----------------------------------------------------------------------------
void FModularServices::Clear() {
    LOG(Modular, Verbose, L"clearing '{0}' modular services ", _name);

    _services.clear();
}
//----------------------------------------------------------------------------
void FModularServices::ReleaseMemory() NOEXCEPT {
    LOG(Modular, Verbose, L"releasing memory in '{0}' modular services", _name);

    for (const auto& it : _services) {
        if (it.second.ReleaseMemoryIFP())
            LOG(Modular, Verbose, L"released memory in <{0}> from '{1}' modular services", it.first.name, _name);
    }
}
//----------------------------------------------------------------------------
void* FModularServices::GetService_(const FServiceKey_& key) const NOEXCEPT {
    const auto it = _services.Find(key);
    if (_services.end() != it)
        return it->second.Get();

    if (_parent)
        return _parent->GetService_(key);

    LOG(Modular, Error, L"can't find service <{0}> in '{1}' modular services", key.name, _name);
    return nullptr;
}
//----------------------------------------------------------------------------
void FModularServices::AddService_(const FServiceKey_& key, FServiceHolder_&& rholder) {
    Assert_NoAssume(not _parent || not _parent->GetService_(key));

    LOG(Modular, Verbose, L"add modular service <{0}> in '{1}'", key.name, _name);

    _services.Insert_AssertUnique(FServiceKey_{ key }, std::move(rholder));
}
//----------------------------------------------------------------------------
void FModularServices::RemoveService_(const FServiceKey_& key) {
    Assert_NoAssume(not _parent || not _parent->GetService_(key));

    LOG(Modular, Verbose, L"remove modular service <{0}> from '{1}'", key.name, _name);

    _services.Remove_AssertExists(key);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
