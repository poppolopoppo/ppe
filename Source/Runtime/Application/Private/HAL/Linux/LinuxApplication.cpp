#include "stdafx.h"

#ifdef PLATFORM_LINUX

#include "HAL/Linux/LinuxApplication.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLinuxApplication::FLinuxApplication(const FModularDomain& domain, FString&& name)
:   FGenericApplication(domain, std::move(name))
{}
//----------------------------------------------------------------------------
FLinuxApplication::~FLinuxApplication()
{}
//----------------------------------------------------------------------------
void FLinuxApplication::Start() {
    parent_type::Start();
}
//----------------------------------------------------------------------------
bool FLinuxApplication::PumpMessages() NOEXCEPT {
    return parent_type::PumpMessages();
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
