#pragma once

#include "Application_fwd.h"

#include "RHI_fwd.h"
#include "RHI/ResourceId.h"

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

    const RHI::FSwapchainID& SwapchainRHI() const { return _swapchainRHI; }
    void SetSwapchainRHI(RHI::FSwapchainID&& surfaceRHI) NOEXCEPT;
    void ReleaseSwapchainRHI() NOEXCEPT;

private:
    RHI::FSwapchainID _swapchainRHI;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
