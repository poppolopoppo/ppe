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
using m128i_t = ::__m128i;
//----------------------------------------------------------------------------
#define m128i_epi8_tznct_mask() u32(1 << 16)
#define m128i_epi8_broadcast(v) ::_mm_set1_epi8(v)
#define m128i_epi8_set_zero() ::_mm_setzero_si128()
#define m128i_epi8_set_false() ::_mm_setzero_si128()
#if 1
#define m128i_epi8_set_true() ::_mm_cmpeq_epi32(::_mm_setzero_si128(), ::_mm_setzero_si128())
#else
#define m128i_epi8_set_true() m128i_epi8_broadcast(-1)
#endif
#define m128i_epi8_set_true_after(i) ::_mm_cmpgt_epi8(::_mm_set1_epi8(i), ::_mm_set_epi64x(0x0f0e0d0c0b0a0908ULL, 0x0706050403020100ULL))
#if defined(_MSC_VER) && _MSC_VER == 1921 // workaround on MSVC19 for
#   define m128i_epi8_load_aligned(p) ::_mm_lddqu_si128((const m128i_t*)p)
#else
#   define m128i_epi8_load_aligned(p) ::_mm_load_si128((const m128i_t*)p)
#endif
#define m128i_epi8_load_unaligned(p) ::_mm_lddqu_si128((const m128i_t*)p)
#define m128i_epi8_store_aligned(p, v) ::_mm_store_si128((m128i_t*)p, v)
#if 0 // assume SSE4.1 is available for everyone now
#define m128i_epi8_blend(if_false, if_true, m) ::_mm_xor_si128(if_false, ::_mm_and_si128(m, ::_mm_xor_si128(if_true, if_false)))
#else
#define m128i_epi8_blend(if_false, if_true, m) ::_mm_blendv_epi8(if_false, if_true, m)
#endif
#define m128i_epi8_findlt(v, h) u16(::_mm_movemask_epi8(::_mm_cmplt_epi8(v, h)))
#define m128i_epi8_findgt(v, h) u16(::_mm_movemask_epi8(::_mm_cmpgt_epi8(v, h)))
#define m128i_epi8_findge(v, h) u16(::_mm_movemask_epi8(::_mm_or_si128(::_mm_cmpgt_epi8(v, h), ::_mm_cmpeq_epi8(v, h))))
#define m128i_epi8_findeq(v, h) u16(::_mm_movemask_epi8(::_mm_cmpeq_epi8(v, h)))
#define m128i_epi8_findeq_after(v, h, i) u16(::_mm_movemask_epi8(::_mm_and_si128( \
    ::_mm_cmpeq_epi8(v, h), \
    ::_mm_cmpgt_epi8(::_mm_set_epi64x(0x0f0e0d0c0b0a0908ULL, 0x0706050403020100ULL), ::_mm_set1_epi8(i)) )))
#define m128i_epi8_findneq(v, h) u16(::_mm_movemask_epi8(::_mm_cmpeq_epi8(::_mm_cmpeq_epi8(v, h), ::_mm_setzero_si128())))
#define m128i_epi8_findneq_after(v, h, i) u16(::_mm_movemask_epi8(::_mm_andnot_si128( \
    ::_mm_cmpeq_epi8(v, h), \
    ::_mm_cmpgt_epi8(::_mm_set_epi64x(0x0f0e0d0c0b0a0908ULL, 0x0706050403020100ULL), ::_mm_set1_epi8(i)) )))
#define m128i_epi8_findge_after(v, h, i) u16(::_mm_movemask_epi8(::_mm_andnot_si128( \
    ::_mm_cmplt_epi8(v, h), \
    ::_mm_cmpgt_epi8(::_mm_set_epi64x(0x0f0e0d0c0b0a0908ULL, 0x0706050403020100ULL), ::_mm_set1_epi8(i)) )))
