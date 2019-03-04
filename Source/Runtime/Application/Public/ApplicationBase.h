#pragma once

#include "Application.h"

#include "HAL/PlatformApplication.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Dummy wrapper for fwd declaration (FPlatformApplication is a using)
class PPE_APPLICATION_API FApplicationBase : public FPlatformApplication {
public:
    explicit FApplicationBase(FWString&& name);
    virtual ~FApplicationBase();

    virtual void Start() override;
    virtual void Shutdown() override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
