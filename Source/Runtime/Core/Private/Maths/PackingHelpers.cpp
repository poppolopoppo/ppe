#include "stdafx.h"

#include "PackingHelpers.h"

#include "Maths/ScalarVectorHelpers.h"
#include "Maths/Quaternion.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
    // https://gist.github.com/rygorous/2156668

    union FP32 {
        u32 u;
        float f;
        struct ieee {
            u32 Mantissa    : 23;
            u32 Exponent    : 8;
            u32 Sign        : 1;
        };
    };

    union FP16 {
        u16 u;
        struct ieee {
            u16 Mantissa    : 10;
            u16 Exponent    : 5;
            u16 Sign        : 1;
        };
    };
} //!namespace
//----------------------------------------------------------------------------
float FP16_to_FP32(u16 value) {
    static constexpr FP32 magic = { 113 << 23 };
    static constexpr u32 shifted_exp = 0x7c00 << 13; // exponent mask after shift

    FP16 h;
    h.u = value;
    FP32 o;

    o.u = (h.u & 0x7fff) << 13;     // exponent/mantissa bits
    u32 exp = shifted_exp & o.u;   // just the exponent
    o.u += (127 - 15) << 23;        // exponent adjust

    // handle exponent special cases
    if (exp == shifted_exp) // Inf/NaN?
        o.u += (128 - 16) << 23;    // extra exp adjust
    else if (exp == 0) // Zero/Denormal?
    {
        o.u += 1 << 23;             // extra exp adjust
        o.f -= magic.f;             // renormalize
    }

    o.u |= (h.u & 0x8000) << 16;    // sign bit
    return o.f;
}
//----------------------------------------------------------------------------
u16 FP32_to_FP16(float value) {
    Assert(FHalfFloat::IsConvertible(value));

    static constexpr FP32 f32infty = { 255 << 23 };
    static constexpr FP32 f16infty = { 31 << 23 };
    static constexpr FP32 magic = { 15 << 23 };
    static constexpr u32 sign_mask = 0x80000000u;
    static constexpr u32 round_mask = ~0xfffu;

    FP32 f;
    f.f = value;
    FP16 o = { 0 };

    u32 sign = f.u & sign_mask;
    f.u ^= sign;

    // NOTE all the integer compares in this function can be safely
    // compiled into signed compares since all operands are below
    // 0x80000000. Important if you want fast straight SSE2 code
    // (since there's no unsigned PCMPGTD).

    if (f.u >= f32infty.u) // Inf or NaN (all exponent bits set)
        o.u = (f.u > f32infty.u) ? 0x7e00 : 0x7c00; // NaN->qNaN and Inf->Inf
    else // (De)normalized number or zero
    {
        f.u &= round_mask;
        f.f *= magic.f;
        f.u -= round_mask;
        if (f.u > f16infty.u) f.u = f16infty.u; // Clamp to signed infinity if overflowed

        o.u = static_cast<u16>(f.u >> 13); // Take the bits!
    }

    o.u |= sign >> 16;
    return o.u;
}
//----------------------------------------------------------------------------
#if 0
FORCE_INLINE static __m128i FP32_to_FP16_SSE2_(__m128  value) {
#define SSE_CONST4(name, val) static const __declspec(align(16)) uint name[4] = { (val), (val), (val), (val) }
#define CONST(name) *(const __m128i *)&name
#define CONSTF(name) *(const __m128 *)&name

    SSE_CONST4(mask_sign,           0x80000000u);
    SSE_CONST4(mask_round,          ~0xfffu);
    SSE_CONST4(c_f32infty,          255 << 23);
    SSE_CONST4(c_magic,             15 << 23);
    SSE_CONST4(c_nanbit,            0x200);
    SSE_CONST4(c_infty_as_fp16,     0x7c00);
    SSE_CONST4(c_clamp,             (31 << 23) - 0x1000);

    __m128  msign       = CONSTF(mask_sign);
    __m128  justsign    = _mm_and_ps(msign, f);
    __m128i f32infty    = CONST(c_f32infty);
    __m128  absf        = _mm_xor_ps(f, justsign);
    __m128  mround      = CONSTF(mask_round);
    __m128i absf_int    = _mm_castps_si128(absf); // pseudo-op, but val needs to be copied once so count as mov
    __m128i b_isnan     = _mm_cmpgt_epi32(absf_int, f32infty);
    __m128i b_isnormal  = _mm_cmpgt_epi32(f32infty, _mm_castps_si128(absf));
    __m128i nanbit      = _mm_and_si128(b_isnan, CONST(c_nanbit));
    __m128i inf_or_nan  = _mm_or_si128(nanbit, CONST(c_infty_as_fp16));

    __m128  fnosticky   = _mm_and_ps(absf, mround);
    __m128  scaled      = _mm_mul_ps(fnosticky, CONSTF(c_magic));
    __m128  clamped     = _mm_min_ps(scaled, CONSTF(c_clamp)); // logically, we want PMINSD on "biased", but this should gen better code
    __m128i biased      = _mm_sub_epi32(_mm_castps_si128(clamped), _mm_castps_si128(mround));
    __m128i shifted     = _mm_srli_epi32(biased, 13);
    __m128i normal      = _mm_and_si128(shifted, b_isnormal);
    __m128i not_normal  = _mm_andnot_si128(b_isnormal, inf_or_nan);
    __m128i joined      = _mm_or_si128(normal, not_normal);

    __m128i sign_shift  = _mm_srli_epi32(_mm_castps_si128(justsign), 16);
    __m128i final       = _mm_or_si128(joined, sign_shift);

    // ~20 SSE2 ops
    return final;

#undef SSE_CONST4
#undef CONST
#undef CONSTF
}
#endif
//----------------------------------------------------------------------------
bool FHalfFloat::IsConvertible(float value) {
    Assert(std::isfinite(value));
    return  MinValue().Unpack() <= value &&
            MaxValue().Unpack() >= value;
}
//----------------------------------------------------------------------------
FHalfFloat Lerp(const FHalfFloat v0, const FHalfFloat v1, float f) {
    return Lerp(v0.Unpack(), v1.Unpack(), f);
}
//----------------------------------------------------------------------------
FHalfFloat BarycentricLerp(const FHalfFloat v0, const FHalfFloat v1, const FHalfFloat v2, float f0, float f1, float f2) {
    return BarycentricLerp(v0.Unpack(), v1.Unpack(), v2.Unpack(), f0, f1, f2);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(byten) == sizeof(i8));
STATIC_ASSERT(sizeof(ubyten) == sizeof(u8));
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(shortn) == sizeof(i16));
STATIC_ASSERT(sizeof(ushortn) == sizeof(u16));
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(wordn) == sizeof(i32));
STATIC_ASSERT(sizeof(uwordn) == sizeof(u32));
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
