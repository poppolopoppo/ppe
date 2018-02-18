#include "stdafx.h"

#include "MipMapChain.h"

#include "FloatImage.h"
#include "Pixmap_fwd.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Color/Color.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/Maths/MathHelpers.h"

namespace Core {
namespace Pixmap {
EXTERN_LOG_CATEGORY(CORE_PIXMAP_API, Pixmap)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static float AlphaTestCoverage_(const FFloatImage* img, float cutoff, float scale = 1.0f) {
    Assert(img);
    Assert(img->Width() > 1);
    Assert(img->Height() > 1);

    const size_t subSamples = 4;
    const float subSamplesOO = 1.0f/subSamples;
    const float halfSubSamplesOO = 0.5f * subSamplesOO;

    size_t visible = 0;
    size_t total = 0;

    forrange(y, 0, img->Height()-1) {
        forrange(x, 0, img->Width()-1) {
            const float samples[4] = {
                Saturate(img->at(x+0, y+0).A * scale),
                Saturate(img->at(x+0, y+1).A * scale),
                Saturate(img->at(x+1, y+0).A * scale),
                Saturate(img->at(x+1, y+1).A * scale)
            };

            forrange(dy, 0, subSamples) {
                const float ddy = dy*subSamplesOO+halfSubSamplesOO;

                const float alpha_x0 = Lerp(samples[0], samples[1], ddy);
                const float alpha_x1 = Lerp(samples[2], samples[3], ddy);

                forrange(dx, 0, subSamples) {
                    const float ddx = dx*subSamplesOO+halfSubSamplesOO;
                    const float alpha = Lerp(alpha_x0, alpha_x1, ddx);

                    if (alpha > cutoff)
                        ++visible;

                    ++total;
                }
            }
        }
    }

    Assert(total > 0);
    const float coverage = visible / float(total);
    Assert(coverage >= 0 && coverage <= 1);

    return coverage;
}
//----------------------------------------------------------------------------
static void ScaleAlphaTestCoverage_(FFloatImage* img, float cutoff, float desiredCoverage) {
    float minAlphaScale = 0.0f;
    float maxAlphaScale = 4.0f;
    float alphaScale = 1.0f;

    const float initialCoverage = AlphaTestCoverage_(img, cutoff, alphaScale);
    float currentCoverage = initialCoverage;

    // Determine desired scale using a binary search. Hardcoded to 20 steps max.
    for (int i = 0; i < 20; i++) {

        if (currentCoverage < desiredCoverage) {
            minAlphaScale = alphaScale;
        }
        else if (currentCoverage > desiredCoverage) {
            maxAlphaScale = alphaScale;
        }
        else {
            break;
        }

        alphaScale = (minAlphaScale + maxAlphaScale) * 0.5f;

        if (alphaScale < 1)
            return; // we don't want to scale down alpha

        currentCoverage = AlphaTestCoverage_(img, cutoff, alphaScale);
    }

    LOG(Pixmap, Info, L"upscaling alpha coverage of a {0}x{1} FFloatImage from {2} to {3}, desired = {4} (x{5})",
        img->Width(), img->Height(),
        initialCoverage, currentCoverage, desiredCoverage, alphaScale );

    forrange(y, 0, img->Height()) {
        const TMemoryView<FFloatImage::color_type> scanline = img->Scanline(y);
        forrange(x, 0, img->Width()) {
            float& alpha = scanline[x].A;
            alpha = Saturate(alpha * alphaScale);
        }
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Pixmap, FMipMapChain, );
//----------------------------------------------------------------------------
FMipMapChain::FMipMapChain() {}
//----------------------------------------------------------------------------
FMipMapChain::~FMipMapChain() {}
//----------------------------------------------------------------------------
void FMipMapChain::Generate(FFloatImage* topMip, bool hq/* = false */) {
    Assert(topMip);
    Assert( topMip->Width() >= 4 &&
            topMip->Height() >= 4 );

    const size_t mipCount = Meta::FloorLog2(Min(topMip->Width(), topMip->Height())) - 1;

    LOG(Pixmap, Info, L"generate {2} mips from a FFloatImage {0}x{1}",
        topMip->Width(), topMip->Height(),
        mipCount );

    _chain.clear();
    _chain.Push(topMip);

    const FFloatImage* src = topMip;

    size_t w = src->Width();
    size_t h = src->Height();

    while (w > 4 && h > 4) {
        w = w>>1;
        h = h>>1;

        PFloatImage dst = new FFloatImage(w, h);
        Resize(dst.get(), src);

        if (false == hq)
            src = dst.get();

        _chain.Push(std::move(dst));
    }

    Assert(_chain.size() == mipCount);
}
//----------------------------------------------------------------------------
void FMipMapChain::PreserveAlphaTestCoverage(float cutoff) {
    if (_chain.size() < 2)
        return;

    const float coverage = AlphaTestCoverage_(_chain[0].get(), cutoff);

    forrange(m, 1, _chain.size())
        ScaleAlphaTestCoverage_(_chain[m].get(), cutoff, coverage);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
