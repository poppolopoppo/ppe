#include "stdafx.h"

#include "Drawing.h"

#include "FloatImage.h"

#include "Core/Color/Color.h"
#include "Core/Maths/Geometry/ScalarBoundingBox.h"
#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Geometry/ScalarVectorHelpers.h"
#include "Core/Maths/MathHelpers.h"

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DrawPoint(FloatImage* img, int x, int y, const ColorRGBAF& color) {
    const int w = int(img->Width());
    const int h = int(img->Height());

    if (x < 0 || x >= w || y < 0 || y >= h )
        return;

    FloatImage::color_type& dst = img->at(checked_cast<size_t>(x), checked_cast<size_t>(y));
    if (color.a() < 1)
        dst = Lerp(dst.Data(), color.Data(), color.a());
    else
        dst = color;
}
//----------------------------------------------------------------------------
void DrawLine(FloatImage* img, int x0, int y0, int x1, int y1, const ColorRGBAF& color) {
    Assert(img);

    const int w = checked_cast<int>(img->Width());
    const int h = checked_cast<int>(img->Height());

    const bool steep = Abs(int(y1) - int(y0)) > Abs(int(x1) - int(x0));

    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    const int deltax = int(x1) - int(x0);
    const int deltay = Abs(int(y1) - int(y0));

    int error = -deltax / 2;

    int ystep = y0 < y1 ? 1 : -1;
    int y = int(y0);
    for(int x = int(x0); x <= int(x1); x++) {
        if (steep)
            DrawPoint(img, y, x, color);
        else
            DrawPoint(img, x, y, color);

        error += deltay;
        if (error > 0) {
            y += ystep;
            error -= deltax;
        }
    }
}
//----------------------------------------------------------------------------
void DrawLine(FloatImage* img, const int2& p0, const int2& p1, const ColorRGBAF& color) {
    DrawLine(img, p0.x(), p0.y(), p1.x(), p1.y(), color);
}
//----------------------------------------------------------------------------
void DrawLine(FloatImage* img, const float2& uv0, const float2& uv1, const ColorRGBAF& color) {
    const int w = checked_cast<int>(img->Width());
    const int h = checked_cast<int>(img->Height());
    DrawLine(
        img,
        int(uv0.x() * w),
        int(uv0.y() * h),
        int(uv1.x() * w),
        int(uv1.y() * h),
        color );
}
//----------------------------------------------------------------------------
void DrawBoundingBox(FloatImage* img, const AABB2i& box, const ColorRGBAF& color) {
    int2 corners[4];
    box.GetCorners(corners);
    DrawPolygon(img, corners, color);
}
//----------------------------------------------------------------------------
void DrawBoundingBox(FloatImage* img, const AABB2f& uvs, const ColorRGBAF& color) {
    float2 corners[4];
    uvs.GetCorners(corners);
    DrawPolygon(img, corners, color);
}
//----------------------------------------------------------------------------
void DrawPolyline(FloatImage* img, const MemoryView<const int2>& polyline, const ColorRGBAF& color) {
    forrange(i, 1, polyline.size())
        DrawLine(img, polyline[i - 1].x(), polyline[i - 1].y(), polyline[i].x(), polyline[i].y(), color);
}
//----------------------------------------------------------------------------
void DrawPolyline(FloatImage* img, const MemoryView<const float2>& uvs, const ColorRGBAF& color) {
    const int w = checked_cast<int>(img->Width());
    const int h = checked_cast<int>(img->Height());
    forrange(i, 1, uvs.size())
        DrawLine(img, uvs[i - 1],  uvs[i], color);
}
//----------------------------------------------------------------------------
void DrawPolygon(FloatImage* img, const MemoryView<const int2>& polygon, const ColorRGBAF& color) {
    DrawPolyline(img, polygon, color);
    DrawLine(img, polygon.back().x(), polygon.back().y(), polygon.front().x(), polygon.front().y(), color);
}
//----------------------------------------------------------------------------
void DrawPolygon(FloatImage* img, const MemoryView<const float2>& uvs, const ColorRGBAF& color) {
    const int w = checked_cast<int>(img->Width());
    const int h = checked_cast<int>(img->Height());
    DrawPolyline(img, uvs, color);
    DrawLine(img, uvs.back(), uvs.front(), color);
}
//----------------------------------------------------------------------------
void FillBoundingBox(FloatImage* img, const AABB2i& box, const ColorRGBAF& color) {
    ColorRGBAF border = color;
    border.a() = 1.0f;

    forrange(x, box.Min().x(), box.Max().x())
        DrawPoint(img, x, box.Min().y(), border);

    forrange(y, box.Min().y() + 1, box.Max().y()) {
        DrawPoint(img, box.Min().x(), y, border);

        forrange(x, box.Min().x() + 1, box.Max().x() - 1)
            DrawPoint(img, x, y, color);

        DrawPoint(img, box.Max().x() - 1, y, border);
    }

    forrange(x, box.Min().x(), box.Max().x())
        DrawPoint(img, x, box.Max().y() - 1, border);
}
//----------------------------------------------------------------------------
void FillBoundingBox(FloatImage* img, const AABB2f& uvs, const ColorRGBAF& color) {
    const int w = checked_cast<int>(img->Width());
    const int h = checked_cast<int>(img->Height());
    FillBoundingBox(img, AABB2i(
        int2(int(uvs.Min().x() * w), int(uvs.Min().y() * h)),
        int2(int(uvs.Max().x() * w), int(uvs.Max().y() * h)) ),
        color );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
