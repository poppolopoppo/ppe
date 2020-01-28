#include "stdafx.h"

#include "HAL/Linux/LinuxApplication.h"

#ifdef PLATFORM_LINUX

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLinuxApplication::FLinuxApplication(FWString&& name)
:   FGenericApplication(std::move(name))
{}
//----------------------------------------------------------------------------
FLinuxApplication::~FLinuxApplication()
{}
//----------------------------------------------------------------------------
void FLinuxApplication::Start() {
    parent_type::Start();
}
//----------------------------------------------------------------------------
void FLinuxApplication::PumpMessages() {
    parent_type::PumpMessages();
}
//----------------------------------------------------------------------------
void FLinuxApplication::Tick(FTimespan dt) {
    parent_type::Tick(dt);
}
//----------------------------------------------------------------------------
void FLinuxApplication::Shutdown() {
    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_LINUX
