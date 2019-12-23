#include "stdafx.h"

#include "HAL/Windows/WindowsApplication.h"

#ifdef PLATFORM_WINDOWS

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWindowsApplication::FWindowsApplication(FWString&& name)
:   FGenericApplication(std::move(name))
{}
//----------------------------------------------------------------------------
FWindowsApplication::~FWindowsApplication()
{}
//----------------------------------------------------------------------------
void FWindowsApplication::Start() {
    parent_type::Start();
}
//----------------------------------------------------------------------------
void FWindowsApplication::PumpMessages() {
    parent_type::PumpMessages();
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
