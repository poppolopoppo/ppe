#pragma once

#include "Core.Pixmap/Pixmap.h"
#include "Core.Pixmap/Pixmap_fwd.h"

#include "Core/Maths/ScalarBoundingBox_fwd.h"
#include "Core/Maths/ScalarVector_fwd.h"

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DistanceField_CDT(Image* dst, const FFloatImage* src, float alphaCutoff);
//----------------------------------------------------------------------------
void DistanceField_DRA(Image* dst, const FFloatImage* src, float alphaCutoff);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ExpandColorToTransparentPixels(FFloatImage* img, float alphaCutoff);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool BoundingBox(AABB2f& uvs, const FFloatImage* img, float alphaCutoff);
//----------------------------------------------------------------------------
bool ConvexHull(const TMemoryView<float2>& uvs, const FFloatImage* img, float alphaCutoff);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
