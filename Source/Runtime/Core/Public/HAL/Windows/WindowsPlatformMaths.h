#pragma once

#include "HAL/Generic/GenericPlatformMaths.h"

#ifdef PLATFORM_WINDOWS

#include <cmath>
#include <immintrin.h>
#include <xmmintrin.h>
#include <intrin.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformMaths : FGenericPlatformMaths {
public:

    //------------------------------------------------------------------------
    // Float helpers

    static FORCE_INLINE i32 CeilToInt(float f) NOEXCEPT {
        // Note: the x2 is to workaround the rounding-to-nearest-even-number issue when the fraction is .5
        return (-(::_mm_cvt_ss2si(::_mm_set_ss(-0.5f - (f + f))) >> 1));
    }

    static FORCE_INLINE float CeilToFloat(float f) NOEXCEPT {
        return float(CeilToInt(f));
    }

    static FORCE_INLINE i32 FloorToInt(float f) NOEXCEPT {
        // Note: the x2 is to workaround the rounding-to-nearest-even-number issue when the fraction is .5
        return (::_mm_cvt_ss2si(::_mm_set_ss(f + f - 0.5f)) >> 1);
    }

    static FORCE_INLINE float FloorToFloat(float f) NOEXCEPT {
        return float(FloorToInt(f));
    }

    static FORCE_INLINE i32 RoundToInt(float f) NOEXCEPT {
        // Note: the x2 is to workaround the rounding-to-nearest-even-number issue when the fraction is .5
        return (::_mm_cvt_ss2si(::_mm_set_ss(f + f + 0.5f)) >> 1);
    }

    static FORCE_INLINE float RoundToFloat(float f) NOEXCEPT {
        return float(RoundToInt(f));
    }

    static FORCE_INLINE i32 TruncToInt(float f) NOEXCEPT {
        return ::_mm_cvtt_ss2si(::_mm_set_ss(f));
    }

    static FORCE_INLINE float TruncToFloat(float f) NOEXCEPT {
        return float(TruncToInt(f));
    }

    static FORCE_INLINE float Fractional(float f) NOEXCEPT {
        return (f - TruncToFloat(f)); // ]0,1] or [-1,0[ according to sign of f
    }

    static FORCE_INLINE float Frac(float f) NOEXCEPT {
        return (f - FloorToFloat(f)); // always ]0,1]
    }

    using FGenericPlatformMaths::Modf;

    static FORCE_INLINE float Fmod(float x, float y) NOEXCEPT {
        AssertRelease(::fabsf(y) > 1.e-8f);

        float i = y * TruncToFloat(x / y);

        // Rounding and imprecision could cause IntPortion to exceed X and cause the result to be outside the expected range.
        // For example Fmod(55.8, 9.3) would result in a very small negative value!
        if (::fabsf(i) > ::fabsf(x))
            i = x;

        return (x - i);
    }

    //------------------------------------------------------------------------
    // Double helpers

    using FGenericPlatformMaths::CeilToDouble;
    using FGenericPlatformMaths::FloorToDouble;
    using FGenericPlatformMaths::RoundToDouble;
    using FGenericPlatformMaths::TruncToDouble;

    //-----------------------------------------------------------------------
    // Computes absolute value in a generic way


    using FGenericPlatformMaths::Abs;
    using FGenericPlatformMaths::Sign;
    using FGenericPlatformMaths::Max;
    using FGenericPlatformMaths::Max3;
    using FGenericPlatformMaths::Min;
    using FGenericPlatformMaths::Min3;

    //------------------------------------------------------------------------
    // arithmetics

    using FGenericPlatformMaths::Sqrt;
    using FGenericPlatformMaths::Pow;
    using FGenericPlatformMaths::RSqrt;
    using FGenericPlatformMaths::RSqrt_Low;
    using FGenericPlatformMaths::Exp;
    using FGenericPlatformMaths::Exp2;
    using FGenericPlatformMaths::Loge;
    using FGenericPlatformMaths::LogX;
    using FGenericPlatformMaths::Log2;

    //------------------------------------------------------------------------
    // using _BitScanReverse

    using FGenericPlatformMaths::IsPow2;

    static FORCE_INLINE u32 FloorLog2(u32 v) NOEXCEPT {
        // Use BSR to return the log2 of the integer
        unsigned long log2;
        if (::_BitScanReverse(&log2, v) != 0) {
            return log2;
        }
        return 0;
    }

    static FORCE_INLINE u64 FloorLog2(u64 v) NOEXCEPT {
#ifdef ARCH_X64 // _BitScanReverse64 is not available on x86
        // Use BSR to return the log2 of the integer
        unsigned long log2;
        if (::_BitScanReverse64(&log2, v) != 0) {
            return log2;
        }
        return 0;
#else
        return FGenericPlatformMaths::FloorLog2(v);
#endif
    }

    static FORCE_INLINE u32 CeilLog2(u32 v) NOEXCEPT {
        return (FloorLog2(v) + u32(!IsPow2(v)));
    }

    static FORCE_INLINE u64 CeilLog2(u64 v) NOEXCEPT {
        return (FloorLog2(v) + u64(!IsPow2(v)));
    }

    static FORCE_INLINE u32 NextPow2(u32 v) NOEXCEPT {
        return (u32(1) << CeilLog2(v));
    }

    static FORCE_INLINE u64 NextPow2(u64 v) NOEXCEPT {
        return (u64(1) << CeilLog2(v));
    }

    static FORCE_INLINE u32 PrevOrEqualPow2(u32 value) NOEXCEPT {
        return (u32(1) << FloorLog2(value));
    }

    static FORCE_INLINE u64 PrevOrEqualPow2(u64 value) NOEXCEPT {
        return (u64(1) << FloorLog2(value));
    }

    //------------------------------------------------------------------------
    // trigonometry

    using FGenericPlatformMaths::Sin;
    using FGenericPlatformMaths::Asin;
    using FGenericPlatformMaths::Sinh;
    using FGenericPlatformMaths::Cos;
    using FGenericPlatformMaths::Acos;
    using FGenericPlatformMaths::Tan;
    using FGenericPlatformMaths::Atan;
    using FGenericPlatformMaths::Atan2;

    //------------------------------------------------------------------------
    // branch-less float comparisons

    using FGenericPlatformMaths::Select;

    //------------------------------------------------------------------------
    // lzcnt:   leading zero count (MSB)
    // tzcnt:   trailing zero count (LSB)
    // popcnt:  number of bits set to 1

    static FORCE_INLINE u32 lzcnt(u32 u) NOEXCEPT { return ::__lzcnt(u); }
