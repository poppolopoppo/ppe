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
T Area(const TScalarBoundingBox<T, 2>& aabb);
//----------------------------------------------------------------------------
template <typename T>
T Area(const TScalarBoxWExtent<T, 2>& aabb);
//----------------------------------------------------------------------------
template <typename T>
T Volume(const TScalarBoundingBox<T, 3>& aabb);
//----------------------------------------------------------------------------
template <typename T>
T Volume(const TScalarBoxWExtent<T, 3>& aabb);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<float, _Dim> LinearStep(const TScalarBoundingBox<T, _Dim>& aabb, const TScalarVector<T, _Dim>& value);
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
TScalarVector<U, _Dim> QuantizeCeil(const TScalarBoundingBox<T, _Dim>& aabb, const TScalarVector<T, _Dim>& value);
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
TScalarVector<U, _Dim> QuantizeFloor(const TScalarBoundingBox<T, _Dim>& aabb, const TScalarVector<T, _Dim>& value);
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
TScalarVector<U, _Dim> QuantizeRound(const TScalarBoundingBox<T, _Dim>& aabb, const TScalarVector<T, _Dim>& value);
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
TScalarVector<T, _Dim> Unquantize(const TScalarBoundingBox<T, _Dim>& aabb, const TScalarVector<U, _Dim>& quantized);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/ScalarBoundingBoxHelpers-inl.h"
