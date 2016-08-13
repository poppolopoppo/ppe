#pragma once

#include "Core/Core.h"

#include "Core/Maths/ScalarBoundingBox.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarVectorHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T Area(const ScalarBoundingBox<T, 2>& aabb);
//----------------------------------------------------------------------------
template <typename T>
T Area(const ScalarBoxWExtent<T, 2>& aabb);
//----------------------------------------------------------------------------
template <typename T>
T Volume(const ScalarBoundingBox<T, 3>& aabb);
//----------------------------------------------------------------------------
template <typename T>
T Volume(const ScalarBoxWExtent<T, 3>& aabb);
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

#include "Core/Maths/ScalarBoundingBoxHelpers-inl.h"
