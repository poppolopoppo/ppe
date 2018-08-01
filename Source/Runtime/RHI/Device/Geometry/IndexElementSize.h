#pragma once

#include "Core.Graphics/Graphics.h"

namespace Core {
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
} //!namespace Core
