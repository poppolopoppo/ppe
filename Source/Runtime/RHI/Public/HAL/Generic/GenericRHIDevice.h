#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

#include "Maths/ScalarVector.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericPresentMode {
    Immediate = 0,
    Fifo,
    RelaxedFifo,
    Mailbox,
};
//----------------------------------------------------------------------------
class PPE_RHI_API FGenericDevice : Meta::FNonCopyable {
public: // must be defined by every RHI:
    FGenericDevice() = default;

    const TMemoryView<const EGenericPresentMode> PresentModes() const NOEXCEPT = delete;
    const TMemoryView<const FGenericSurfaceFormat> SurfaceFormats() const NOEXCEPT = delete;

    const FGenericSwapChain* SwapChain() const NOEXCEPT = delete;

    void CreateSwapChain(
        FGenericWindowSurface surface,
        EGenericPresentMode present,
        const FGenericSurfaceFormat& surfaceFormat ) = delete;
    void DestroySwapChain() = delete;

public: // shared by each device

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
