#pragma once

#include "Graphics.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EIndexElementSize {
    SixteenBits     = sizeof(u16),
    ThirtyTwoBits   = sizeof(u32)
};
//----------------------------------------------------------------------------
inline EIndexElementSize IndexCountToIndexElementSize(size_t indexCount) {
    return (indexCount <= UINT16_MAX
        ? EIndexElementSize::SixteenBits
        : EIndexElementSize::ThirtyTwoBits);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
