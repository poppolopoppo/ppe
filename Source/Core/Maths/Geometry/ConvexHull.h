#pragma once

#include "Core/Core.h"

#include "Core/Maths/Geometry/ScalarVector_fwd.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Monotone chain
// https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain#C.2B.2B
//----------------------------------------------------------------------------
size_t ConvexHull2D_MonotoneChain(const MemoryView<size_t>& hull, const MemoryView<const float2>& points);
//----------------------------------------------------------------------------
void ConvexHull2D_FixedSize(const MemoryView<float2>& hull, const MemoryView<const float2>& points);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
