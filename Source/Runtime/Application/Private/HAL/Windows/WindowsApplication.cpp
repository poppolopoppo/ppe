#include "stdafx.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsApplication.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWindowsApplication::FWindowsApplication(const FModularDomain& domain, FWString&& name)
:   FGenericApplication(domain, std::move(name))
{}
//----------------------------------------------------------------------------
FWindowsApplication::~FWindowsApplication()
{}
//----------------------------------------------------------------------------
void FWindowsApplication::Start() {
    parent_type::Start();
}
//----------------------------------------------------------------------------
bool FWindowsApplication::PumpMessages() NOEXCEPT {
    return parent_type::PumpMessages();
}
//----------------------------------------------------------------------------
void FWindowsApplication::Tick(FTimespan dt) {
    parent_type::Tick(dt);
}
//----------------------------------------------------------------------------
void FWindowsApplication::Shutdown() {
    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
