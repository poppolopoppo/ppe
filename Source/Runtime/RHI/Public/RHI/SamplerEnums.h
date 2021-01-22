#pragma once

#include "RHI_fwd.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EAddressMode : u32 {
    Repeat,
    MirrorRepeat,
    ClampToEdge,
    ClampToBorder,
    MirrorClampToEdge,

    Unknown	= ~0u,
};
//----------------------------------------------------------------------------
enum class EBorderColor : u32 {
    FloatTransparentBlack,
    FloatOpaqueBlack,
    FloatOpaqueWhite,

    IntTransparentBlack,
    IntOpaqueBlack,
    IntOpaqueWhite,

    Unknown	= ~0u,
};
//----------------------------------------------------------------------------
enum class EMipmapFilter : u32 {
    Nearest,
    Linear,

    Unknown	= ~0u,
};
//----------------------------------------------------------------------------
enum class ETextureFilter : u32 {
    Nearest,
    Linear,
    Anisotropic,

    Unknown	= ~0u,
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
