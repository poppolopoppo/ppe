#pragma once

#include "RHI_fwd.h"

#include "RHI/ResourceEnums.h"

#include "Container/Stack.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ECompositeAlpha : u8 {
    Opaque = 0,
    PreMultiplied,
    PostMultiplied,
    Inherit,

    Unknown = UINT8_MAX,
};
//----------------------------------------------------------------------------
enum class EPresentMode : u8 {
    Immediate = 0,
    Fifo,
    RelaxedFifo,
    Mailbox,

    Unknown = UINT8_MAX,
};
//----------------------------------------------------------------------------
enum class ESurfaceTransform : u8 {
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

    friend bool operator ==(const FSurfaceFormat& lhs, const FSurfaceFormat& rhs) NOEXCEPT {
        return (lhs.Format == rhs.Format && lhs.ColorSpace == rhs.ColorSpace);
    }
    friend bool operator !=(const FSurfaceFormat& lhs, const FSurfaceFormat& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
struct FSwapchainDesc {
    using FRequiredSurfaceFormats = TFixedSizeStack<FSurfaceFormat, 4>;
    using FRequiredPresentModes = TFixedSizeStack<EPresentMode, 4>;

    FWindowSurface Surface{ nullptr };
    uint2 Dimensions{ 0 };
    ECompositeAlpha CompositeAlpha{ ECompositeAlpha::Opaque };
    ESurfaceTransform PreTransform{ ESurfaceTransform::Identity };
    u32 MinImageCount{ 2 };

    EImageUsage RequiredUsage{};
    EImageUsage OptionalUsage{};

    // both from higher to lower priority:
    FRequiredSurfaceFormats SurfaceFormats;
    FRequiredPresentModes PresentModes;
};
PPE_ASSUME_TYPE_AS_POD(FSwapchainDesc);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
