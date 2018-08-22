#include "stdafx.h"

#include "HAL/Windows/WindowsApplication.h"

#ifdef PLATFORM_WINDOWS

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FWindowsApplication* GAppInstance = nullptr;
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWindowsApplication& FWindowsApplication::Get() {
    AssertRelease(GAppInstance);
    return (*GAppInstance);
}
//----------------------------------------------------------------------------
FWindowsApplication::FWindowsApplication(FWString&& name)
:   FGenericApplication(std::move(name)) {
    Assert(nullptr == GAppInstance);
    GAppInstance = this;
}
//----------------------------------------------------------------------------
FWindowsApplication::~FWindowsApplication() {
    Assert(this == GAppInstance);
    GAppInstance = nullptr;
}
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