//----------------------------------------------------------------------------
FORCE_INLINE m128i_t VECTORCALL m128i_epi8_mask_move(u16 m) NOEXCEPT {
    m128i_t mask = ::_mm_set1_epi16(m);
    mask = ::_mm_shuffle_epi8(mask, ::_mm_set_epi64x(0x0101010101010101ULL, 0x0000000000000000ULL));
    mask = ::_mm_or_si128(mask, ::_mm_set1_epi64x(0x7fbfdfeff7fbfdfeULL));
    mask = ::_mm_cmpeq_epi8(mask, ::_mm_set1_epi64x(-1));
    return mask;
}
//----------------------------------------------------------------------------
FORCE_INLINE m128i_t VECTORCALL m128i_epi8_tzcnt_mask(const m128i_t& mask_epi8) NOEXCEPT {
    const m128i_t shuf_epi64 = ::_mm_set_epi64x(0x0706050403020100ULL, 0x8080808080808080ULL);
    const m128i_t shuf_epi32 = ::_mm_set_epi64x(0x0b0a090880808080ULL, 0x0302010080808080ULL);
    const m128i_t shuf_epi16 = ::_mm_set_epi64x(0x0d0c808009088080ULL, 0x0504808001008080ULL);
    const m128i_t shuf_epi8 = ::_mm_set_epi64x(0x0e800c800a800880ULL, 0x0680048002800080ULL);

    m128i_t nmsk_epi64 = ::_mm_cmpeq_epi64(::_mm_shuffle_epi8(mask_epi8, shuf_epi64), ::_mm_setzero_si128());
    m128i_t nmsk_epi32 = ::_mm_cmpeq_epi32(::_mm_shuffle_epi8(mask_epi8, shuf_epi32), ::_mm_setzero_si128());
    m128i_t nmsk_epi16 = ::_mm_cmpeq_epi16(::_mm_shuffle_epi8(mask_epi8, shuf_epi16), ::_mm_setzero_si128());
    m128i_t nmsk_epi8 = ::_mm_cmpeq_epi8(::_mm_shuffle_epi8(mask_epi8, shuf_epi8), ::_mm_setzero_si128());

    return ::_mm_and_si128(mask_epi8, ::_mm_and_si128(
        ::_mm_and_si128(nmsk_epi64, nmsk_epi32),
        ::_mm_and_si128(nmsk_epi16, nmsk_epi8)));
}
//----------------------------------------------------------------------------
FORCE_INLINE size_t VECTORCALL m128i_epi8_replace_first_assume_unique(m128i_t* st, const m128i_t& hold, const m128i_t& hnew) NOEXCEPT {
    Assert(st);
    m128i_t mask_epi8 = m128i_epi8_tzcnt_mask(::_mm_cmpeq_epi8(*st, hold));
    *st = m128i_epi8_blend(*st, hnew, mask_epi8);
    return FPlatformMaths::tzcnt(m128i_epi8_tznct_mask() | size_t(::_mm_movemask_epi8(mask_epi8)));
}
//----------------------------------------------------------------------------
FORCE_INLINE size_t VECTORCALL m128i_epi8_replace_first_loop(m128i_t* st, m128i_t* visited, const m128i_t& hold, const m128i_t& hnew) NOEXCEPT {
    Assert(st);
    Assert(visited);
    m128i_t mask_epi8 = m128i_epi8_tzcnt_mask(::_mm_andnot_si128(*visited, ::_mm_cmpeq_epi8(*st, hold)));
    *visited = ::_mm_or_si128(*visited, mask_epi8);
    *st = m128i_epi8_blend(*st, hnew, mask_epi8);
    return FPlatformMaths::tzcnt(m128i_epi8_tznct_mask() | size_t(::_mm_movemask_epi8(mask_epi8)));
}
//----------------------------------------------------------------------------
FORCE_INLINE m128i_t VECTORCALL m128i_epi8_remove_at(const m128i_t& st, size_t i) NOEXCEPT {
    m128i_t mask_epi8 = ::_mm_set1_epi16(size_t(1) << i);
    mask_epi8 = ::_mm_shuffle_epi8(mask_epi8, ::_mm_set_epi64x(0x0101010101010101ULL, 0x0000000000000000ULL));
    mask_epi8 = ::_mm_or_si128(mask_epi8, ::_mm_set1_epi64x(0x7fbfdfeff7fbfdfeULL));
    mask_epi8 = ::_mm_cmpeq_epi8(mask_epi8, ::_mm_set1_epi64x(UINT64_MAX));
    return m128i_epi8_blend(st, mask_epi8, mask_epi8);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE