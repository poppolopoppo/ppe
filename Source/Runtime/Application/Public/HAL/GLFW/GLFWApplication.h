#pragma once

#include "HAL/Generic/GenericApplication.h"

#ifndef PLATFORM_GLFW
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FGLFWApplication : public FGenericApplication {
public: // must be defined for every platform
    using parent_type = FGenericApplication;

    explicit FGLFWApplication(const FModularDomain& domain, FString&& name);
    virtual ~FGLFWApplication();

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
