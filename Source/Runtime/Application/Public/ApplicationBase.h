#pragma once

#include "Application.h"

#include "HAL/PlatformApplication.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Dummy wrapper for fwd declaration (FPlatformApplication is a using)
class FApplicationBase : public FPlatformApplication {
public:
    explicit FApplicationBase(FWString&& name);
    virtual ~FApplicationBase();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
