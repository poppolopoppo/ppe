#pragma once

#include "HAL/TargetPlatform.h"

#include <cmath>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_API FGenericPlatformMaths {
public: // must be defined for every platform

    //------------------------------------------------------------------------
    // Float helpers

    static FORCE_INLINE i32 CeilToInt(float f) NOEXCEPT {
        return i32(::ceilf(f));
    }

    static FORCE_INLINE float CeilToFloat(float f) NOEXCEPT {
        return ::ceilf(f);
    }

    static FORCE_INLINE i32 FloorToInt(float f) NOEXCEPT {
        return i32(::floorf(f));
    }

    static FORCE_INLINE float FloorToFloat(float f) NOEXCEPT {
        return ::floorf(f);
    }

    static FORCE_INLINE i32 RoundToInt(float f) NOEXCEPT {
        return i32(::roundf(f));
    }

    static FORCE_INLINE float RoundToFloat(float f) NOEXCEPT {
        return ::roundf(f);
    }

    static FORCE_INLINE i32 TruncToInt(float f) NOEXCEPT {
        return i32(::truncf(f));
    }

    static FORCE_INLINE float TruncToFloat(float f) NOEXCEPT {
        return ::truncf(f);
    }

    static FORCE_INLINE float Fractional(float f) NOEXCEPT {
        return (f - TruncToFloat(f)); // ]0,1] or [-1,0[ according to sign of f
    }

    static FORCE_INLINE float Frac(float f) NOEXCEPT {
        return (f - FloorToFloat(f)); // always ]0,1]
    }

    static FORCE_INLINE float Modf(const float f, float* intPart) NOEXCEPT {
        return ::modff(f, intPart);
    }

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

    static FORCE_INLINE double CeilToDouble(double d) NOEXCEPT {
        return ::ceil(d);
    }

    static FORCE_INLINE double FloorToDouble(double d) NOEXCEPT {
        return ::floor(d);
    }

    static FORCE_INLINE double RoundToDouble(double d) NOEXCEPT {
        return ::round(d);
    }

    static FORCE_INLINE double TruncToDouble(double d) NOEXCEPT {
        return ::trunc(d);
    }

    static FORCE_INLINE double Modf(const double f, double* intPart) NOEXCEPT {
        return ::modf(f, intPart);
    }


    //-----------------------------------------------------------------------
    // Computes absolute value in a generic way

    template< class T >
    static CONSTEXPR FORCE_INLINE T Abs(const T A) NOEXCEPT {
        return (A < T(0) ? -A : A);
    }

    //-----------------------------------------------------------------------
    // Returns 1, 0, or -1 depending on relation of T to 0 */

    template< class T >
    static CONSTEXPR FORCE_INLINE T Sign(const T A) NOEXCEPT {
        return (A < T(0) ? T(-1) : (T(0) < A ? T(1) : T(0)));
    }

    //-----------------------------------------------------------------------
    // Returns higher value in a generic way

    template< class T >
    static CONSTEXPR FORCE_INLINE T Max(const T A, const T B) NOEXCEPT {
        return (A < B ? B : A);
    }

    template< class T >
    static CONSTEXPR FORCE_INLINE T Max3(const T A, const T B, const T C) NOEXCEPT {
        return Max(A, Max(B, C));
    }

    //-----------------------------------------------------------------------
    // Returns lower value in a generic way

    template< class T >
    static CONSTEXPR FORCE_INLINE T Min(const T A, const T B) NOEXCEPT {
        return (A < B ? A : B);
    }

    template< class T >
    static CONSTEXPR FORCE_INLINE T Min3(const T A, const T B, const T C) NOEXCEPT {
        return Min(A, Min(B, C));
    }

    //------------------------------------------------------------------------
    // arithmetics

    static FORCE_INLINE float Sqrt(float f) { return ::sqrtf(f); }
    static FORCE_INLINE float Pow(float a, float b) { return ::powf(a, b); }

    static FORCE_INLINE float RSqrt(float F) { return 1.0f / ::sqrtf(F); }
    static FORCE_INLINE float RSqrt_Low(float F) { return 1.0f / ::sqrtf(F); }

    static FORCE_INLINE float Exp(float f) { return ::expf(f); } // Returns e^f
    static FORCE_INLINE float Exp2(float f) { return ::powf(2.f, f); /*exp2f(f);*/ } // Returns 2^f
    static FORCE_INLINE float Loge(float f) { return ::logf(f); }
    static FORCE_INLINE float LogX(float base, float f) { return Loge(f) / Loge(base); }
    static FORCE_INLINE float Log2(float f) { return Loge(f) * 1.4426950f; } // 1.0 / Loge(2) = 1.4426950f

    //------------------------------------------------------------------------
    // see http://codinggorilla.domemtech.com/?p=81 or http://en.wikipedia.org/wiki/Binary_logarithm
    // but modified to return 0 for a input value of 0, 686ms on test data

    static FORCE_INLINE CONSTEXPR bool IsPow2(u32 v) { return (v && !(v & (v - 1))); }
    static FORCE_INLINE CONSTEXPR bool IsPow2(u64 v) { return (v && !(v & (v - 1))); }

    static FORCE_INLINE CONSTEXPR u32 FloorLog2(u32 v, u32 pos = 0) {
        if (v >= 1 << 16) { v >>= 16; pos += 16; }
        if (v >= 1 << 8) { v >>= 8; pos += 8; }
        if (v >= 1 << 4) { v >>= 4; pos += 4; }
        if (v >= 1 << 2) { v >>= 2; pos += 2; }
        if (v >= 1 << 1) { pos += 1; }
        return (v == 0) ? 0 : pos;
    }

    static FORCE_INLINE CONSTEXPR u64 FloorLog2(u64 v, u64 pos = 0) {
        if (v >= 1ull << 32) { v >>= 32; pos += 32; }
        if (v >= 1ull << 16) { v >>= 16; pos += 16; }
        if (v >= 1ull << 8) { v >>= 8; pos += 8; }
        if (v >= 1ull << 4) { v >>= 4; pos += 4; }
        if (v >= 1ull << 2) { v >>= 2; pos += 2; }
        if (v >= 1ull << 1) { pos += 1; }
        return (v == 0) ? 0 : pos;
    }

    static FORCE_INLINE CONSTEXPR u32 CeilLog2(u32 v) {
        return (FloorLog2(v) + u32(!IsPow2(v)));
    }

    static FORCE_INLINE CONSTEXPR u64 CeilLog2(u64 v) {
        return (FloorLog2(v) + u64(!IsPow2(v)));
    }

    static FORCE_INLINE CONSTEXPR u32 NextPow2(u32 v) {
        return (u32(1) << CeilLog2(v));
    }

    static FORCE_INLINE CONSTEXPR u64 NextPow2(u64 v) {
        return (u64(1) << CeilLog2(v));
    }

    static FORCE_INLINE CONSTEXPR u32 PrevOrEqualPow2(u32 value) {
        return (u32(1) << FloorLog2(value));
    }

    static FORCE_INLINE CONSTEXPR u64 PrevOrEqualPow2(u64 value) {
        return (u64(1) << FloorLog2(value));
    }

    //------------------------------------------------------------------------
    // trigonometry

    static FORCE_INLINE float Sin(float f) { return ::sinf(f); }
    static FORCE_INLINE float Asin(float f) { return ::asinf((f < -1.f) ? -1.f : ((f < 1.f) ? f : 1.f)); }
    static FORCE_INLINE float Sinh(float f) { return ::sinhf(f); }
    static FORCE_INLINE float Cos(float f) { return ::cosf(f); }
    static FORCE_INLINE float Acos(float f) { return ::acosf((f < -1.f) ? -1.f : ((f < 1.f) ? f : 1.f)); }
    static FORCE_INLINE float Tan(float f) { return ::tanf(f); }
    static FORCE_INLINE float Atan(float f) { return ::atanf(f); }
    static FORCE_INLINE float Atan2(float y, float x) { return ::atan2f(y, x); }

    //------------------------------------------------------------------------
    // branch-less float comparisons

    static FORCE_INLINE float Select(float cmp, float geZero, float ltZero) {
        return (cmp < 0.f ? ltZero : geZero);
    }

    static FORCE_INLINE double Select(double cmp, double geZero, double ltZero) {
        return (cmp < 0.0 ? ltZero : geZero);
    }

    //------------------------------------------------------------------------
    // lzcnt:   leading zero count (MSB)
    // tzcnt:   trailing zero count (LSB)
    // popcnt:  number of bits set to 1

    static u32 lzcnt(u32 u) NOEXCEPT = delete;
    static u32 tzcnt(u32 u) NOEXCEPT = delete;
    static u32 popcnt(u32 u) NOEXCEPT = delete;

