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

    _services.Clear();
}
//----------------------------------------------------------------------------
void FModularServices::ReleaseMemory() NOEXCEPT {
    LOG(Modular, Verbose, L"releasing memory in '{0}' modular services", _name);

    _services.Broadcast<FReleaseMemoryFunc_>();
}
//----------------------------------------------------------------------------
#if USE_PPE_LOGGER
void FModularServices::LogServiceAdd_(FStringView base, FStringView derived) const NOEXCEPT {
    LOG(Modular, Verbose, L"add modular service <{0}> in '{1}' implemented with <{2}>", base, _name, derived);
}
void FModularServices::LogServiceRemove_(FStringView base) const NOEXCEPT {
    LOG(Modular, Verbose, L"remove modular service <{0}> from '{1}'", base, _name);
}
void FModularServices::LogServiceReleaseMemory_(FStringView base) NOEXCEPT {
    LOG(Modular, Verbose, L"release memory in service <{0}>", base);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
