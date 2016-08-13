#include "stdafx.h"

#include "ImageHelpers.h"

#include "Enums.h"
#include "FloatImage.h"
#include "Image.h"

#include "Core/Container/Vector.h"
#include "Core/Maths/ConvexHull.h"
#include "Core/Maths/ScalarBoundingBox.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarVectorHelpers.h"
#include "Core/Maths/MathHelpers.h"

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// CDT - Chamfer Distance Transform
// http://thomasdiewald.com/blog/?p=1994
// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.102.7988&rep=rep1&type=pdf
//----------------------------------------------------------------------------
void DistanceField_CDT(Image* dst, const FloatImage* src, float alphaCutoff) {
    Assert(dst);
    Assert(src);
    Assert(alphaCutoff >= 0.f && alphaCutoff < 1.f);

    const size_t w = src->Width();
    const size_t h = src->Height();
    if (w == 0 || h == 0)
        return;

    const float DISTMAX = float(w*w + h*h) + 1;

    dst->Resize_DiscardData(w, h, ColorDepth::_32bits, ColorMask::R, ColorSpace::Float);

    const MemoryView<const FloatImage::color_type> srcData = src->MakeConstView();
    const MemoryView<float> dstData = dst->MakeView().Cast<float>();

    STACKLOCAL_POD_ARRAY(ushort2, nearest, w*h);

    size_t opaqueCount = 0;

    forrange(y, 0, h) {
        const size_t offset = y * w;
        forrange(x, 0, w) {
            const size_t index = offset + x;
            if (srcData[index].a() > alphaCutoff) {
                dstData[index] = 0;
                nearest[index] = ushort2(ushort(x), ushort(y));
                opaqueCount++;
            }
            else {
                dstData[index] = DISTMAX;
            }
        }
    }

    if (w*h == opaqueCount || 0 == opaqueCount)
        return;

    constexpr float d1 = 1;
    constexpr float d2 = (d1*d1 + d1*d1);

    //    1  2  3
    //    0  i  4
    //    5  6  7

    // first pass: forward -> L->R, T-B
    forrange(y, 0, h) {
        const size_t offset = y * w;
        forrange(x, 0, w) {
            const size_t index = offset + x;
            float nd = dstData[index];
            if (nd != 0) {
                float d;
                if (x > 0 && (d = d1 + dstData[index - 1]) < nd) // 0
                    nd = d;
                if (x > 0 && y > 0 && (d = d2 + dstData[index - 1 - w]) < nd) // 1
                    nd = d;
                if (y > 0 && (d = d1 + dstData[index - w]) < nd) // 2
                    nd = d;
                if (x + 1 < w && y > 0 && (d = d2 + dstData[index - w + 1]) < nd) // 2
                    nd = d;

                dstData[index] = nd;
            }
        }
    }

    float actualMaxDistSq = 0;

    // second pass: backwards -> R->L, B-T
    // exactly same as first pass, just in the reverse direction
    reverseforrange(y, 0, h) {
        const size_t offset = y * w;
        reverseforrange(x, 0, w) {
            const size_t index = offset + x;
            float nd = dstData[index];
            if (nd != 0) {
                float d;
                if (x + 1 < w && (d = d1 + dstData[index + 1]) < nd) // 4
                    nd = d;
                if (x > 0 && y + 1 < h && (d = d2 + dstData[index + w - 1]) < nd) // 5
                    nd = d;
                if (y + 1 < h && (d = d1 + dstData[index + w]) < nd) // 6
                    nd = d;
                if (x + 1 < w && y + 1 < h && (d = d2 + dstData[index + w + 1]) < nd) // 7
                    nd = d;

                dstData[index] = nd;
                actualMaxDistSq = Max(actualMaxDistSq, nd);
            }
        }
    }

    const float actualMaxDistOO = Rcp(Sqrt(actualMaxDistSq));

    for (float& f : dstData) {
        f = Saturate(Sqrt(f) * actualMaxDistOO);
    }
}
//----------------------------------------------------------------------------
// DRA - Dead Reckoning Algorithm
// http://thomasdiewald.com/blog/?p=1994
// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.102.7988&rep=rep1&type=pdf
//----------------------------------------------------------------------------
void DistanceField_DRA(Image* dst, const FloatImage* src, float alphaCutoff) {
    Assert(dst);
    Assert(src);
    Assert(alphaCutoff >= 0.f && alphaCutoff < 1.f);

    const size_t w = src->Width();
    const size_t h = src->Height();
    if (w == 0 || h == 0)
        return;

    const float DISTMAX = float(w*w + h*h) + 1;

    dst->Resize_DiscardData(w, h, ColorDepth::_32bits, ColorMask::R, ColorSpace::Float);

    const MemoryView<const FloatImage::color_type> srcData = src->MakeConstView();
    const MemoryView<float> dstData = dst->MakeView().Cast<float>();

    STACKLOCAL_POD_ARRAY(ushort2, nearest, w*h);

    size_t opaqueCount = 0;

    forrange(y, 0, h) {
        const size_t offset = y * w;
        forrange(x, 0, w) {
            const size_t index = offset + x;
            if (srcData[index].a() > alphaCutoff) {
                dstData[index] = 0;
                nearest[index] = ushort2(ushort(x), ushort(y));
                opaqueCount++;
            }
            else {
                dstData[index] = DISTMAX;
            }
        }
    }

    if (w*h == opaqueCount || 0 == opaqueCount)
        return;

    constexpr float d1 = 1;
    constexpr float d2 = (d1*d1 + d1*d1);

    struct DistanceSq_ {
        float operator()(size_t x0, size_t y0, size_t x1, size_t y1) {
            const size_t xx = (x0 < x1 ? x1 - x0 : x0 - x1);
            const size_t yy = (y0 < y1 ? y1 - y0 : y0 - y1);
            return float(xx*xx + yy*yy);
        }
    };

    //    1  2  3
    //    0  i  4
    //    5  6  7

    // first pass: forward -> L->R, T-B
    forrange(y, 0, h) {
        const size_t offset = y * w;
        forrange(x, 0, w) {
            const size_t index = offset + x;
            float nd = dstData[index];
            if (nd != 0) {
                ushort2 nn = nearest[index];

                if (x > 0 && d1 + dstData[index - 1] < nd) { // 0
                    nn = nearest[index - 1];
                    nd = DistanceSq_()(x, y, nn.x(), nn.y());
                }

                if (x > 0 && y > 0 && d2 + dstData[index - 1 - w] < nd) { // 1
                    nn = nearest[index - 1 - w];
                    nd = DistanceSq_()(x, y, nn.x(), nn.y());
                }

                if (y > 0 && d1 + dstData[index - w] < nd) { // 2
                    nn = nearest[index - w];
                    nd = DistanceSq_()(x, y, nn.x(), nn.y());
                }

                if (x + 1 < w && y > 0 && d2 + dstData[index - w + 1] < nd) { // 2
                    nn = nearest[index - w + 1];
                    nd = DistanceSq_()(x, y, nn.x(), nn.y());
                }

                nearest[index] = nn;
                dstData[index] = nd;
            }
        }
    }

    float actualMaxDistSq = 0;

    // second pass: backwards -> R->L, B-T
    // exactly same as first pass, just in the reverse direction
    reverseforrange(y, 0, h) {
        const size_t offset = y * w;
        reverseforrange(x, 0, w) {
            const size_t index = offset + x;
            float nd = dstData[index];
            if (nd != 0) {
                ushort2 nn = nearest[index];

                if (x + 1 < w && d1 + dstData[index + 1] < nd) { // 4
                    nn = nearest[index + 1];
                    nd = DistanceSq_()(x, y, nn.x(), nn.y());
                }

                if (x > 0 && y + 1 < h && d2 + dstData[index + w - 1] < nd) { // 5
                    nn = nearest[index + w - 1];
                    nd = DistanceSq_()(x, y, nn.x(), nn.y());
                }

                if (y + 1 < h && d1 + dstData[index + w] < nd) { // 6
                    nn = nearest[index + w];
                    nd = DistanceSq_()(x, y, nn.x(), nn.y());
                }

                if (x + 1 < w && y + 1 < h && d2 + dstData[index + w + 1] < nd) { // 7
                    nn = nearest[index + w + 1];
                    nd = DistanceSq_()(x, y, nn.x(), nn.y());
                }

                nearest[index] = nn;
                dstData[index] = nd;

                actualMaxDistSq = Max(actualMaxDistSq, nd);
            }
        }
    }

    const float actualMaxDistOO = Rcp(Sqrt(actualMaxDistSq));

    for (float& f : dstData) {
        f = Saturate(Sqrt(f) * actualMaxDistOO);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ExpandColorToTransparentPixels(FloatImage* img, float alphaCutoff) {
    Assert(img);
    Assert(alphaCutoff >= 0.f && alphaCutoff < 1.f);

    const size_t w = img->Width();
    const size_t h = img->Height();
    if (w == 0 || h == 0)
        return;

    typedef FloatImage::color_type color_type;

    const float DISTMAX = float(w*w + h*h) + 1;
    const MemoryView<color_type> colors = img->MakeView();

    STACKLOCAL_POD_ARRAY(float, distance, w*h);

    size_t opaqueCount = 0;

    forrange(y, 0, h) {
        const size_t offset = y * w;
        forrange(x, 0, w) {
            const size_t index = offset + x;
            if (colors[index].a() > alphaCutoff) {
                distance[index] = 0;
                opaqueCount++;
            }
            else {
                distance[index] = DISTMAX;
            }
        }
    }

    if (w*h == opaqueCount || 0 == opaqueCount)
        return;

    constexpr float d1 = 1;
    constexpr float d2 = (d1*d1 + d1*d1);

    //    1  2  3
    //    0  i  4
    //    5  6  7

    // first pass: forward -> L->R, T-B
    forrange(y, 0, h) {
        const size_t offset = y * w;
        forrange(x, 0, w) {
            const size_t index = offset + x;
            float nd = distance[index];
            if (nd != 0) {
                color_type& dst = colors[index];
                float r = dst.r();
                float g = dst.g();
                float b = dst.b();
                float d;

                if (x > 0 && (d = d1 + distance[index - 1]) < nd) { // 0
                    const color_type& src = colors[index - 1];
                    r = src.r();
                    g = src.g();
                    b = src.b();
                    nd = d;
                }

                if (x > 0 && y > 0 && (d = d2 + distance[index - 1 - w]) < nd) { // 1
                    const color_type& src = colors[index - 1 - w];
                    r = src.r();
                    g = src.g();
                    b = src.b();
                    nd = d;
                }

                if (y > 0 && (d = d1 + distance[index - w]) < nd) { // 2
                    const color_type& src = colors[index - w];
                    r = src.r();
                    g = src.g();
                    b = src.b();
                    nd = d;
                }

                if (x + 1 < w && y > 0 && (d = d2 + distance[index - w + 1]) < nd) { // 2
                    const color_type& src = colors[index - w + 1];
                    r = src.r();
                    g = src.g();
                    b = src.b();
                    nd = d;
                }

                dst.r() = r;
                dst.g() = g;
                dst.b() = b;

                distance[index] = nd;
            }
        }
    }

    // second pass: backwards -> R->L, B-T
    // exactly same as first pass, just in the reverse direction
    reverseforrange(y, 0, h) {
        const size_t offset = y * w;
        reverseforrange(x, 0, w) {
            const size_t index = offset + x;
            float nd = distance[index];
            if (nd != 0) {
                color_type& dst = colors[index];
                float r = dst.r();
                float g = dst.g();
                float b = dst.b();
                float d;

                if (x + 1 < w && (d = d1 + distance[index + 1]) < nd) { // 4
                    const color_type& src = colors[index + 1];
                    r = src.r();
                    g = src.g();
                    b = src.b();
                    nd = d;
                }

                if (x > 0 && y + 1 < h && (d = d2 + distance[index + w - 1]) < nd) { // 5
                    const color_type& src = colors[index + w - 1];
                    r = src.r();
                    g = src.g();
                    b = src.b();
                    nd = d;
                }

                if (y + 1 < h && (d = d1 + distance[index + w]) < nd) { // 6
                    const color_type& src = colors[index + w];
                    r = src.r();
                    g = src.g();
                    b = src.b();
                    nd = d;
                }

                if (x + 1 < w && y + 1 < h && (d = d2 + distance[index + w + 1]) < nd) { // 7
                    const color_type& src = colors[index + w + 1];
                    r = src.r();
                    g = src.g();
                    b = src.b();
                    nd = d;
                }

                dst.r() = r;
                dst.g() = g;
                dst.b() = b;

                distance[index] = nd;
            }
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool BoundingBox(AABB2f& uvs, const FloatImage* img, float alphaCutoff) {
    Assert(img);
    Assert(img->Width() > 1 && img->Height() > 1);
    Assert(alphaCutoff >= 0.f && alphaCutoff < 1.f);

    typedef FloatImage::color_type color_type;

    const size_t w = img->Width();
    const size_t h = img->Height();

    const float2 dUdV = img->DuDv();

    uvs = AABB2f();

    forrange(y, 0, h) {
        const float v = y * dUdV.y();
        const MemoryView<const color_type> scanline = img->Scanline(y);

        size_t xMin = w;
        size_t xMax = 0;
        forrange(x, 0, w) {
            const color_type& col = scanline[x];
            if (col.a() > alphaCutoff) {
                // add 1 offset to the left
                xMin = Min(xMin, x);
                // add 1 offset to the right
                xMax = Max(xMax, x);
            }
        }

        if (xMin <= xMax) {
            uvs.Add(float2(xMin*dUdV.x(), v));
            uvs.Add(float2(xMax*dUdV.x(), v));
        }
    }

    if (not uvs.HasPositiveExtents())
        return false;

    const float2 borderDuDv = dUdV * 5;
    uvs.Add(Max(uvs.Min() - borderDuDv, float2::Zero()));
    uvs.Add(Min(uvs.Max() + borderDuDv, float2::One() - dUdV));

    return true;
}
//----------------------------------------------------------------------------
bool ConvexHull(const MemoryView<float2>& uvs, const FloatImage* img, float alphaCutoff) {
    Assert(img);
    Assert(img->Width() > 1 && img->Height() > 1);
    Assert(uvs.size() > 2);
    Assert(alphaCutoff >= 0.f && alphaCutoff < 1.f);

    typedef FloatImage::color_type color_type;

    const size_t w = img->Width();
    const size_t h = img->Height();

    VECTOR_THREAD_LOCAL(FloatImage, float2) points;

    constexpr size_t BORDER = 4;

    forrange(y, 0, h) {
        const MemoryView<const color_type> scanline = img->Scanline(y);

        size_t xMin = w;
        size_t xMax = 0;
        forrange(x, 0, w) {
            const color_type& col = scanline[x];
            if (col.a() > alphaCutoff) {
                // add 1 offset to the left
                xMin = Min(xMin, (x < BORDER ? 0 : x - BORDER));
                // add 1 offset to the right
                xMax = Max(xMax, (w - 1 < x + BORDER ? w - 1 : x + BORDER));
            }
        }

        if (xMin < xMax) {
            points.emplace_back(float(xMin), float(y));
            points.emplace_back(float(xMax), float(y));
        }
        else if (xMin == xMax) {
            points.emplace_back(float(xMin), float(y));
        }
    }

    if (points.size() < 3)
        return false;

    float2 topBorder = (points[0] + points[1]) * 0.5f;
    topBorder.y() = (topBorder.y() < BORDER ? 0 : topBorder.y() - BORDER);

    float2 bottomBorder = (points[points.size() - 2] + points[points.size() - 1]) * 0.5f;
    bottomBorder.y() = (h - 1 < bottomBorder.y() + BORDER ? h - 1 : bottomBorder.y() + BORDER);

    points.insert(points.begin(), topBorder);
    points.push_back(bottomBorder);

    const float2 DuDv = img->DuDv();
    for (float2& p : points)
        p *= DuDv;

    ConvexHull2D_FixedSize(uvs, MakeView(points));

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
