#pragma once

#include "RHI_fwd.h"

#include "RHI/ResourceEnums.h"

#include "Container/Stack.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EPresentMode : u32 {
    Immediate = 0,
    Fifo,
    RelaxedFifo,
    Mailbox,

    Unknown = ~0u,
};
//----------------------------------------------------------------------------
enum class ESurfaceTransform : u32 {
    Identity = 0,
    TransformRotate90,
    TransformRotate180,
    TransformRotate270,
    HorizontalMirror,
    HorizontalMirror_TransformRotate90,
    HorizontalMirror_TransformRotate180,
    HorizontalMirror_TransformRotate270,
};
//----------------------------------------------------------------------------
struct FSurfaceFormat {
    EPixelFormat Format{ Default };
    EColorSpace ColorSpace{ Default };
};
//----------------------------------------------------------------------------
struct FSwapchainDesc {
    using FRequiredSurfaceFormats = TFixedSizeStack<FSurfaceFormat, 4>;
    using FRequiredPresentModes = TFixedSizeStack<EPresentMode, 4>;

    FWindowSurface Surface{ nullptr };
    ESurfaceTransform PreTransform{ ESurfaceTransform::Identity };
    u32 MinImageCount{ 2 };

    // both from higher to lower priority:
    FRequiredSurfaceFormats SurfaceFormats;
    FRequiredPresentModes PresentModes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
