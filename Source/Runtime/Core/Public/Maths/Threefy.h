#pragma once

#include "Core_fwd.h"

#include "Maths/MathHelpers.h"
#include "Maths/ScalarVector.h"
#include "Memory/MemoryView.h"

// open sourced threefy random generator kernel from DE Shaw Research
// https://github.com/apache/incubator-singa/blob/master/src/core/tensor/distribution.cl

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FThreefy_4x32 {
    struct u32x4_t {
        u32 v[4];
    };

    enum r123_enum_threefry32x4 {
        R_32x4_0_0 = 10,
        R_32x4_0_1 = 26,
        R_32x4_1_0 = 11,
        R_32x4_1_1 = 21,
        R_32x4_2_0 = 13,
        R_32x4_2_1 = 27,
        R_32x4_3_0 = 23,
        R_32x4_3_1 = 5,
        R_32x4_4_0 = 6,
        R_32x4_4_1 = 20,
        R_32x4_5_0 = 17,
        R_32x4_5_1 = 11,
        R_32x4_6_0 = 25,
        R_32x4_6_1 = 10,
        R_32x4_7_0 = 18,
        R_32x4_7_1 = 20
    };

    using counter_t = u32x4_t;
    using key_t = u32x4_t;
    using ukey_t = u32x4_t;

    counter_t State{ { 0 } };

    STATIC_CONST_INTEGRAL(size_t, NumRounds, 20);

    CONSTEXPR void Seed(u32 seed) NOEXCEPT {
        State = { { seed, seed, seed, seed } };
    }
    void RandomSeed() NOEXCEPT {
        State = { { u32(::rand()), u32(::rand()), u32(::rand()), u32(::rand()) } };
    }

    template <size_t _NRounds = NumRounds>
    CONSTEXPR float4 Bernoulli(float inf, float sup, float threshold) NOEXCEPT;
    template <size_t _NRounds = NumRounds>
    CONSTEXPR float4 UniformF(float inf, float sup) NOEXCEPT;
    template <size_t _NRounds = NumRounds>
    CONSTEXPR u324 UniformI(u32 inf, u32 sup) NOEXCEPT;
    template <size_t _NRounds = NumRounds>
    CONSTEXPR float4 Gaussian(float E, float V) NOEXCEPT;

    CONSTEXPR u32 operator ()() NOEXCEPT;
    CONSTEXPR u32 operator ()(u32 sup) NOEXCEPT;
    CONSTEXPR u32 operator ()(u32 inf, u32 sup) NOEXCEPT;

    template <typename T>
    T& RandomElement(const TMemoryView<T>& view) {
        Assert(not view.empty());
        return view[UniformI<NumRounds>(0, checked_cast<u32>(view.size())).x];
    }

    template <typename T>
    void Shuffle(const TMemoryView<T>& view) {
        const u32 n = checked_cast<u32>(view.size());
        const u32 a = n - (n & 3);

        using std::swap;

        forrange(i, 0, a/4) {
            const u32 off = (i * 4);
            const u324 target = UniformI<NumRounds>(0, off + 4);
            if (off + 0 != target.x) swap(view[off + 0], view[target.x]);
            if (off + 1 != target.y) swap(view[off + 1], view[target.y]);
            if (off + 2 != target.z) swap(view[off + 2], view[target.z]);
            if (off + 3 != target.w) swap(view[off + 3], view[target.w]);
        }

        if (a < n) {
            const u32 off = a;
            const u324 target = UniformI<NumRounds>(0, n);
            if ((off + 0 < n) & (off + 0 != target.x)) swap(view[off + 0], view[target.x]);
            if ((off + 1 < n) & (off + 1 != target.y)) swap(view[off + 1], view[target.y]);
            if ((off + 2 < n) & (off + 2 != target.z)) swap(view[off + 2], view[target.z]);
            if ((off + 3 < n) & (off + 3 != target.w)) swap(view[off + 3], view[target.w]);
        }
    }

    static CONSTEXPR u32 RotL_32(u32 x, unsigned int N) NOEXCEPT {
        return (x << (N & 31)) | (x >> ((32 - N) & 31));
    }

    template <size_t _NRounds>
    static CONSTEXPR counter_t threefry4x32_R(counter_t in, key_t k) NOEXCEPT;
    template <size_t _NRounds>
    static CONSTEXPR float4 threefry4x32_Bernoulli(counter_t* inout, float inf, float sup, float threshold) NOEXCEPT;
    template <size_t _NRounds>
    static CONSTEXPR float4 threefry4x32_UniformF(counter_t* inout, float inf, float sup) NOEXCEPT;
    template <size_t _NRounds>
    static CONSTEXPR u324 threefry4x32_UniformI(counter_t* inout, u32 inf, u32 sup) NOEXCEPT;
    template <size_t _NRounds>
    static CONSTEXPR float4 threefry4x32_Gaussian(counter_t* inout, float E, float V) NOEXCEPT;

};
//----------------------------------------------------------------------------
template <size_t _NRounds>
inline CONSTEXPR auto FThreefy_4x32::threefry4x32_R(counter_t in, key_t k) NOEXCEPT -> counter_t {
    counter_t X;
    u32 ks[4 + 1] = { 0, 0, 0, 0, 0x1BD11BDAul };

    {
        ks[0] = k.v[0];
        X.v[0] = in.v[0];
        ks[4] ^= k.v[0];

        ks[1] = k.v[1];
        X.v[1] = in.v[1];
        ks[4] ^= k.v[1];

        ks[2] = k.v[2];
        X.v[2] = in.v[2];
        ks[4] ^= k.v[2];

        ks[3] = k.v[3];
        X.v[3] = in.v[3];
        ks[4] ^= k.v[3];
    }

    X.v[0] += ks[0];
    X.v[1] += ks[1];
    X.v[2] += ks[2];
    X.v[3] += ks[3];

    IF_CONSTEXPR(_NRounds > 0) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_0_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_0_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 1) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_1_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_1_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 2) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_2_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_2_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 3) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_3_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_3_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 3) {
        X.v[0] += ks[1];
        X.v[1] += ks[2];
        X.v[2] += ks[3];
        X.v[3] += ks[4];
        X.v[4 - 1] += 1;
    }

    IF_CONSTEXPR(_NRounds > 4) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_4_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_4_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 5) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_5_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_5_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 6) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_6_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_6_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 7) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_7_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_7_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 7) {
        X.v[0] += ks[2];
        X.v[1] += ks[3];
        X.v[2] += ks[4];
        X.v[3] += ks[0];
        X.v[4 - 1] += 2;
    }

    IF_CONSTEXPR(_NRounds > 8) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_0_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_0_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 9) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_1_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_1_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 10) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_2_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_2_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 11) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_3_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_3_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 11) {
        X.v[0] += ks[3];
        X.v[1] += ks[4];
        X.v[2] += ks[0];
        X.v[3] += ks[1];
        X.v[4 - 1] += 3;
    }

    IF_CONSTEXPR(_NRounds > 12) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_4_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_4_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 13) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_5_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_5_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 14) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_6_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_6_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 15) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_7_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_7_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 15) {
        X.v[0] += ks[4];
        X.v[1] += ks[0];
        X.v[2] += ks[1];
        X.v[3] += ks[2];
        X.v[4 - 1] += 4;
    }

    IF_CONSTEXPR(_NRounds > 16) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_0_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_0_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 17) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_1_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_1_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 18) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_2_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_2_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 19) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_3_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_3_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 19) {
        X.v[0] += ks[0];
        X.v[1] += ks[1];
        X.v[2] += ks[2];
        X.v[3] += ks[3];
        X.v[4 - 1] += 5;
    }

    IF_CONSTEXPR(_NRounds > 20) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_4_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_4_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 21) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_5_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_5_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 22) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_6_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_6_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 23) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_7_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_7_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 23) {
        X.v[0] += ks[1];
        X.v[1] += ks[2];
        X.v[2] += ks[3];
        X.v[3] += ks[4];
        X.v[4 - 1] += 6;
    }

    IF_CONSTEXPR(_NRounds > 24) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_0_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_0_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 25) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_1_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_1_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 26) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_2_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_2_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 27) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_3_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_3_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 27) {
        X.v[0] += ks[2];
        X.v[1] += ks[3];
        X.v[2] += ks[4];
        X.v[3] += ks[0];
        X.v[4 - 1] += 7;
    }

    IF_CONSTEXPR(_NRounds > 28) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_4_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_4_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 29) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_5_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_5_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 30) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_6_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_6_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 31) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_7_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_7_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 31) {
        X.v[0] += ks[3];
        X.v[1] += ks[4];
        X.v[2] += ks[0];
        X.v[3] += ks[1];
        X.v[4 - 1] += 8;
    }

    IF_CONSTEXPR(_NRounds > 32) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_0_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_0_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 33) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_1_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_1_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 34) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_2_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_2_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 35) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_3_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_3_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 35) {
        X.v[0] += ks[4];
        X.v[1] += ks[0];
        X.v[2] += ks[1];
        X.v[3] += ks[2];
        X.v[4 - 1] += 9;
    }

    IF_CONSTEXPR(_NRounds > 36) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_4_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_4_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 37) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_5_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_5_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 38) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_6_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_6_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 39) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_7_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_7_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 39) {
        X.v[0] += ks[0];
        X.v[1] += ks[1];
        X.v[2] += ks[2];
        X.v[3] += ks[3];
        X.v[4 - 1] += 10;
    }

    IF_CONSTEXPR(_NRounds > 40) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_0_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_0_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 41) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_1_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_1_1);
        X.v[1] ^= X.v[2];
    }
    IF_CONSTEXPR(_NRounds > 42) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_2_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_2_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 43) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_3_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_3_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 43) {
        X.v[0] += ks[1];
        X.v[1] += ks[2];
        X.v[2] += ks[3];
        X.v[3] += ks[4];
        X.v[4 - 1] += 11;
    }

    IF_CONSTEXPR(_NRounds > 44) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_4_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_4_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 45) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_5_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_5_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 46) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_6_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_6_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 47) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_7_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_7_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 47) {
        X.v[0] += ks[2];
        X.v[1] += ks[3];
        X.v[2] += ks[4];
        X.v[3] += ks[0];
        X.v[4 - 1] += 12;
    }

    IF_CONSTEXPR(_NRounds > 48) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_0_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_0_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 49) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_1_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_1_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 50) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_2_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_2_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 51) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_3_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_3_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 51) {
        X.v[0] += ks[3];
        X.v[1] += ks[4];
        X.v[2] += ks[0];
        X.v[3] += ks[1];
        X.v[4 - 1] += 13;
    }

    IF_CONSTEXPR(_NRounds > 52) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_4_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_4_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 53) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_5_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_5_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 54) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_6_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_6_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 55) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_7_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_7_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 55) {
        X.v[0] += ks[4];
        X.v[1] += ks[0];
        X.v[2] += ks[1];
        X.v[3] += ks[2];
        X.v[4 - 1] += 14;
    }

    IF_CONSTEXPR(_NRounds > 56) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_0_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_0_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 57) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_1_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_1_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 58) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_2_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_2_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 59) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_3_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_3_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 59) {
        X.v[0] += ks[0];
        X.v[1] += ks[1];
        X.v[2] += ks[2];
        X.v[3] += ks[3];
        X.v[4 - 1] += 15;
    }

    IF_CONSTEXPR(_NRounds > 60) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_4_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_4_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 61) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_5_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_5_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 62) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_6_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_6_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 63) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_7_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_7_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 63) {
        X.v[0] += ks[1];
        X.v[1] += ks[2];
        X.v[2] += ks[3];
        X.v[3] += ks[4];
        X.v[4 - 1] += 16;
    }

    IF_CONSTEXPR(_NRounds > 64) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_0_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_0_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 65) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_1_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_1_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 66) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_2_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_2_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 67) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_3_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_3_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 67) {
        X.v[0] += ks[2];
        X.v[1] += ks[3];
        X.v[2] += ks[4];
        X.v[3] += ks[0];
        X.v[4 - 1] += 17;
    }

    IF_CONSTEXPR(_NRounds > 68) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_4_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_4_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 69) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_5_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_5_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 70) {
        X.v[0] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_6_0);
        X.v[1] ^= X.v[0];
        X.v[2] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_6_1);
        X.v[3] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 71) {
        X.v[0] += X.v[3];
        X.v[3] = RotL_32(X.v[3], R_32x4_7_0);
        X.v[3] ^= X.v[0];
        X.v[2] += X.v[1];
        X.v[1] = RotL_32(X.v[1], R_32x4_7_1);
        X.v[1] ^= X.v[2];
    }

    IF_CONSTEXPR(_NRounds > 71) {
        X.v[0] += ks[3];
        X.v[1] += ks[4];
        X.v[2] += ks[0];
        X.v[3] += ks[1];
        X.v[4 - 1] += 18;
    }

    return X;
}
//----------------------------------------------------------------------------
template <size_t _NRounds>
inline CONSTEXPR float4 FThreefy_4x32::threefry4x32_Bernoulli(counter_t* inout, float inf, float sup, float threshold) NOEXCEPT {
    Assert(inout);

    const counter_t ctr = *inout;
    const ukey_t ukey{ { u32(_NRounds), u32(_NRounds), u32(_NRounds), u32(_NRounds) } };
    const counter_t random4 = threefry4x32_R<_NRounds>(ctr, ukey);

    *inout = random4;

    CONSTEXPR const float r = float(UINT32_MAX);
    return float4{
        ( ((((float)random4.v[0]) / r) * (sup - inf) + inf) < threshold ? 1.0f : 0.0f ),
        ( ((((float)random4.v[1]) / r) * (sup - inf) + inf) < threshold ? 1.0f : 0.0f ),
        ( ((((float)random4.v[2]) / r) * (sup - inf) + inf) < threshold ? 1.0f : 0.0f ),
        ( ((((float)random4.v[3]) / r) * (sup - inf) + inf) < threshold ? 1.0f : 0.0f ) };
}
//----------------------------------------------------------------------------
template <size_t _NRounds>
inline CONSTEXPR float4 FThreefy_4x32::threefry4x32_UniformF(counter_t* inout, float inf, float sup) NOEXCEPT {
    Assert(inout);

    const counter_t ctr = *inout;
    const ukey_t ukey{ { u32(_NRounds), u32(_NRounds), u32(_NRounds), u32(_NRounds) } };
    const counter_t random4 = threefry4x32_R<_NRounds>(ctr, ukey);

    *inout = random4;

    CONSTEXPR const float r = float(UINT32_MAX);
    return float4{
        ( (((float)random4.v[0]) / r) * (sup - inf) + inf ),
        ( (((float)random4.v[1]) / r) * (sup - inf) + inf ),
        ( (((float)random4.v[2]) / r) * (sup - inf) + inf ),
        ( (((float)random4.v[3]) / r) * (sup - inf) + inf ) };
}
//----------------------------------------------------------------------------
template <size_t _NRounds>
inline CONSTEXPR u324 FThreefy_4x32::threefry4x32_UniformI(counter_t* inout, u32 inf, u32 sup) NOEXCEPT {
    Assert(inout);

    const counter_t ctr = *inout;
    const ukey_t ukey{ { u32(_NRounds), u32(_NRounds), u32(_NRounds), u32(_NRounds) } };
    const counter_t random4 = threefry4x32_R<_NRounds>(ctr, ukey);

    *inout = random4;

    return u324{
        random4.v[0] % (sup - inf) + inf,
        random4.v[1] % (sup - inf) + inf,
        random4.v[2] % (sup - inf) + inf,
        random4.v[3] % (sup - inf) + inf };
}
//----------------------------------------------------------------------------
template <size_t _NRounds>
inline CONSTEXPR float4 FThreefy_4x32::threefry4x32_Gaussian(counter_t* inout, float e, float V) NOEXCEPT {
    Assert(inout);

    const counter_t ctr = *inout;
    const ukey_t ukey1{ { u32(_NRounds), u32(_NRounds), u32(_NRounds), u32(_NRounds) } };
    const ukey_t ukey2{ { 0, 0, 0, 0 } };

    const counter_t random1 = threefry4x32_R<_NRounds>(ctr, ukey1);
    const counter_t random2 = threefry4x32_R<_NRounds>(ctr, ukey2);

    *inout = random2;

    CONSTEXPR const float r = float(UINT32_MAX);

    const float r1 = (((float)random1.v[0]) / r); // generate a random sequence of uniform distribution
    float r2 = (((float)random2.v[0]) / r);
    const float r3 = (((float)random1.v[1]) / r);
    float r4 = (((float)random2.v[1]) / r);
    const float r5 = (((float)random1.v[2]) / r);
    float r6 = (((float)random2.v[2]) / r);
    const float r7 = (((float)random1.v[3]) / r);
    float r8 = (((float)random2.v[3]) / r);

    if (r2 == 0 || r4 == 0 || r6 == 0 || r8 == 0) {
        r2 += 0.0001f;
        r4 += 0.0001f;
        r6 += 0.0001f;
        r8 += 0.0001f;
    }

    return float4{
        FPlatformMaths::Cos(2.f * PI * r1) * FPlatformMaths::Sqrt(-2.f * FPlatformMaths::Loge(r2)) * V + e ,// return a pseudo sequence of normal distribution using two above uniform noise data
        FPlatformMaths::Cos(2.f * PI * r3) * FPlatformMaths::Sqrt(-2.f * FPlatformMaths::Loge(r4)) * V + e ,// return a pseudo sequence of normal distribution using two above uniform noise data
        FPlatformMaths::Cos(2.f * PI * r5) * FPlatformMaths::Sqrt(-2.f * FPlatformMaths::Loge(r6)) * V + e ,// return a pseudo sequence of normal distribution using two above uniform noise data
        FPlatformMaths::Cos(2.f * PI * r7) * FPlatformMaths::Sqrt(-2.f * FPlatformMaths::Loge(r8)) * V + e };// return a pseudo sequence of normal distribution using two above uniform noise data
}
//----------------------------------------------------------------------------
template <size_t _NRounds>
inline CONSTEXPR float4 FThreefy_4x32::Bernoulli(float inf, float sup, float threshold) NOEXCEPT {
    return threefry4x32_Bernoulli<_NRounds>(&State, inf, sup, threshold);
}
//----------------------------------------------------------------------------
template <size_t _NRounds>
inline CONSTEXPR float4 FThreefy_4x32::UniformF(float inf, float sup) NOEXCEPT {
    return threefry4x32_UniformF<_NRounds>(&State, inf, sup);
}
//----------------------------------------------------------------------------
template <size_t _NRounds>
inline CONSTEXPR u324 FThreefy_4x32::UniformI(u32 inf, u32 sup) NOEXCEPT {
    return threefry4x32_UniformI<_NRounds>(&State, inf, sup);
}
//----------------------------------------------------------------------------
template <size_t _NRounds>
inline CONSTEXPR float4 FThreefy_4x32::Gaussian(float e, float V) NOEXCEPT {
    return threefry4x32_Gaussian<_NRounds>(&State, e, V);
}
//----------------------------------------------------------------------------
inline CONSTEXPR u32 FThreefy_4x32::operator ()() NOEXCEPT {
    const ukey_t ukey{ { u32(NumRounds), u32(NumRounds), u32(NumRounds), u32(NumRounds) } };
    State = threefry4x32_R<NumRounds>(State, ukey);
    return State.v[0];
}
//----------------------------------------------------------------------------
inline CONSTEXPR u32 FThreefy_4x32::operator ()(u32 sup) NOEXCEPT {
    return UniformI<NumRounds>(0, sup).x;
}
//----------------------------------------------------------------------------
inline CONSTEXPR u32 FThreefy_4x32::operator ()(u32 inf, u32 sup) NOEXCEPT {
    return UniformI<NumRounds>(inf, sup).x;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
