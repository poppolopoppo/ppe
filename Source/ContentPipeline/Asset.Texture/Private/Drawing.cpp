#include "stdafx.h"

#include "Drawing.h"

#include "FloatImage.h"

#include "Color/Color.h"
#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Maths/MathHelpers.h"

namespace PPE {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DrawPoint(FFloatImage* img, int x, int y, const FLinearColor& color) {
    const int w = int(img->Width());
    const int h = int(img->Height());
    if (x >= 0 && x < w && y >= 0 && y < h ) {
        FFloatImage::color_type& dst = img->at(size_t(x), size_t(y));
        dst = AlphaBlend(dst, color);
    }
}
//----------------------------------------------------------------------------
void DrawLine(FFloatImage* img, int x0, int y0, int x1, int y1, const FLinearColor& color) {
    Assert(img);

    const int dx = x1 - x0;
    const int dy = y1 - y0;

    if (Abs(dx) > Abs(dy)) {
        if (x1 < x0) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        const float gradient = float(y1 - y0) / (x1 - x0);

        FLinearColor c = color;
        float yy = float(y0);
        forrange(x, x0, x1) {
            const int y = FloorToInt(yy);
            const float fy = yy - y;
            c.A = color.A * (1.0f - fy);
            DrawPoint(img, x, y, c);
            c.A = color.A * fy;
            DrawPoint(img, x, y + 1, c);
            yy += gradient;
        }
    }
    else {
        if (y1 < y0) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        const float gradient = float(x1 - x0) / (y1 - y0);

        FLinearColor c = color;
        float xx = float(x0);
        forrange(y, y0, y1) {
            const int x = FloorToInt(xx);
            const float fx = xx - x;
            c.A = color.A * (1.0f - fx);
            DrawPoint(img, x, y, c);
            c.A = color.A * fx;
            DrawPoint(img, x + 1, y, c);
            xx += gradient;
        }
    }
}
//----------------------------------------------------------------------------
void DrawLine(FFloatImage* img, const int2& p0, const int2& p1, const FLinearColor& color) {
    DrawLine(img, p0.x(), p0.y(), p1.x(), p1.y(), color);
}
//----------------------------------------------------------------------------
void DrawLine(FFloatImage* img, const float2& uv0, const float2& uv1, const FLinearColor& color) {
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
void DrawBoundingBox(FFloatImage* img, const FAabb2i& box, const FLinearColor& color) {
    int2 corners[4];
    box.GetCorners(corners);
    DrawPolygon(img, corners, color);
}
//----------------------------------------------------------------------------
void DrawBoundingBox(FFloatImage* img, const FAabb2f& uvs, const FLinearColor& color) {
    float2 corners[4];
    uvs.GetCorners(corners);
    DrawPolygon(img, corners, color);
}
//----------------------------------------------------------------------------
void DrawPolyline(FFloatImage* img, const TMemoryView<const int2>& polyline, const FLinearColor& color) {
    forrange(i, 1, polyline.size())
        DrawLine(img, polyline[i - 1].x(), polyline[i - 1].y(), polyline[i].x(), polyline[i].y(), color);
}
//----------------------------------------------------------------------------
void DrawPolyline(FFloatImage* img, const TMemoryView<const float2>& uvs, const FLinearColor& color) {
    forrange(i, 1, uvs.size())
        DrawLine(img, uvs[i - 1],  uvs[i], color);
}
//----------------------------------------------------------------------------
void DrawPolygon(FFloatImage* img, const TMemoryView<const int2>& polygon, const FLinearColor& color) {
    DrawPolyline(img, polygon, color);
    DrawLine(img, polygon.back().x(), polygon.back().y(), polygon.front().x(), polygon.front().y(), color);
}
//----------------------------------------------------------------------------
void DrawPolygon(FFloatImage* img, const TMemoryView<const float2>& uvs, const FLinearColor& color) {
    DrawPolyline(img, uvs, color);
    DrawLine(img, uvs.back(), uvs.front(), color);
}
//----------------------------------------------------------------------------
void FillBoundingBox(FFloatImage* img, const FAabb2i& box, const FLinearColor& color) {
    FLinearColor border = color;
    border.A = 1.0f;

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
void FillBoundingBox(FFloatImage* img, const FAabb2f& uvs, const FLinearColor& color) {
    const int w = checked_cast<int>(img->Width());
    const int h = checked_cast<int>(img->Height());
    FillBoundingBox(img, FAabb2i(
        int2(int(uvs.Min().x() * w), int(uvs.Min().y() * h)),
        int2(int(uvs.Max().x() * w), int(uvs.Max().y() * h)) ),
        color );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace PPE
