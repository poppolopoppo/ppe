#pragma once

#include "HAL/Generic/GenericApplication.h"

#ifndef PLATFORM_LINUX
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FLinuxApplication : public FGenericApplication {
public: // must be defined for every platform
    using parent_type = FGenericApplication;

    explicit FLinuxApplication(FWString&& name);
    virtual ~FLinuxApplication();

    virtual void Start() override;
    virtual void PumpMessages() override;
    virtual void Tick(FTimespan dt) override;
    virtual void Shutdown() override;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