#   ifdef __clang__
    static FORCE_INLINE u32 tzcnt(u32 u) NOEXCEPT { return ::__tzcnt_u32(u); }
#   else
    static FORCE_INLINE u32 tzcnt(u32 u) NOEXCEPT { return ::_tzcnt_u32(u); }
#   endif
    static FORCE_INLINE u32 popcnt(u32 u) NOEXCEPT { return ::__popcnt(u); }

#   ifdef ARCH_X64
    static FORCE_INLINE u64 lzcnt(u64 u) NOEXCEPT { return ::__lzcnt64(u); }
#       ifdef __clang__
    static FORCE_INLINE u64 tzcnt(u64 u) NOEXCEPT { return ::__tzcnt_u64(u); }
#       else
    static FORCE_INLINE u64 tzcnt(u64 u) NOEXCEPT { return ::_tzcnt_u64(u); }
#       endif
    static FORCE_INLINE u64 popcnt(u64 u) NOEXCEPT { return ::__popcnt64(u); }
#   endif

    static u64 tzcnt64(u64 u) NOEXCEPT {
#   ifdef ARCH_X64
        return _tzcnt_u64(u);
#   else
        return (Unlikely(0 == u32(u))
            ? 32 + ::_tzcnt_u32(u32(u >> 32))
            : ::_tzcnt_u32(u32(u)) );
#   endif
    }

    //------------------------------------------------------------------------
    // Bit Scan Reverse

    static FORCE_INLINE void bsr(u32* r, u32 v) NOEXCEPT {
        unsigned long index;
        Verify(::_BitScanReverse(&index, v));
        *r = checked_cast<u32>(index);
    }

#   ifdef ARCH_X64
    static FORCE_INLINE void bsr(u32* r, u64 v) NOEXCEPT {
        unsigned long index;
        Verify(::_BitScanReverse64(&index, v));
        *r = checked_cast<u32>(index);
    }
#   endif

    //------------------------------------------------------------------------
    // half float support

    // sadly no intrinsic support on windows, fall back on dummy versions
    // https://software.intel.com/en-us/node/524287
    using FGenericPlatformMaths::FP32_to_FP16;
    using FGenericPlatformMaths::FP16_to_FP32;

public:
    using FGenericPlatformMaths::popcnt_constexpr;
    using FGenericPlatformMaths::popcnt64;

    static u64 popcnt64(u64 v) NOEXCEPT {
#   ifdef ARCH_X64
        return ::__popcnt64(v);
#   else
        return (::__popcnt(u32(v)) + ::__popcnt(u32(v >> 32)));
#   endif
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
