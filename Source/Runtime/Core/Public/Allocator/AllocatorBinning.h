#pragma once

#include "Core_fwd.h"

#include "Maths/SSEHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FAllocatorBinning {
    // made common so that external code can align on block sizes
    static CONSTEXPR const u8 NumBins = 47;
    static CONSTEXPR const u16 BinSizes[NumBins + 1] = {
        16,     32,     48,     64,     80,     96,     112,    128,
        144,    176,    208,    240,    272,    320,    368,    416,
        480,    544,    624,    704,    800,    928,    1056,   1216,
        1424,   1616,   1840,   2112,   2384,   2720,   3184,   3680,
        4240,   4912,   5696,   6432,   7360,   8480,   9552,   10912,
        12736,  15008,  16960,  19104,  21824,  24560,  27632,  INT16_MAX/* padding/dummy */
    };
    static CONSTEXPR const u32 MaxBinSize = BinSizes[NumBins - 1];

    static CONSTEXPR u32 IndexFromSizeConst(u16 blockSize) NOEXCEPT {
        u32 cnt = 0;
        for (u16 sz : BinSizes)
            cnt += (sz < blockSize);
        return cnt;
    }

    static u32 IndexFromSize(size_t blockSize) NOEXCEPT {
        return IndexFromSize(checked_cast<u16>(blockSize));
    }
    static u32 IndexFromSize(u16 blockSize) NOEXCEPT {
#ifdef __clang__ // codegen should out-perform the explicit SSE version bellow
        return IndexFromSizeConst(blockSize);

#else
        m128i_t key = ::_mm_set1_epi16(blockSize);

        m128i_t cnt = ::_mm_setzero_si128();
        cnt = ::_mm_subs_epi16(cnt, ::_mm_cmplt_epi16(::_mm_load_si128((m128i_t*)&BinSizes[ 0]), key));
        cnt = ::_mm_subs_epi16(cnt, ::_mm_cmplt_epi16(::_mm_load_si128((m128i_t*)&BinSizes[ 8]), key));
        cnt = ::_mm_subs_epi16(cnt, ::_mm_cmplt_epi16(::_mm_load_si128((m128i_t*)&BinSizes[16]), key));
        cnt = ::_mm_subs_epi16(cnt, ::_mm_cmplt_epi16(::_mm_load_si128((m128i_t*)&BinSizes[24]), key));
        cnt = ::_mm_subs_epi16(cnt, ::_mm_cmplt_epi16(::_mm_load_si128((m128i_t*)&BinSizes[32]), key));
        cnt = ::_mm_subs_epi16(cnt, ::_mm_cmplt_epi16(::_mm_load_si128((m128i_t*)&BinSizes[40]), key));

        cnt = ::_mm_sad_epu8(cnt, ::_mm_setzero_si128());
        cnt = ::_mm_adds_epi16(cnt, _mm_shuffle_epi32(cnt, _MM_SHUFFLE(1, 0, 3, 2)));

        return (::_mm_cvtsi128_si32(cnt) & 0xFFFFu);

#endif
    }

    static CONSTEXPR u32 IndexToBlockSize(u32 poolIndex) NOEXCEPT {
        Assert(poolIndex < NumBins);
        return BinSizes[poolIndex];
    }

    static u32 BoundSizeToBins(u32 sizeInBytes) NOEXCEPT {
        Assert(sizeInBytes <= MaxBinSize);
        return IndexToBlockSize(IndexFromSize(u16(sizeInBytes)));
    }
    static CONSTEXPR u32 LowestBoundIndex(size_t sizeInBytes) NOEXCEPT {
        if (sizeInBytes <= MaxBinSize) {
            u32 pool = IndexFromSize(sizeInBytes);
            if (BinSizes[pool] > sizeInBytes)
                --pool;
            Assert_NoAssume(BinSizes[pool] <= sizeInBytes);
            return pool;
        }
        return (NumBins - 1);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
