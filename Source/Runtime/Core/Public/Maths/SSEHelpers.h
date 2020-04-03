#pragma once

#include "Core_fwd.h"

#include "HAL/PlatformMaths.h"

#include <emmintrin.h>
#include <immintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>

#define USE_PPE_SSE2   (1) // deployed since Netburst, 2000
#define USE_PPE_SSE4_1 (1) // deployed since Penryn, 2007
#ifdef __AVX2__
#define USE_PPE_AVX2   (1) // deployed since Haswell, 2013
#define USE_PPE_AVX512 (0) // only since high-end Skylake, 2013 (%_NOCOMMIT%)
#else
#define USE_PPE_AVX512 (0)
#endif
namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_SSE2
//----------------------------------------------------------------------------
using mask16_t = u16;
using m128i_t = ::__m128i;
//----------------------------------------------------------------------------
static CONSTEXPR mask16_t m128i_mask_all_ones{ 0xFFFF };
static CONSTEXPR mask16_t m128i_mask_all_zeros{ 0 };
//----------------------------------------------------------------------------
#define m128i_epi8_tznct_mask() u32(1 << 16)
#define m128i_epi8_broadcast(v) ::_mm_set1_epi8((char)v)
#define m128i_epi8_set_zero() ::_mm_setzero_si128()
#define m128i_epi8_set_false() ::_mm_setzero_si128()
#if 1
#   define m128i_epi8_set_true() ::_mm_cmpeq_epi32(::_mm_setzero_si128(), ::_mm_setzero_si128())
#else
#   define m128i_epi8_set_true() m128i_epi8_broadcast(-1)
#endif
#define m128i_epi8_set_true_after(i) ::_mm_cmpgt_epi8(::_mm_set1_epi8(i), ::_mm_set_epi64x(0x0f0e0d0c0b0a0908ULL, 0x0706050403020100ULL))
//----------------------------------------------------------------------------
#define m128i_epi8_load_aligned(p) ::_mm_load_si128((const m128i_t*)(p))
#define m128i_epi8_load_unaligned(p) ::_mm_lddqu_si128((const m128i_t*)(p))
#define m128i_epi8_load_stream(p) ::_mm_stream_load_si128((const m128i_t*)(p))
#define m128i_epi8_loadlo_epi64(p) ::_mm_loadl_epi64((const m128i_t*)(p))
//----------------------------------------------------------------------------
#define m128i_epi8_store_aligned(p, v) ::_mm_store_si128((m128i_t*)(p), v)
#define m128i_epi8_store_stream(p, v) ::_mm_stream_si128((m128i_t*)(p), v)
#define m128i_epi8_storelo_epi64(p, v) ::_mm_storel_epi64((m128i_t*)(p), v)
//----------------------------------------------------------------------------
#if USE_PPE_SSE4_1
#   define m128i_epi8_blend(if_false, if_true, m) ::_mm_blendv_epi8(if_false, if_true, m)
#else
#   define m128i_epi8_blend(if_false, if_true, m) ::_mm_xor_si128(if_false, ::_mm_and_si128(m, ::_mm_xor_si128(if_true, if_false)))
#endif
//----------------------------------------------------------------------------
#define m128i_epi8_and(a, b) ::_mm_and_si128(a, b)
#define m128i_epi8_or(a, b)  ::_mm_or_si128(a, b)
#define m128i_epi8_xor(a, b) ::_mm_xor_si128(a, b)
//----------------------------------------------------------------------------
#define m128i_epi8_cmpeq(a, b) ::_mm_cmpeq_epi8(a, b)
#define m128i_epi8_cmplt(a, b) ::_mm_cmplt_epi8(a, b)
#define m128i_epi8_cmpgt(a, b) ::_mm_cmpgt_epi8(a, b)
#if 1
#   define m128i_epi8_cmple(a, b) ::_mm_cmpeq_epi8(::_mm_min_epi8(a, b), a)
#   define m128i_epi8_cmpge(a, b) m128i_epi8_cmple(b, a)
#else
#   define m128i_epi8_cmple(a, b) ::_mm_or_si128(::_mm_cmplt_epi8(a, b), ::_mm_cmpeq_epi8(a, b))
#   define m128i_epi8_cmpge(a, b) ::_mm_or_si128(::_mm_cmpgt_epi8(a, b), ::_mm_cmpeq_epi8(a, b))
#endif
//----------------------------------------------------------------------------
#define m128i_epi8_movemask(m) (unsigned int)::_mm_movemask_epi8(m)
#define m128i_epi8_maskmoveu(v, m, p) ::_mm_maskmoveu_si128(v, m, (char*)(p))
//----------------------------------------------------------------------------
#define m128i_epi8_findlt(v, h) m128i_epi8_movemask(m128i_epi8_cmplt(v, h))
#define m128i_epi8_findgt(v, h) m128i_epi8_movemask(m128i_epi8_cmpgt(v, h))
#define m128i_epi8_findle(v, h) m128i_epi8_movemask(m128i_epi8_cmple(v, h))
#define m128i_epi8_findge(v, h) m128i_epi8_movemask(m128i_epi8_cmpge(v, h))
#define m128i_epi8_findeq(v, h) m128i_epi8_movemask(m128i_epi8_cmpeq(v, h))
#define m128i_epi8_findneq(v, h) m128i_epi8_movemask(::_mm_cmpeq_epi8(::_mm_cmpeq_epi8(v, h), ::_mm_setzero_si128()))
#define m128i_epi8_findeq_after(v, h, i) m128i_epi8_movemask(::_mm_and_si128( \
    ::_mm_cmpeq_epi8(v, h), \
    ::_mm_cmpgt_epi8(::_mm_set_epi64x(0x0f0e0d0c0b0a0908ULL, 0x0706050403020100ULL), ::_mm_set1_epi8(i)) ))
