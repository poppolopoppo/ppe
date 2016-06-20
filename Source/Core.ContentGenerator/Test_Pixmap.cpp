#include "stdafx.h"

#include "Core/Color/Color.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Thread/Task.h"

#include "Core.Pixmap/Drawing.h"
#include "Core.Pixmap/DXTImage.h"
#include "Core.Pixmap/FloatImage.h"
#include "Core.Pixmap/Image.h"
#include "Core.Pixmap/ImageHelpers.h"
#include "Core.Pixmap/MipMapChain.h"

#include "Core/Maths/Geometry/BinPacking.h"
#include "Core/Maths/Geometry/ScalarBoundingBox.h"
#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/RandomGenerator.h"

namespace Core {
namespace ContentGenerator {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
constexpr float AlphaCutoff = 0.33333f;
//----------------------------------------------------------------------------
static void Test_ExpandAlphaMask_(const Filename& input) {
    const Dirpath output = L"Process:/";

    Pixmap::Image img;
    if (false == Pixmap::Load(&img, input))
        AssertNotReached();

    Pixmap::PFloatImage intermediate = new Pixmap::FloatImage();
    img.ConvertTo(intermediate.get());

    const bool hasAlpha = intermediate->HasAlpha();

    if (hasAlpha) {
        Pixmap::FloatImage cpy = *intermediate;
        cpy.DiscardAlpha();
        img.ConvertFrom(&cpy);
        if (false == Pixmap::Save(&img, StringFormat(L"{0}/{1}_DiscardAlpha.png", output, input.BasenameNoExt())) )
            AssertNotReached();
    }

    if (hasAlpha)
        Pixmap::ExpandColorToTransparentPixels(intermediate.get(), AlphaCutoff);

    if (hasAlpha) {
        Pixmap::FloatImage cpy = *intermediate;
        cpy.DiscardAlpha();
        img.ConvertFrom(&cpy);
        if (false == Pixmap::Save(&img, StringFormat(L"{0}/{1}_ExpandAlphaColor.png", output, input.BasenameNoExt())) )
            AssertNotReached();
    }

    Pixmap::MipMapChain mipMaps;
    mipMaps.Generate(intermediate.get(), true);

    if (hasAlpha)
        mipMaps.PreserveAlphaTestCoverage(AlphaCutoff);

    const auto chain = mipMaps.MakeView();
    forrange(i, 0, chain.size()) {
        //chain[i]->DiscardAlpha();

        img.ConvertFrom(chain[i].get());
        if (false == Pixmap::Save(&img, StringFormat(L"{0}/{1}_Mip{2:#2}.png", output, input.BasenameNoExt(), i)) )
            AssertNotReached();
    }
}
//----------------------------------------------------------------------------
static void Test_DistanceField(const Filename& input) {
    Pixmap::Image img;
    if (false == Pixmap::Load(&img, input))
        AssertNotReached();

    Pixmap::FloatImage tmp;
    img.ConvertTo(&tmp);

    Pixmap::DistanceField_DRA(&img, &tmp, AlphaCutoff);

    if (false == Pixmap::Save(&img, StringFormat(L"Process:/{0}_DistanceField.hdr", input.BasenameNoExt())))
        AssertNotReached();
}
//----------------------------------------------------------------------------
static void Test_DXTCompression_(const Filename& input) {
    Pixmap::Image img;
    if (false == Pixmap::Load(&img, input))
        AssertNotReached();

    Pixmap::DXTImage compressed;
    Pixmap::Compress(&compressed, &img);
}
//----------------------------------------------------------------------------
static void Test_ConvexHull_(const Filename& input) {
    Pixmap::Image img;
    if (false == Pixmap::Load(&img, input))
        AssertNotReached();

    Pixmap::FloatImage convexhull;
    img.ConvertTo(&convexhull);

    if (not convexhull.HasAlpha())
        return;

    AABB2f aabb;
    if (not Pixmap::BoundingBox(aabb, &convexhull, AlphaCutoff))
        AssertNotReached();

    float2 corners[4];
    aabb.GetCorners(corners);

    float2 uvs[18];
    if (not Pixmap::ConvexHull(uvs, &convexhull, AlphaCutoff))
        AssertNotReached();

    Pixmap::DrawPolygon(&convexhull, corners, Color::Indigo().ToLinear());
    Pixmap::DrawPolygon(&convexhull, uvs, Color::Red().ToLinear());

    img.ConvertFrom(&convexhull);
    if (false == Pixmap::Save(&img, StringFormat(L"Process:/{0}_ConvexHull.png", input.BasenameNoExt())))
        AssertNotReached();
}
//----------------------------------------------------------------------------
void Test_Binpacking() {
    const size_t COUNT = 1024;

    const float2 minSize(31.0f);
    const float2 maxSize(127.0f);

    STACKLOCAL_POD_ARRAY(float2, boxes, COUNT);

    RandomGenerator rng;
    forrange(i, 0, COUNT) {
        float2 box;
#if 0
        box.x() = float(ROUND_TO_NEXT_32(size_t(Lerp(minSize.x(), maxSize.x(), rng.NextFloat01()))));
        box.y() = float(ROUND_TO_NEXT_32(size_t(Lerp(minSize.y(), maxSize.y(), rng.NextFloat01()))));
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

    Pixmap::FloatImage intermediate(
        size_t(binsize.x()),
        size_t(binsize.y()),
        Color::Transparent() );

    forrange(i, 0, COUNT) {
        const float2 pos = offsets[i].xy();

        const AABB2f box(
            (pos) * dUdV,
            (pos + boxes[i]) * dUdV );

        const float3 hsv(
            rng.NextFloat01(),
            0.5f + 0.5f * rng.NextFloat01(),
            0.5f + 0.5f * rng.NextFloat01() );

        const float3 rgb = HSV_to_RGB(hsv);
        ColorRGBAF color(rgb.OneExtend());

        color.a() = 0.8f;
        Pixmap::FillBoundingBox(&intermediate, box, color.ToLinear());
    }

    Pixmap::Image img;
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
    const Filename inputs[] = {
        L"Data:/Textures/Tech/Flower.png",
        L"Data:/Textures/Tech/Flower2.png",
        L"Data:/Textures/Tech/Chain.png",
        L"Data:/Textures/Tech/text.png",
        L"Data:/Textures/Tech/error.png"
    };

    Test_Binpacking();

    parallel_for(std::begin(inputs), std::end(inputs), [](const Filename& fname) {
        Test_ConvexHull_(fname);
    });

    parallel_for(std::begin(inputs), std::end(inputs), [](const Filename& fname) {
        Test_ExpandAlphaMask_(fname);
    });

    parallel_for(std::begin(inputs), std::end(inputs), [](const Filename& fname) {
        Test_DistanceField(fname);
    });

    parallel_for(std::begin(inputs), std::end(inputs), [](const Filename& fname) {
        Test_DXTCompression_(fname);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core
