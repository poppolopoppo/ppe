#include "stdafx.h"

#ifdef PLATFORM_GLFW

#include "HAL/GLFW/GLFWApplication.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGLFWApplication::FGLFWApplication(const FModularDomain& domain, FString&& name)
:   FGenericApplication(domain, std::move(name))
{}
//----------------------------------------------------------------------------
FGLFWApplication::~FGLFWApplication()
{}
//----------------------------------------------------------------------------
void FGLFWApplication::Start() {
    parent_type::Start();
}
//----------------------------------------------------------------------------
bool FGLFWApplication::PumpMessages() NOEXCEPT {
    return parent_type::PumpMessages();
}
//----------------------------------------------------------------------------
void FGLFWApplication::Tick(FTimespan dt) {
    parent_type::Tick(dt);
}
//----------------------------------------------------------------------------
void FGLFWApplication::Shutdown() {
    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_GLFW
