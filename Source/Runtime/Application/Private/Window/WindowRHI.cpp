#include "stdafx.h"

#include "Window/WindowRHI.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWindowRHI::FWindowRHI(FWString&& title, const FWindowDefinition& def)
:   FWindowBase(std::move(title), def)
{}
//----------------------------------------------------------------------------
void FWindowRHI::SetSwapchainRHI(RHI::FSwapchainID&& swapchainRHI) NOEXCEPT {
    Assert(swapchainRHI);
    _swapchainRHI = std::move(swapchainRHI);
}
//----------------------------------------------------------------------------
void FWindowRHI::ReleaseSwapchainRHI() NOEXCEPT {
    Assert(_swapchainRHI);
    _swapchainRHI.Release();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
