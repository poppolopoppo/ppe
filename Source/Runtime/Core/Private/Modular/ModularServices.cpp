// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
    PPE_SLOG(Modular, Verbose, "clearing modular services", {{"domain", _name}});

    _services.Clear();
}
//----------------------------------------------------------------------------
void FModularServices::Run(const FModularDomain& domain) {
    PPE_SLOG(Modular, Verbose, "running memory domain in modular services", {{"domain", _name}});

    _services.Broadcast<run_f>(domain);
}
//----------------------------------------------------------------------------
void FModularServices::ReleaseMemory() NOEXCEPT {
    PPE_SLOG(Modular, Verbose, "releasing memory in modular services", {{"domain", _name}});

    _services.Broadcast<release_memory_f>();
}
//----------------------------------------------------------------------------
#if USE_PPE_LOGGER
void FModularServices::LogServiceAdd_(FStringView base, FStringView derived) const NOEXCEPT {
    PPE_SLOG(Modular, Verbose, "add modular service", {{"domain", _name}, {"service", base}, {"type", derived}});
}
void FModularServices::LogServiceRemove_(FStringView base) const NOEXCEPT {
    PPE_SLOG(Modular, Verbose, "remove modular service", {{"domain", _name}, {"service", base}});
}
void FModularServices::LogServiceRun_(FStringView base) NOEXCEPT {
    PPE_SLOG(Modular, Verbose, "run memory domain in modular service", {{"service", base}});
}
void FModularServices::LogServiceReleaseMemory_(FStringView base) NOEXCEPT {
    PPE_SLOG(Modular, Verbose, "release memory in modular service", {{"service", base}});
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
