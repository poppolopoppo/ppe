#include "stdafx.h"

#include "Core/Color/Color.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Thread/Task.h"

#include "Core.Pixmap/DXTImage.h"
#include "Core.Pixmap/FloatImage.h"
#include "Core.Pixmap/Image.h"
#include "Core.Pixmap/ImageHelpers.h"
#include "Core.Pixmap/MipMapChain.h"

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

    if (hasAlpha)
    {
        Pixmap::FloatImage cpy = *intermediate;
        cpy.DiscardAlpha();
        img.ConvertFrom(&cpy);
        if (false == Pixmap::Save(&img, StringFormat(L"{0}/{1}_DiscardAlpha.png", output, input.BasenameNoExt())) )
            AssertNotReached();
    }

    if (hasAlpha)
        Pixmap::ExpandColorToTransparentPixels(intermediate.get(), AlphaCutoff);

    if (hasAlpha)
    {
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
    if (not Pixmap::ConvexHull(MakeView(uvs), &convexhull, AlphaCutoff))
        AssertNotReached();

    Pixmap::DrawPolygon(&convexhull, corners, Color::Cyan().ToLinear());
    Pixmap::DrawPolygon(&convexhull, uvs, Color::Red().ToLinear());

    img.ConvertFrom(&convexhull);
    if (false == Pixmap::Save(&img, StringFormat(L"Process:/{0}_ConvexHull.png", input.BasenameNoExt())))
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
