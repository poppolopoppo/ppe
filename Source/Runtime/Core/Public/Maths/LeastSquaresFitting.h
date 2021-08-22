#pragma once

#include "Core.h"

#include "Maths/ScalarVector_fwd.h"
#include "Memory/MemoryView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool LeastSquaresFittingGaussian2(float2& center, float2& axis0, float2& axis1, float2& extents, const TMemoryView<const float2>& points);
bool LeastSquaresFittingGaussian3(float3& center, float3& axis0, float3& axis1, float3& axis2, float3& extents, const TMemoryView<const float3>& points);
//----------------------------------------------------------------------------
float LeastSquaresFittingQuadratic2(TScalarVector<float, 6>& coefficients, const TMemoryView<const float2>& points);
float LeastSquaresFittingQuadraticCircle2(float2& center, float& radius, const TMemoryView<const float2>& points);
float LeastSquaresFittingQuadratic3(TScalarVector<float, 10>& coefficients, const TMemoryView<const float3>& points);
float LeastSquaresFittingQuadraticSphere3(float3& center, float& radius, const TMemoryView<const float3>& points);
//----------------------------------------------------------------------------
bool LeastSquaresFittingOrthogonalLine2(float2& origin, float2& direction, const TMemoryView<const float2>& points);
bool LeastSquaresFittingOrthogonalLine3(float3& origin, float3& direction, const TMemoryView<const float3>& points);
bool LeastSquaresFittingOrthogonalPlane3(class FPlane& plane, const TMemoryView<const float3>& points);
//----------------------------------------------------------------------------
// TODO
/*
bool LeastSquaresFittingEllipse2(class FPlane& plane, const TMemoryView<const float3>& points);
bool LeastSquaresFittingEllipsoid3(class FPlane& plane, const TMemoryView<const float3>& points);
bool LeastSquaresFittingParabloid3(class FPlane& plane, const TMemoryView<const float3>& points);
*/
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
