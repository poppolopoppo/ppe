#pragma once

#include "Application_fwd.h"

#include "RHI_fwd.h"

#include "Window/WindowBase.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(WindowRHI);
class PPE_APPLICATION_API FWindowRHI : public FWindowBase {
public:
    FWindowRHI(FWString&& title, const FWindowDefinition& def);

    RHI::FWindowSurface SurfaceRHI() const { return _surfaceRHI; }
    void SetSurfaceRHI(RHI::FWindowSurface surfaceRHI) NOEXCEPT;

private:
    RHI::FWindowSurface _surfaceRHI;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