#define m128i_epi8_findneq_after(v, h, i) m128i_epi8_movemask(::_mm_andnot_si128( \
    ::_mm_cmpeq_epi8(v, h), \
    ::_mm_cmpgt_epi8(::_mm_set_epi64x(0x0f0e0d0c0b0a0908ULL, 0x0706050403020100ULL), ::_mm_set1_epi8(i)) ))
//----------------------------------------------------------------------------
FORCE_INLINE m128i_t VECTORCALL m128i_epi8_mask_move(mask16_t m) NOEXCEPT {
    m128i_t mask = ::_mm_set1_epi16(m);
    mask = ::_mm_shuffle_epi8(mask, ::_mm_set_epi64x(0x0101010101010101ULL, 0x0000000000000000ULL));
    mask = ::_mm_or_si128(mask, ::_mm_set1_epi64x(0x7fbfdfeff7fbfdfeULL));
    mask = ::_mm_cmpeq_epi8(mask, ::_mm_set1_epi64x(-1));
    return mask;
}
//----------------------------------------------------------------------------
FORCE_INLINE m128i_t VECTORCALL m128i_epi8_tzcnt_mask(m128i_t mask_epi8) NOEXCEPT {
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
FORCE_INLINE mask16_t VECTORCALL m128i_epi8_replace_first_assume_unique(m128i_t* st, const m128i_t& hold, const m128i_t& hnew) NOEXCEPT {
    Assert(st);
    m128i_t mask_epi8 = m128i_epi8_tzcnt_mask(::_mm_cmpeq_epi8(*st, hold));
    *st = m128i_epi8_blend(*st, hnew, mask_epi8);
    return mask16_t(FPlatformMaths::tzcnt(m128i_epi8_tznct_mask() | m128i_epi8_movemask(mask_epi8)));
}
//----------------------------------------------------------------------------
FORCE_INLINE mask16_t VECTORCALL m128i_epi8_replace_first_loop(m128i_t* st, m128i_t* visited, const m128i_t& hold, const m128i_t& hnew) NOEXCEPT {
    Assert(st);
    Assert(visited);
    m128i_t mask_epi8 = m128i_epi8_tzcnt_mask(::_mm_andnot_si128(*visited, ::_mm_cmpeq_epi8(*st, hold)));
    *visited = ::_mm_or_si128(*visited, mask_epi8);
    *st = m128i_epi8_blend(*st, hnew, mask_epi8);
    return mask16_t(FPlatformMaths::tzcnt(m128i_epi8_tznct_mask() | m128i_epi8_movemask(mask_epi8)));
}
//----------------------------------------------------------------------------
FORCE_INLINE m128i_t VECTORCALL m128i_epi8_remove_at(m128i_t st, size_t i) NOEXCEPT {
    m128i_t mask_epi8 = ::_mm_set1_epi16(size_t(1) << i);
    mask_epi8 = ::_mm_shuffle_epi8(mask_epi8, ::_mm_set_epi64x(0x0101010101010101ULL, 0x0000000000000000ULL));
    mask_epi8 = ::_mm_or_si128(mask_epi8, ::_mm_set1_epi64x(0x7fbfdfeff7fbfdfeULL));
    mask_epi8 = ::_mm_cmpeq_epi8(mask_epi8, ::_mm_set1_epi64x(UINT64_MAX));
    return m128i_epi8_blend(st, mask_epi8, mask_epi8);
}
//----------------------------------------------------------------------------
#endif //!USE_PPE_SSE2
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_AVX2
//----------------------------------------------------------------------------
using mask32_t = u32;
using m256i_t = ::__m256i;
//----------------------------------------------------------------------------
#define m256i_epi8_tznct_mask() u64(1 << 32)
#define m256i_epi8_broadcast(v) ::_mm256_set1_epi8((char)v)
#define m256i_epi8_set_zero() ::_mm256_setzero_si256()
#define m256i_epi8_set_false() ::_mm256_setzero_si256()
#define m256i_epi8_set_true() ::_mm256_cmpeq_epi32(::_mm256_setzero_si256(), ::_mm256_setzero_si256())
#define m256i_epi8_set_true_after(i) ::_mm256_cmpgt_epi8(::_mm256_set1_epi8(i), \
    ::_mm256_set_epi64x(0x1f1e1d1c1b1a1918ULL, 0x1716151413121110ULL, 0x0f0e0d0c0b0a0908ULL, 0x0706050403020100ULL))
