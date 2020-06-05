#include "stdafx.h"

#include "Window/WindowRHI.h"

#include "HAL/RHIInstance.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWindowRHI::FWindowRHI(FWString&& title, const FWindowDefinition& def)
:   FWindowBase(std::move(title), def)
{}
//----------------------------------------------------------------------------
void FWindowRHI::SetSurfaceRHI(RHI::FWindowSurface surfaceRHI) NOEXCEPT {
    _surfaceRHI = surfaceRHI;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
