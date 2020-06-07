#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

#include "Maths/ScalarVector.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHI_API FGenericSwapChain : Meta::FNonCopyable {
public: // must be defined by every RHI:
    FGenericSwapChain() = default;

    const FGenericSurfaceFormat& SurfaceFormat() const NOEXCEPT = delete;
    const u322 Extent() const NOEXCEPT = delete;
    const u322 NumImages() const NOEXCEPT = delete;

public: // shared by each device
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
