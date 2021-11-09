#pragma once

#include "HAL/Generic/GenericPlatformMaths.h"

#ifdef PLATFORM_LINUX

#include "Meta/Assert.h"

#include <cmath>
#include <immintrin.h>
#include <x86intrin.h>
#include <xmmintrin.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FLinuxPlatformMaths : FGenericPlatformMaths {
public:

    //------------------------------------------------------------------------
    // SSE registers

    using FGenericPlatformMaths::Disable_FP_Assist;

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

    static FORCE_INLINE u32 CeilToUnsigned(float f) {
        return static_cast<u32>(CeilToInt(f));
    }

    static FORCE_INLINE u32 FloorToUnsigned(float f) {
        return static_cast<u32>(FloorToInt(f));
    }

    static FORCE_INLINE u32 RoundToUnsigned(float f) {
        return static_cast<u32>(RoundToInt(f));
    }

    static FORCE_INLINE u32 TruncToUnsigned(float f) {
        return static_cast<u32>(TruncToInt(f));
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
    // prime numbers

    using FGenericPlatformMaths::IsPrime;
    using FGenericPlatformMaths::NextPrime;

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

    static FORCE_INLINE u32 clz(u32 u) NOEXCEPT {
        if (u == 0) return 32;
        return (u32)__builtin_clz(u);
    }
    static FORCE_INLINE u64 clz(u64 u) NOEXCEPT {
        if (u == 0) return 64;
        return (u64)__builtin_clzll(u);
    }
    static FORCE_INLINE u32 ctz(u32 u) NOEXCEPT {
        if (u == 0) return 32;
        return (u32)__builtin_ctz(u);
    }
    static FORCE_INLINE u64 ctz(u64 u) NOEXCEPT {
        if (u == 0) return 64;
        return (u64)__builtin_ctzll(u);
    }

    static FORCE_INLINE u32 lzcnt(u32 u) NOEXCEPT { return ::__lzcnt32(u); }
    static FORCE_INLINE u32 tzcnt(u32 u) NOEXCEPT { return ::_mm_tzcnt_32(u); }
    static FORCE_INLINE u32 popcnt(u32 u) NOEXCEPT { return ::_popcnt32(u); }

#   ifdef ARCH_X64
    static FORCE_INLINE u64 lzcnt(u64 u) NOEXCEPT { return ::__lzcnt64(u); }
    static FORCE_INLINE u64 tzcnt(u64 u) NOEXCEPT { return ::__tzcnt_u64(u); }
    static FORCE_INLINE u64 popcnt(u64 u) NOEXCEPT { return ::_popcnt64(u); }
#   else
    static FORCE_INLINE u64 lzcnt(u64 u) NOEXCEPT { return FGenericPlatformMaths::lzcnt(u); }
    static FORCE_INLINE u64 tzcnt(u64 u) NOEXCEPT { return FGenericPlatformMaths::tzcnt(u); }
    static FORCE_INLINE u64 popcnt(u64 u) NOEXCEPT { return FGenericPlatformMaths::popcnt(u); }
#   endif

    static u64 ctz(u128 u) NOEXCEPT {
        return (u.lo ? ctz(u.lo) : 64 + ctz(u.hi));
    }
    static u64 lzcnt(u128 u) NOEXCEPT {
        return (u.hi ? lzcnt(u.hi) : 64 + lzcnt(u.lo));
    }
    static u64 tzcnt(u128 u) NOEXCEPT {
        return (u.lo ? tzcnt(u.lo) : 64 + tzcnt(u.hi));
    }
    static u64 popcnt(u128 u) NOEXCEPT {
        return (popcnt(u.lo) + popcnt(u.hi));
    }

    // number of bits set to one (support u64 on ARCH_X86)
    template <typename T>
    static FORCE_INLINE u64 popcnt64(T v) NOEXCEPT {
#ifdef ARCH_X86
        STATIC_ASSERT(sizeof(T) > sizeof(u32));
        return FGenericPlatformMaths::popcnt_constexpr(static_cast<u64>(v));
#else
        return popcnt(static_cast<u64>(v));
#endif
    }

    static u64 tzcnt64(u64 u) NOEXCEPT {
#   ifdef ARCH_X64
        return tzcnt(u);
#   else
        return (Unlikely(0 == u32(u))
            ? 32 + tzcnt(u32(u >> 32))
            : tzcnt(u32(u)) );
#   endif
    }

    //------------------------------------------------------------------------
    // Bit Scan Forward

    static FORCE_INLINE bool bsf(unsigned long* __restrict r, u32 v) NOEXCEPT {
        Assert(r);
        *r = __builtin_ctz(v);
        return true;
    }
    static FORCE_INLINE bool bsf(u32* __restrict r, u32 v) NOEXCEPT {
        Assert(r);
        *r = __builtin_ctz(v);
        return true;
    }

#   ifdef ARCH_X64
    static FORCE_INLINE bool bsf(unsigned long* __restrict r, u64 v) NOEXCEPT {
        Assert(r);
        *r = __builtin_ctz(v);
        return true;
    }
    static FORCE_INLINE bool bsf(u32* __restrict r, u64 v) NOEXCEPT {
        Assert(r);
        *r = __builtin_ctz(v);
        return true;
    }
#   endif

    //------------------------------------------------------------------------
    // Bit Scan Reverse

    static FORCE_INLINE bool bsr(unsigned long* __restrict r, u32 v) NOEXCEPT {
        *r = 31 ^ __builtin_clz(v);
        return (0 != v);
    }
    static FORCE_INLINE bool bsr(u32* __restrict r, u32 v) NOEXCEPT {
        *r = 31 ^ __builtin_clz(v);
        return (0 != v);
    }

#   ifdef ARCH_X64
    static FORCE_INLINE bool bsr(unsigned long* __restrict r, u64 v) NOEXCEPT {
        *r = 63 ^ __builtin_clz(v);
        return (0 != v);
    }
    static FORCE_INLINE bool bsr(u32* __restrict r, u64 v) NOEXCEPT {
        *r = 63 ^ __builtin_clz(v);
        return (0 != v);
    }
#   endif

//------------------------------------------------------------------------
    // using _BitScanReverse

    using FGenericPlatformMaths::IsPow2;

    static FORCE_INLINE u32 FloorLog2(u32 v) NOEXCEPT {
        // Use BSR to return the log2 of the integer
        unsigned long log2;
        if (bsr(&log2, v) != 0) {
            return log2;
        }
        return 0;
    }

    static FORCE_INLINE u64 FloorLog2(u64 v) NOEXCEPT {
#ifdef ARCH_X64 // _BitScanReverse64 is not available on x86
        // Use BSR to return the log2 of the integer
        unsigned long log2;
        if (bsr(&log2, v) != 0) {
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
        return ::_popcnt64(v);
#   else
        return (u64(::_popcnt32(u32(v))) +
                u64(::_popcnt32(u32(v >> 32))) );
#   endif
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