#define m256i_epi8_load_aligned(p) ::_mm256_load_si256((const m256i_t*)(p))
#define m256i_epi8_load_unaligned(p) ::_mm256_lddqu_si256((const m256i_t*)(p))
#define m256i_epi8_store_aligned(p, v) ::_mm256_store_si256((m256i_t*)(p), v)
#define m256i_epi8_blend(if_false, if_true, m) ::_mm256_blendv_epi8(if_false, if_true, m)
#define m256i_epi8_movemask(m) (unsigned int)::_mm256_movemask_epi8(m)
#if USE_PPE_AVX512
#   define m256i_epi8_findlt(v, h) ::_mm256_cmplt_epi8_mask(v, h)
#   define m256i_epi8_findgt(v, h) ::_mm256_cmpgt_epi8_mask(v, h)
#   define m256i_epi8_findge(v, h) ::_mm256_cmpge_epi8_mask(v, h)
#   define m256i_epi8_findeq(v, h) ::_mm256_cmpeq_epi8_mask(v, h)
#else
#   define m256i_epi8_findlt(v, h) m256i_epi8_movemask(::_mm256_cmpgt_epi8(h, v))
#   define m256i_epi8_findgt(v, h) m256i_epi8_movemask(::_mm256_cmpgt_epi8(v, h))
#	if 1
#		define m256i_epi8_findle(v, h) m256i_epi8_movemask(::_mm256_cmpeq_epi8(::_mm256_min_epi8(v, h), v))
#		define m256i_epi8_findge(v, h) m256i_epi8_findle(h, v)
#	else
#		define m256i_epi8_findge(v, h) m256i_epi8_movemask(::_mm256_or_si256(::_mm256_cmpgt_epi8(v, h), ::_mm256_cmpeq_epi8(v, h)))
#	endif
#   define m256i_epi8_findeq(v, h) m256i_epi8_movemask(::_mm256_cmpeq_epi8(v, h))
#endif
//----------------------------------------------------------------------------
#endif //!USE_PPE_AVX2
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE