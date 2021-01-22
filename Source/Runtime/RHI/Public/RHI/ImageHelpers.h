#pragma once

#include "RHI_fwd.h"

#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FImageLayer user-literal : 0_layer
//----------------------------------------------------------------------------
CONSTEXPR FImageLayer operator "" _layer (u32 value) {
    return FImageLayer(value);
}
//----------------------------------------------------------------------------
// FImageSwizzle user-literal: "RGBA"_swizzle
//----------------------------------------------------------------------------
CONSTEXPR FImageSwizzle operator "" _swizzle (const uint4& v) {
    Assert(AllLess(v, uint4(7)));
    return FImageSwizzle{
        (v.x <<  0) |
        (v.y <<  4) |
        (v.z <<  8) |
        (v.w << 12)
    };
}
//----------------------------------------------------------------------------
CONSTEXPR FImageSwizzle operator "" _swizzle (const char* str, size_t len) {
    u32 value = 0;
    forrange(i, 0, len) {
        u32 channel;
        switch (str[i]) {
        case 'r': case 'R': channel = 1; break;
        case 'g': case 'G': channel = 2; break;
        case 'b': case 'B': channel = 3; break;
        case 'a': case 'A': channel = 4; break;
        case '0': channel = 5; break;
        case '1': channel = 6; break;
        default: AssertNotReached(); // only accepts [R,G,B,A,0,1]
        }

        value = (channel << (i*4));
    }

    return FImageSwizzle(value);
}
//----------------------------------------------------------------------------
CONSTEXPR uint4 operator "" _uint4 (FImageSwizzle swizzle) {
    return uint4{
        (swizzle.Value >>  0) & 0xF,
        (swizzle.Value >>  4) & 0xF,
        (swizzle.Value >>  8) & 0xF,
        (swizzle.Value >> 12) & 0xF
    };
}
//----------------------------------------------------------------------------
// FMipmapLevel
//----------------------------------------------------------------------------
CONSTEXPR FMipmapLevel operator "" _mipmap (u32 value) {
    return FMipmapLevel(value);
}
//----------------------------------------------------------------------------
// FMultiSamples
//----------------------------------------------------------------------------
CONSTEXPR FMultiSamples operator "" _samples (u32 value) {
    return FMultiSamples(value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
