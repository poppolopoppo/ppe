#pragma once

#include "Core.h"

#include "Maths/ScalarVector_fwd.h"
#include "Memory/MemoryView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Monotone chain
// https://en.wikibooks.org/wiki/Algorithm_Implementation/Convex_hull/Monotone_chain#C.2B.2B
//----------------------------------------------------------------------------
size_t ConvexHull2D_MonotoneChain(const TMemoryView<size_t>& hull, const TMemoryView<const float2>& points);
//----------------------------------------------------------------------------
void ConvexHull2D_FixedSize(const TMemoryView<float2>& hull, const TMemoryView<const float2>& points);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
