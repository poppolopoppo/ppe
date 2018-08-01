#pragma once

#include "Pixmap.h"
#include "Pixmap_fwd.h"

#include "Color/Color_fwd.h"
#include "Maths/ScalarBoundingBox_fwd.h"
#include "Maths/ScalarVector_fwd.h"

namespace PPE {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DrawPoint(FFloatImage* img, int x, int y, const FLinearColor& color);
//----------------------------------------------------------------------------
void DrawLine(FFloatImage* img, int x0, int y0, int x1, int y1, const FLinearColor& color);
void DrawLine(FFloatImage* img, const int2& p0, const int2& p1, const FLinearColor& color);
void DrawLine(FFloatImage* img, const float2& uv0, const float2& uv1, const FLinearColor& color);
//----------------------------------------------------------------------------
void DrawBoundingBox(FFloatImage* img, const FAabb2i& box, const FLinearColor& color);
void DrawBoundingBox(FFloatImage* img, const FAabb2f& uvs, const FLinearColor& color);
//----------------------------------------------------------------------------
void DrawPolyline(FFloatImage* img, const TMemoryView<const int2>& polyline, const FLinearColor& color);
void DrawPolyline(FFloatImage* img, const TMemoryView<const float2>& uvs, const FLinearColor& color);
//----------------------------------------------------------------------------
void DrawPolygon(FFloatImage* img, const TMemoryView<const int2>& polygon, const FLinearColor& color);
void DrawPolygon(FFloatImage* img, const TMemoryView<const float2>& uvs, const FLinearColor& color);
//----------------------------------------------------------------------------
void FillBoundingBox(FFloatImage* img, const FAabb2i& box, const FLinearColor& color);
void FillBoundingBox(FFloatImage* img, const FAabb2f& uvs, const FLinearColor& color);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace PPE
