#pragma once

#include "Core/Core.h"

#include "Core/Maths/Geometry/ScalarBoundingBox.h"
#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Geometry/ScalarVectorHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<float, _Dim> LinearStep(const ScalarBoundingBox<T, _Dim>& aabb, const ScalarVector<T, _Dim>& value);
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
ScalarVector<U, _Dim> QuantizeCeil(const ScalarBoundingBox<T, _Dim>& aabb, const ScalarVector<T, _Dim>& value);
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
ScalarVector<U, _Dim> QuantizeFloor(const ScalarBoundingBox<T, _Dim>& aabb, const ScalarVector<T, _Dim>& value);
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
ScalarVector<U, _Dim> QuantizeRound(const ScalarBoundingBox<T, _Dim>& aabb, const ScalarVector<T, _Dim>& value);
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
ScalarVector<T, _Dim> Unquantize(const ScalarBoundingBox<T, _Dim>& aabb, const ScalarVector<U, _Dim>& quantized);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/Geometry/ScalarBoundingBoxHelpers-inl.h"
