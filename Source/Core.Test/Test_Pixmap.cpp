#include "stdafx.h"

#include "Core/Color/Color.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Thread/Task.h"

#include "Core.Pixmap/Drawing.h"
#include "Core.Pixmap/CompressedImage.h"
#include "Core.Pixmap/FloatImage.h"
#include "Core.Pixmap/Image.h"
#include "Core.Pixmap/ImageHelpers.h"
#include "Core.Pixmap/MipMapChain.h"

#include "Core/Maths/BinPacking.h"
#include "Core/Maths/ScalarBoundingBox.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/RandomGenerator.h"

#include "Core/IO/Format.h"
#include "Core/IO/FS/ConstNames.h"
#include "Core/IO/String.h"
#include "Core/IO/StringBuilder.h"
#include "Core/IO/FS/Filename.h"

namespace Core {
namespace Test {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
constexpr float AlphaCutoff = 0.33333f;
//----------------------------------------------------------------------------
static void Test_ExpandAlphaMask_(const FFilename& input) {
    const FDirpath output = L"Process:/";

    Pixmap::FImage img;
    if (false == Pixmap::Load(&img, Pixmap::_8bits, Pixmap::sRGB, input))
        AssertNotReached();

    Pixmap::PFloatImage intermediate = new Pixmap::FFloatImage();
    img.ConvertTo(intermediate.get());

    const bool hasAlpha = intermediate->HasAlpha();

    if (hasAlpha) {
        Pixmap::FFloatImage cpy = *intermediate;
        cpy.DiscardAlpha();
        img.ConvertFrom(&cpy);
        const FFilename fname = output / (input.BasenameNoExt() + L"_DiscardAlpha" + FFSConstNames::Png());
        if (false == Pixmap::Save(&img, fname))
            AssertNotReached();
    }

    if (hasAlpha)
        Pixmap::ExpandColorToTransparentPixels(intermediate.get(), AlphaCutoff);

    if (hasAlpha) {
        Pixmap::FFloatImage cpy = *intermediate;
        cpy.DiscardAlpha();
        img.ConvertFrom(&cpy);
        const FFilename fname = output / (input.BasenameNoExt() + L"_ExpandAlphaColor" + FFSConstNames::Png());
        if (false == Pixmap::Save(&img, fname))
            AssertNotReached();
    }

    Pixmap::FMipMapChain mipMaps;
    mipMaps.Generate(intermediate.get(), true);

    if (hasAlpha)
        mipMaps.PreserveAlphaTestCoverage(AlphaCutoff);

    const auto chain = mipMaps.MakeView();
    forrange(i, 0, chain.size()) {
        //chain[i]->DiscardAlpha();

        img.ConvertFrom(chain[i].get());

        const FWString fname = StringFormat(L"{0}/{1}_Mip{2:#2}.png", output, input.BasenameNoExt(), i);
        if (false == Pixmap::Save(&img, fname.MakeView()))
            AssertNotReached();
    }
}
//----------------------------------------------------------------------------
static void Test_DistanceField(const FFilename& input) {
    Pixmap::FImage img;
    if (false == Pixmap::Load(&img, Pixmap::_32bits, Pixmap::sRGB, input))
        AssertNotReached();

    Pixmap::FFloatImage tmp;
    img.ConvertTo(&tmp);

    Pixmap::DistanceField_DRA(&img, &tmp, AlphaCutoff);

    const FWString fname = StringFormat(L"Process:/{0}_DistanceField.hdr", input.BasenameNoExt());
    if (false == Pixmap::Save(&img, fname.MakeView()))
        AssertNotReached();
}
//----------------------------------------------------------------------------
static void Test_DXTCompression_(const FFilename& input) {
    Pixmap::FImage img;
    if (false == Pixmap::Load(&img, Pixmap::_8bits, Pixmap::sRGB, input))
        AssertNotReached();

    Pixmap::FCompressedImage compressed;
    Pixmap::Compress(&compressed, &img, Pixmap::FCompressedImage::EQuality::HighQuality);
}
//----------------------------------------------------------------------------
static void Test_ConvexHull_(const FFilename& input) {
    Pixmap::FImage img;
    if (false == Pixmap::Load(&img, Pixmap::_8bits, Pixmap::sRGB, input))
        AssertNotReached();

    Pixmap::FFloatImage convexhull;
    img.ConvertTo(&convexhull);

    if (not convexhull.HasAlpha())
        return;

    FAabb2f aabb;
    if (not Pixmap::BoundingBox(aabb, &convexhull, AlphaCutoff))
        AssertNotReached();

    float2 corners[4];
    aabb.GetCorners(corners);

    float2 uvs[18];
    if (not Pixmap::ConvexHull(uvs, &convexhull, AlphaCutoff))
        AssertNotReached();

    Pixmap::DrawPolygon(&convexhull, corners, FLinearColor::Indigo());
    Pixmap::DrawPolygon(&convexhull, uvs, FLinearColor::Red());

    img.ConvertFrom(&convexhull);

    const FWString fname = StringFormat(L"Process:/{0}_ConvexHull.png", input.BasenameNoExt());
    if (false == Pixmap::Save(&img, fname.MakeView()))
        AssertNotReached();
}
//----------------------------------------------------------------------------
void Test_Binpacking() {
    const size_t COUNT = 1024;

    const float2 minSize(31.0f);
    const float2 maxSize(127.0f);

    STACKLOCAL_POD_ARRAY(float2, boxes, COUNT);

    FRandomGenerator rng;
    forrange(i, 0, COUNT) {
        float2 box;
#if 0
        box.x() = float(ROUND_TO_NEXT_32(size_t(TLerp(minSize.x(), maxSize.x(), rng.NextFloat01()))));
        box.y() = float(ROUND_TO_NEXT_32(size_t(TLerp(minSize.y(), maxSize.y(), rng.NextFloat01()))));
#else
        box.x() = float(Max(1 << ((1 + rng.NextU32()) & 7), 8));
        box.y() = float(Max(1 << ((1 + rng.NextU32()) & 7), 8));
#endif

        boxes[i] = box;
    }

    STACKLOCAL_POD_ARRAY(float2, offsets, COUNT);

    float2 binsize;
    if (not BinPacking2D(binsize, offsets, boxes))
        AssertNotReached();

    Assert(binsize.x() > 0 && binsize.y() > 0);
    const float2 dUdV = Rcp(binsize);

    Pixmap::FFloatImage intermediate(
        size_t(binsize.x()),
        size_t(binsize.y()),
        FLinearColor::Transparent() );

    forrange(i, 0, COUNT) {
        const float2 pos = offsets[i].xy();

        const FAabb2f box(
            (pos) * dUdV,
            (pos + boxes[i]) * dUdV );

        const float3 hsv(
            rng.NextFloat01(),
            0.5f + 0.5f * rng.NextFloat01(),
            0.5f + 0.5f * rng.NextFloat01() );

        const float3 rgb = HSV_to_RGB(hsv);
        FLinearColor color(rgb.OneExtend());

        color.A = 0.8f;
        Pixmap::FillBoundingBox(&intermediate, box, color);
    }

    Pixmap::FImage img;
    img.ConvertFrom(&intermediate);
    if (false == Pixmap::Save(&img, L"Process:/BinPacking.png"))
        AssertNotReached();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Pixmap() {
    const FFilename inputs[] = {
        L"Data:/Textures/Tech/Flower.png",
        L"Data:/Textures/Tech/Flower2.png",
        L"Data:/Textures/Tech/Chain.png",
        L"Data:/Textures/Tech/text.png",
        L"Data:/Textures/Tech/error.png"
    };

    Test_Binpacking();

    ParallelFor(std::begin(inputs), std::end(inputs), [](const FFilename& fname) {
        Test_ConvexHull_(fname);
    });

    ParallelFor(std::begin(inputs), std::end(inputs), [](const FFilename& fname) {
        Test_ExpandAlphaMask_(fname);
    });

    ParallelFor(std::begin(inputs), std::end(inputs), [](const FFilename& fname) {
        Test_DistanceField(fname);
    });

    ParallelFor(std::begin(inputs), std::end(inputs), [](const FFilename& fname) {
        Test_DXTCompression_(fname);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace Core