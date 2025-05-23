#pragma once

#include "HAL/TargetPlatform.h"

#include <cmath>
#include <limits.h> // CHAR_BIT
#include <pmmintrin.h> // FP_ASSIST : denormals are zero (DAZ)
#include <xmmintrin.h> // FP_ASSIST : flush to zero (FTZ)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FGenericPlatformMaths {
public: // must be defined for every platform

    //------------------------------------------------------------------------
    // SSE registers

    static void Disable_FP_Assist() NOEXCEPT {
        // "It is strongly recommended to set the flush-to-zero mode unless you have special reasons to use subnormal numbers."
        // From : https://www.agner.org/optimize/optimizing_cpp.pdf
        // Code : https://en.wikipedia.org/wiki/Denormal_number

        // Set Flush-To-Zero mode (FTZ) for FP_ASSIST :
        _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
        // Set Denormals-Are-Zero mode (DAZ) for FP_ASSIST :
        _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
    }

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

    static FORCE_INLINE u32 CeilToUnsigned(float f) {
        return static_cast<u32>(::ceilf(f));
    }

    static FORCE_INLINE u32 FloorToUnsigned(float f) {
        return static_cast<u32>(::floorf(f));
    }

    static FORCE_INLINE u32 RoundToUnsigned(float f) {
        return static_cast<u32>(::roundf(f));
    }

    static FORCE_INLINE u32 TruncToUnsigned(float f) {
        return static_cast<u32>(::truncf(f));
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

    static float Fmod(float x, float y) NOEXCEPT;

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
    // Returns 1, or -1 depending on relation of T to 0 */

    template< class T >
    static CONSTEXPR FORCE_INLINE T SignNotZero(const T A) NOEXCEPT {
        return (A >= T(0) ? T(1) : T(-1));
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

    static FORCE_INLINE double Sqrt(double f) { return ::sqrt(f); }
    static FORCE_INLINE double RSqrt(double f) { return 1.0 / ::sqrt(f); }

    static FORCE_INLINE float Sqrt(float f) { return ::sqrtf(f); }
    static FORCE_INLINE float Pow(float a, float b) { return ::powf(a, b); }

    static FORCE_INLINE float RSqrt(float F) { return 1.0f / ::sqrtf(F); }

    // Heil to the king Carmack ;)
    static FORCE_INLINE float RSqrt_Low(float F) {
        long i;
        float x2, y;
        constexpr float threeHalfs = 1.5f;

        x2 = F * 0.5f;
        y  = F;
        i  = *(long*)&y;
        i  = 0x5f3759df - (i >> 1);
        y  = *(float*)&i;
        y  = y * (threeHalfs - (x2 * y * y)); // 1st iteration
        //y  = y * (threeHalfs - (x2 * y * y)); // 2nd iteration (optional)

        return y;
    }
    static FORCE_INLINE float RSqrt_SSE(float F) {
        float result;
        ::_mm_store_ps(&result, ::_mm_rsqrt_ss(::_mm_load_ss(&F)));
        return result;
    }

    static FORCE_INLINE float Exp(float f) { return ::expf(f); } // Returns e^f
    static FORCE_INLINE float Exp2(float f) { return ::powf(2.f, f); /*exp2f(f);*/ } // Returns 2^f
    static FORCE_INLINE float Log(float f) { return ::logf(f); }
    static FORCE_INLINE float Loge(float f) { return ::logf(f); }
    static FORCE_INLINE float LogX(float base, float f) { return Loge(f) / Loge(base); }
    static FORCE_INLINE float Log2(float f) { return Loge(f) * 1.4426950f; } // 1.0 / Loge(2) = 1.4426950f

    //------------------------------------------------------------------------
    // prime numbers

    static FORCE_INLINE CONSTEXPR bool IsPrime(u32 v) NOEXCEPT {
        if (v == 2 || v == 3)
            return true;

        if (v % 2 == 0 || v % 3 == 0)
            return false;

        for (u32 d = 6; d * d - 2 * d + 1 <= v; d += 6) {
            if (v % (d - 1) == 0)
                return false;

            if (v % (d + 1) == 0)
                return false;
        }

        return true;
    }

    static FORCE_INLINE CONSTEXPR bool IsPrime(u64 v) NOEXCEPT {
        if (v == 2 || v == 3)
            return true;

        if (v % 2 == 0 || v % 3 == 0)
            return false;

        for (u64 d = 6; d * d - 2 * d + 1 <= v; d += 6) {
            if (v % (d - 1) == 0)
                return false;

            if (v % (d + 1) == 0)
                return false;
        }

        return true;
    }

    static CONSTEXPR u32 NextPrime(u32 a) NOEXCEPT {
        while (not IsPrime(++a));
        return a;
    }

    static CONSTEXPR u64 NextPrime(u64 a) NOEXCEPT {
        while (not IsPrime(++a));
        return a;
    }

    //------------------------------------------------------------------------
    // Misc integral bit tweed ling

    static CONSTEXPR u32 ContiguousBits(u32 x) {
        u32 nbits = 0;
        for (; x; x &= x << 1, ++nbits);
        return nbits;
    }
    static CONSTEXPR u32 ContiguousBits(u64 x) {
        u32 nbits = 0;
        for (; x; x &= x << 1, ++nbits);
        return nbits;
    }

    // https://graphics.stanford.edu/~seander/bithacks.html#:~:text=Reverse%20an%20N%2Dbit%20quantity%20in%20parallel%20in%205%20*%20lg(N)%20operations%3A
    static CONSTEXPR u32 ReverseBits(u32 v) NOEXCEPT {
        // swap odd and even bits
        v = ((v >> 1) & 0x55555555u) | ((v & 0x55555555u) << 1);
        // swap consecutive pairs
        v = ((v >> 2) & 0x33333333u) | ((v & 0x33333333u) << 2);
        // swap nibbles ...
        v = ((v >> 4) & 0x0F0F0F0Fu) | ((v & 0x0F0F0F0Fu) << 4);
        // swap bytes
        v = ((v >> 8) & 0x00FF00FFu) | ((v & 0x00FF00FFu) << 8);
        // swap 2-byte long pairs
        v = (v >> 16) | (v << 16);
        return v;
    }
    static CONSTEXPR u64 ReverseBits(u64 v) NOEXCEPT {
        // swap odd and even bits
        v = ((v >>  1) & 0x5555555555555555ull) | ((v & 0x5555555555555555ull) << 1);
        // swap consecutive pairs
        v = ((v >>  2) & 0x3333333333333333ull) | ((v & 0x3333333333333333ull) << 2);
        // swap nibbles ...
        v = ((v >>  4) & 0x0F0F0F0F0F0F0F0Full) | ((v & 0x0F0F0F0F0F0F0F0Full) << 4);
        // swap bytes
        v = ((v >>  8) & 0x00FF00FF00FF00FFull) | ((v & 0x00FF00FF00FF00FFull) << 8);
        // swap 2-byte long pairs
        v = ((v >> 16) & 0x0000FFFF0000FFFFull) | ((v & 0x0000FFFF0000FFFFull) << 16);
        // swap 4-byte long pairs
        v = (v >> 32) | (v << 32);
        return v;
    }

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
    static FORCE_INLINE float Tanh(float f) { return ::tanhf(f); }
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
    // rotations

    static u16 rotl(u16 x, u8 d) {
        return (x << d) | (x >> (sizeof(x)*8 - d));
    }
    static u32 rotl(u32 x, u8 d) {
        return (x << d) | (x >> (sizeof(x)*8 - d));
    }
    static u64 rotl(u64 x, u8 d) {
        return (x << d) | (x >> (sizeof(x)*8 - d));
    }

    static u16 rotr(u16 x, u8 d) {
        return (x >> d) | (x << (sizeof(x)*8 - d));
    }
    static u32 rotr(u32 x, u8 d) {
        return (x >> d) | (x << (sizeof(x)*8 - d));
    }
    static u64 rotr(u64 x, u8 d) {
        return (x >> d) | (x << (sizeof(x)*8 - d));
    }

    //------------------------------------------------------------------------
    // lzcnt:   leading zero count (MSB)
    // tzcnt:   trailing zero count (LSB)
    // popcnt:  number of bits set to 1

    // https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
    template <typename T>
    static FORCE_INLINE CONSTEXPR T popcnt_constexpr(T v) NOEXCEPT {
        STATIC_ASSERT(std::is_arithmetic_v<T>);
        v = v - ((v >> 1) & (T)~(T)0 / 3);                                  // temp
        v = (v & (u64)~(u64)0 / 15 * 3) + ((v >> 2) & (T)~(T)0 / 15 * 3);   // temp
        v = (v + (v >> 4)) & (T)~(T)0 / 255 * 15;                           // temp
        return (T)(v * ((T)~(T)0 / 255)) >> (sizeof(T) - 1) * CHAR_BIT;     // count
    }

    static u32 clz(u32 u) NOEXCEPT = delete;
    static u64 clz(u64 u) NOEXCEPT = delete;

    static u32 ctz(u32 u) NOEXCEPT = delete;
    static u64 ctz(u64 u) NOEXCEPT = delete;

    static u32 lzcnt(u32 u) NOEXCEPT = delete;
    static u32 tzcnt(u32 u) NOEXCEPT = delete;
    static u32 popcnt(u32 u) NOEXCEPT = delete;

    // fallback when using ARCH_X64 :
    static u64 lzcnt(u64 u) NOEXCEPT { return lzcnt_constexpr(u); }
    static u64 tzcnt(u64 u) NOEXCEPT { return tzcnt_constexpr(u); }
    static u64 popcnt(u64 u) NOEXCEPT { return popcnt_constexpr(u); }

    static u64 tzcnt64(u64 u) NOEXCEPT = delete;

    static u64 lzcnt(u128 u) NOEXCEPT = delete;
    static u64 tzcnt(u128 u) NOEXCEPT = delete;
    static u64 popcnt(u128 u) NOEXCEPT = delete;

    // number of bits set to one (support u64 on ARCH_X86)
    template <typename T>
    static u64 popcnt64(T v) NOEXCEPT = delete;

    static CONSTEXPR u32 lzcnt_constexpr(u32 u) NOEXCEPT {
        for (i32 i = 31; i >= 0; --i)
            if (u & u32(u32(1) << i))
                return i;
        return 0;
    }
    static CONSTEXPR u64 lzcnt_constexpr(u64 u) NOEXCEPT {
        for (i32 i = 31; i >= 0; --i)
            if (u & u64(u64(1) << i))
                return i;
        return 0;
    }

    static CONSTEXPR u32 tzcnt_constexpr(u32 u) NOEXCEPT {
        for (u32 i = 0; i < 32; ++i)
            if (u & (u32(1) << i))
                return i;
        return 0;
    }
    static CONSTEXPR u64 tzcnt_constexpr(u64 u) NOEXCEPT {
        for (u32 i = 0; i < 64; ++i)
            if (u & (u64(1) << i))
                return i;
        return 0;
    }

    //------------------------------------------------------------------------
    // Bit Scan Forward

    static bool bsf(u32* r, u32 v) = delete;
#ifdef ARCH_X64
    static bool bsf(u32* r, u64 v) = delete;
#endif

    //------------------------------------------------------------------------
    // Bit Scan Reverse

    static bool bsr(u32* r, u32 v) = delete;
#ifdef ARCH_X64
    static bool bsr(u32* r, u64 v) = delete;
#endif

    //------------------------------------------------------------------------
    // half float support

    static u16 FP32_to_FP16(float f);
    static float FP16_to_FP32(u16 u);

public: // generic helpers

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
