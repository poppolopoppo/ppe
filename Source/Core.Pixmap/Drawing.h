#pragma once

#include "Core.Pixmap/Pixmap.h"
#include "Core.Pixmap/Pixmap_fwd.h"

#include "Core/Color/Color_fwd.h"
#include "Core/Maths/Geometry/ScalarBoundingBox_fwd.h"
#include "Core/Maths/Geometry/ScalarVector_fwd.h"

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DrawPoint(FloatImage* img, int x, int y, const ColorRGBAF& color);
//----------------------------------------------------------------------------
void DrawLine(FloatImage* img, int x0, int y0, int x1, int y1, const ColorRGBAF& color);
void DrawLine(FloatImage* img, const int2& p0, const int2& p1, const ColorRGBAF& color);
void DrawLine(FloatImage* img, const float2& uv0, const float2& uv1, const ColorRGBAF& color);
//----------------------------------------------------------------------------
void DrawBoundingBox(FloatImage* img, const AABB2i& box, const ColorRGBAF& color);
void DrawBoundingBox(FloatImage* img, const AABB2f& uvs, const ColorRGBAF& color);
//----------------------------------------------------------------------------
void DrawPolyline(FloatImage* img, const MemoryView<const int2>& polyline, const ColorRGBAF& color);
void DrawPolyline(FloatImage* img, const MemoryView<const float2>& uvs, const ColorRGBAF& color);
//----------------------------------------------------------------------------
void DrawPolygon(FloatImage* img, const MemoryView<const int2>& polygon, const ColorRGBAF& color);
void DrawPolygon(FloatImage* img, const MemoryView<const float2>& uvs, const ColorRGBAF& color);
//----------------------------------------------------------------------------
void FillBoundingBox(FloatImage* img, const AABB2i& box, const ColorRGBAF& color);
void FillBoundingBox(FloatImage* img, const AABB2f& uvs, const ColorRGBAF& color);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
