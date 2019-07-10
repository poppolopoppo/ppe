#pragma once

#include "Core_fwd.h"

#include "HAL/PlatformMaths.h"

#include <emmintrin.h>
#include <immintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct ALIGN(16) FByteMask16 {
    using m128i_t = ::__m128i;
    using u8_16_t = Meta::TArray<u8, 16>;

    STATIC_CONST_INTEGRAL(u32, Capacity, 16);
    STATIC_CONST_INTEGRAL(u32, TZCntMask, u32(1 << 16));

    m128i_t xmm;

    void VECTORCALL broadcast(u8 v) NOEXCEPT {
        xmm = ::_mm_set1_epi8(v);
    }
    void VECTORCALL setzero() NOEXCEPT {
        xmm = ::_mm_setzero_si128();
    }

    void VECTORCALL set_false() NOEXCEPT {
        xmm = ::_mm_setzero_si128();
    }
    void VECTORCALL set_true() NOEXCEPT {
        xmm = ::_mm_cmpeq_epi32(::_mm_setzero_si128(), ::_mm_setzero_si128());
    }
    void VECTORCALL set_true_after(i8 i) NOEXCEPT {
        xmm = ::_mm_cmpgt_epi8(::_mm_set1_epi8(i), ::_mm_set_epi64x(0x0f0e0d0c0b0a0908ULL, 0x0706050403020100ULL));
    }

    void VECTORCALL load(const void* src) NOEXCEPT {
        xmm = ::_mm_load_si128((const m128i_t*)src);
    }
    void VECTORCALL store(void* dst) const NOEXCEPT {
        ::_mm_store_si128((m128i_t*)dst, xmm);
    }

    FByteMask16 VECTORCALL cmpeq(FByteMask16 other) const NOEXCEPT {
        return { ::_mm_cmpeq_epi8(xmm, other.xmm) };
    }

    u16 VECTORCALL move_mask() const NOEXCEPT {
        return u16(::_mm_movemask_epi8(xmm));
    }

    void VECTORCALL mask_move(u16 m) NOEXCEPT {
        xmm = ::_mm_set1_epi16(m);
        xmm = ::_mm_shuffle_epi8(xmm, ::_mm_set_epi64x(0x0101010101010101ULL, 0x0000000000000000ULL));
        xmm = ::_mm_or_si128(xmm, ::_mm_set1_epi64x(0x7fbfdfeff7fbfdfeULL));
        xmm = ::_mm_cmpeq_epi8(xmm, ::_mm_set1_epi64x(-1));
    }

    FByteMask16 VECTORCALL select(FByteMask16 if_true, FByteMask16 if_false) const NOEXCEPT {
#if 0
        return { ::_mm_xor_si128(if_false.xmm, ::_mm_and_si128(xmm, ::_mm_xor_si128(if_true.xmm, if_false.xmm))) };
#else
        return { ::_mm_blendv_epi8(if_false.xmm, if_true.xmm, xmm) };
#endif
    }

    m128i_t VECTORCALL tzcnt_mask(m128i_t mask_epi8) {
        const m128i_t shuf_epi64 = ::_mm_set_epi64x(0x0706050403020100ULL, 0x8080808080808080ULL);
        const m128i_t shuf_epi32 = ::_mm_set_epi64x(0x0b0a090880808080ULL, 0x0302010080808080ULL);
        const m128i_t shuf_epi16 = ::_mm_set_epi64x(0x0d0c808009088080ULL, 0x0504808001008080ULL);
        const m128i_t shuf_epi8  = ::_mm_set_epi64x(0x0e800c800a800880ULL, 0x0680048002800080ULL);

        m128i_t nmsk_epi64 = ::_mm_cmpeq_epi64(::_mm_shuffle_epi8(mask_epi8, shuf_epi64), ::_mm_setzero_si128());
        m128i_t nmsk_epi32 = ::_mm_cmpeq_epi32(::_mm_shuffle_epi8(mask_epi8, shuf_epi32), ::_mm_setzero_si128());
        m128i_t nmsk_epi16 = ::_mm_cmpeq_epi16(::_mm_shuffle_epi8(mask_epi8, shuf_epi16), ::_mm_setzero_si128());
        m128i_t nmsk_epi8  = ::_mm_cmpeq_epi8 (::_mm_shuffle_epi8(mask_epi8, shuf_epi8 ), ::_mm_setzero_si128());

        return ::_mm_and_si128(mask_epi8, ::_mm_and_si128(
            ::_mm_and_si128(nmsk_epi64, nmsk_epi32),
            ::_mm_and_si128(nmsk_epi16, nmsk_epi8 )) );
    }

    u32 VECTORCALL replace_first_assume_unique(FByteMask16 hold, FByteMask16 hnew) NOEXCEPT {
        m128i_t mask_epi8 = ::_mm_cmpeq_epi8(xmm, hold.xmm);
        mask_epi8 = tzcnt_mask(mask_epi8);

        xmm = ::_mm_blendv_epi8(xmm, hnew.xmm, mask_epi8);

        return FPlatformMaths::tzcnt(TZCntMask | u32(::_mm_movemask_epi8(mask_epi8)));
    }

    u32 VECTORCALL replace_first_loop(FByteMask16& visited, FByteMask16 hold, FByteMask16 hnew) NOEXCEPT {
        m128i_t mask_epi8 = ::_mm_andnot_si128(visited.xmm, ::_mm_cmpeq_epi8(xmm, hold.xmm));
        mask_epi8 = tzcnt_mask(mask_epi8);

        visited.xmm = ::_mm_or_si128(mask_epi8, visited.xmm);
        xmm = ::_mm_blendv_epi8(xmm, hnew.xmm, mask_epi8);

        return FPlatformMaths::tzcnt(TZCntMask | u32(::_mm_movemask_epi8(mask_epi8)));
    }

    u16 VECTORCALL find(FByteMask16 h) const NOEXCEPT {
        return u16(::_mm_movemask_epi8(::_mm_cmpeq_epi8(xmm, h.xmm)));
    }
    u16 VECTORCALL find_after(FByteMask16 h, i8 i) const NOEXCEPT {
        return u16(::_mm_movemask_epi8(::_mm_and_si128(
            ::_mm_cmpeq_epi8(xmm, h.xmm),
            ::_mm_cmpgt_epi8(::_mm_set_epi64x(0x0f0e0d0c0b0a0908ULL, 0x0706050403020100ULL), ::_mm_set1_epi8(i)) )));
    }

    u16 VECTORCALL find_not(FByteMask16 h) const NOEXCEPT {
        return u16(::_mm_movemask_epi8(::_mm_cmpeq_epi8(
            ::_mm_cmpeq_epi8(xmm, h.xmm), ::_mm_setzero_si128()) ));
    }
    u16 VECTORCALL find_not_after(FByteMask16 h, i8 i) const NOEXCEPT {
        const m128i_t after = ::_mm_cmpgt_epi8(::_mm_set_epi64x(0x0f0e0d0c0b0a0908ULL, 0x0706050403020100ULL), ::_mm_set1_epi8(i));
        const m128i_t mskeq = ::_mm_cmpeq_epi8(xmm, h.xmm);
        return u16(::_mm_movemask_epi8(::_mm_andnot_si128(mskeq, after)));
    }

    void VECTORCALL remove_at(u32 i) NOEXCEPT {
        m128i_t mask_epi8 = ::_mm_set1_epi16(u32(1) << i);
        mask_epi8 = ::_mm_shuffle_epi8(mask_epi8, ::_mm_set_epi64x(0x0101010101010101ULL, 0x0000000000000000ULL));
        mask_epi8 = ::_mm_or_si128(mask_epi8, ::_mm_set1_epi64x(0x7fbfdfeff7fbfdfeULL));
        mask_epi8 = ::_mm_cmpeq_epi8(mask_epi8, ::_mm_set1_epi64x(UINT64_MAX));

        xmm = ::_mm_blendv_epi8(xmm, mask_epi8, mask_epi8);
    }

    u16 VECTORCALL clear_move(FByteMask16 hdeleted) NOEXCEPT {
        const u16 bitmask = find_not(hdeleted);
        xmm = hdeleted.xmm;
        return bitmask;
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
