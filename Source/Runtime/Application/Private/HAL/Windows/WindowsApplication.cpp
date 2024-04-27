// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsApplication.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWindowsApplication::FWindowsApplication(FModularDomain& domain, FString&& name)
:   FGenericApplication(domain, std::move(name))
{}
//----------------------------------------------------------------------------
void FWindowsApplication::Start() {
    parent_type::Start();
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
