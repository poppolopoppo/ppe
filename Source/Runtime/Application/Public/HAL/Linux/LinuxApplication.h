#pragma once

#include "HAL/GLFW/GLFWApplication.h"

#ifndef PLATFORM_LINUX
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FLinuxApplication : public FGLFWApplication {
public: // must be defined for every platform
    using parent_type = FGLFWApplication;

    explicit FLinuxApplication(const FModularDomain& domain, FString&& name);
    virtual ~FLinuxApplication();

    virtual void Start() override;
    virtual bool PumpMessages() NOEXCEPT override;
    virtual void Tick(FTimespan dt) override;
    virtual void Shutdown() override;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
