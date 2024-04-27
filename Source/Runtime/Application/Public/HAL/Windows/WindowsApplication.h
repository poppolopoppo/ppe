#pragma once

#include "HAL/Generic/GenericApplication.h"

#ifndef PLATFORM_WINDOWS
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FWindowsApplication : public FGenericApplication {
public: // must be defined for every platform
    using parent_type = FGenericApplication;

    explicit FWindowsApplication(FModularDomain& domain, FString&& name);

    virtual void Start() override;
    virtual void Shutdown() override;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
