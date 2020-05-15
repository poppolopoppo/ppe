#include "stdafx.h"

#include "Modular/ModularServices.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FModularServices::FModularServices()
:     _parent(nullptr)
{ }
//----------------------------------------------------------------------------
FModularServices::FModularServices(FModularServices* parent)
:     _parent(parent)
{ }
//----------------------------------------------------------------------------
FModularServices::~FModularServices() = default;
//----------------------------------------------------------------------------
void FModularServices::Clear() {
    _services.clear();
}
//----------------------------------------------------------------------------
void* FModularServices::GetService_(FServiceKey_ key) const NOEXCEPT {
    const auto it = _services.Find(key);
    return ((_services.end() == it)
        ? (_parent ? _parent->GetService_(key) : nullptr)
        : it->second->Get() );
}
//----------------------------------------------------------------------------
void FModularServices::AddService_(FServiceKey_ key, UServiceHolder_&& rholder) {
    Assert_NoAssume(not _parent || not _parent->GetService_(key));
    _services.Insert_AssertUnique(std::move(key), std::move(rholder));
}
//----------------------------------------------------------------------------
void FModularServices::RemoveService_(FServiceKey_ key) {
    if (_parent) {
        if (not _services.Erase(key))
            _parent->RemoveService_(key);
    }
    else {
        _services.Remove_AssertExists(key);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