#ifdef ARCH_X64
    static u64 lzcnt(u64 u) NOEXCEPT = delete;
    static u64 tzcnt(u64 u) NOEXCEPT = delete;
    static u64 popcnt(u64 u) NOEXCEPT = delete;
#endif

    //------------------------------------------------------------------------
    // Bit Scan Reverse

    static void bsr(u32* r, u32 v) = delete;

#ifdef ARCH_X64
    static void bsr(u32* r, u64 v) = delete;
#endif

    //------------------------------------------------------------------------
    // half float support

    static u16 FP32_to_FP16(float f);
    static float FP16_to_FP32(u16 u);

public: // generic helpers

    //------------------------------------------------------------------------
    // https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel

    template <typename T>
    static FORCE_INLINE CONSTEXPR T popcnt_constexpr(T v) NOEXCEPT {
        v = v - ((v >> 1) & (T)~(T)0 / 3);                                // temp
        v = (v & (u64)~(u64)0 / 15 * 3) + ((v >> 2) & (T)~(T)0 / 15 * 3);       // temp
        v = (v + (v >> 4)) & (T)~(T)0 / 255 * 15;                           // temp
        return (T)(v * ((T)~(T)0 / 255)) >> (sizeof(T) - 1) * CHAR_BIT;   // count
    }

    //-----------------------------------------------------------------------
    // number of bits set to one (support u64 on ARCH_X86)

    template <typename T>
    static FORCE_INLINE u64 popcnt64(T v) NOEXCEPT {
#ifdef ARCH_X86
        return popcnt_constexpr(static_cast<u64>(v));
#else
        return FPlatformMaths::popcnt(static_cast<u64>(v));
#endif
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
