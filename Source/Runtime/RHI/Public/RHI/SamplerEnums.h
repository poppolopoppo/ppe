#pragma once

#include "RHI_fwd.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EAddressMode : u8 {
    Repeat,
    MirrorRepeat,
    ClampToEdge,
    ClampToBorder,
    MirrorClampToEdge,

    Unknown	= UINT8_MAX,
};
//----------------------------------------------------------------------------
enum class EBorderColor : u8 {
    FloatTransparentBlack,
    FloatOpaqueBlack,
    FloatOpaqueWhite,

    IntTransparentBlack,
    IntOpaqueBlack,
    IntOpaqueWhite,

    Unknown	= UINT8_MAX,
};
//----------------------------------------------------------------------------
enum class EMipmapFilter : u8 {
    Nearest,
    Linear,

    Unknown	= UINT8_MAX,
};
//----------------------------------------------------------------------------
enum class ETextureFilter : u8 {
    Nearest,
    Linear,
    Cubic,

    Unknown	= UINT8_MAX,
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
