#pragma once

#include "Core.Pixmap/Pixmap.h"
#include "Core.Pixmap/Pixmap_fwd.h"

#include "Core/Color/Color_fwd.h"
#include "Core/Maths/ScalarBoundingBox_fwd.h"
#include "Core/Maths/ScalarVector_fwd.h"

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DrawPoint(FFloatImage* img, int x, int y, const ColorRGBAF& color);
//----------------------------------------------------------------------------
void DrawLine(FFloatImage* img, int x0, int y0, int x1, int y1, const ColorRGBAF& color);
void DrawLine(FFloatImage* img, const int2& p0, const int2& p1, const ColorRGBAF& color);
void DrawLine(FFloatImage* img, const float2& uv0, const float2& uv1, const ColorRGBAF& color);
//----------------------------------------------------------------------------
void DrawBoundingBox(FFloatImage* img, const FAabb2i& box, const ColorRGBAF& color);
void DrawBoundingBox(FFloatImage* img, const FAabb2f& uvs, const ColorRGBAF& color);
//----------------------------------------------------------------------------
void DrawPolyline(FFloatImage* img, const TMemoryView<const int2>& polyline, const ColorRGBAF& color);
void DrawPolyline(FFloatImage* img, const TMemoryView<const float2>& uvs, const ColorRGBAF& color);
//----------------------------------------------------------------------------
void DrawPolygon(FFloatImage* img, const TMemoryView<const int2>& polygon, const ColorRGBAF& color);
void DrawPolygon(FFloatImage* img, const TMemoryView<const float2>& uvs, const ColorRGBAF& color);
//----------------------------------------------------------------------------
void FillBoundingBox(FFloatImage* img, const FAabb2i& box, const ColorRGBAF& color);
void FillBoundingBox(FFloatImage* img, const FAabb2f& uvs, const ColorRGBAF& color);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
