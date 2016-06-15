#pragma once

#include "Core.Pixmap/Pixmap.h"
#include "Core.Pixmap/Pixmap_extern.h"

#include "Core/Maths/Geometry/ScalarVector_fwd.h"

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DistanceField_CDT(Image* dst, const FloatImage* src, float alphaCutoff);
//----------------------------------------------------------------------------
void DistanceField_DRA(Image* dst, const FloatImage* src, float alphaCutoff);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ExpandColorToTransparentPixels(FloatImage* img, float alphaCutoff);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool ConvexHull(const MemoryView<float2>& uvs, const FloatImage* img, float alphaCutoff);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DrawPoint(FloatImage* img, int x, int y, const FloatImage::color_type& color);
//----------------------------------------------------------------------------
void DrawLine(FloatImage* img, int x0, int y0, int x1, int y1, const FloatImage::color_type& color);
//----------------------------------------------------------------------------
void DrawPolyline(FloatImage* img, const MemoryView<const int2>& polyline, const FloatImage::color_type& color);
void DrawPolyline(FloatImage* img, const MemoryView<const float2>& uvs, const FloatImage::color_type& color);
//----------------------------------------------------------------------------
void DrawPolygon(FloatImage* img, const MemoryView<const int2>& polygon, const FloatImage::color_type& color);
void DrawPolygon(FloatImage* img, const MemoryView<const float2>& uvs, const FloatImage::color_type& color);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
