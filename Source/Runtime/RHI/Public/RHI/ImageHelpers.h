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
CONSTEXPR FImageLayer operator "" _layer (unsigned long long value) {
    return FImageLayer(checked_cast<u32>(value));
}
//----------------------------------------------------------------------------
// FImageSwizzle user-literal: "RGBA"_swizzle
//----------------------------------------------------------------------------
CONSTEXPR FImageSwizzle operator "" _swizzle (const char* str, size_t len) {
    u16 value = 0;
    forrange(i, 0, len) {
        u16 channel = 0;
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
CONSTEXPR FImageSwizzle Swizzle(const uint4& v) {
    Assert(AllLess(v, uint4(7)));
    return FImageSwizzle{checked_cast<u16>(
        (v.x <<  0u) |
        (v.y <<  4u) |
        (v.z <<  8u) |
        (v.w << 12u) )};
}
//----------------------------------------------------------------------------
CONSTEXPR uint4 Unswizzle(FImageSwizzle swizzle) {
    return uint4{
        (swizzle.Value >>  0u) & 0xFu,
        (swizzle.Value >>  4u) & 0xFu,
        (swizzle.Value >>  8u) & 0xFu,
        (swizzle.Value >> 12u) & 0xFu
    };
}
//----------------------------------------------------------------------------
CONSTEXPR FImageSwizzle operator "" _swizzle (unsigned long long value) {
    const FImageSwizzle res(checked_cast<u16>(value));
    Assert_NoAssume(Swizzle(Unswizzle(res)) == res);
    return res;
}
//----------------------------------------------------------------------------
// FMipmapLevel
//----------------------------------------------------------------------------
CONSTEXPR FMipmapLevel operator "" _mipmap (unsigned long long value) {
    return FMipmapLevel(checked_cast<u32>(value));
}
//----------------------------------------------------------------------------
// FMultiSamples
//----------------------------------------------------------------------------
CONSTEXPR FMultiSamples operator "" _samples (unsigned long long value) {
    return FMultiSamples(checked_cast<u32>(value));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
