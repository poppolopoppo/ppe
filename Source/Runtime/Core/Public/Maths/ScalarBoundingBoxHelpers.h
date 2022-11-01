#pragma once

#include "Core.h"

#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"

#include "HAL/PlatformEndian.h"

namespace PPE {
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
template <typename T, u32 _Dim>
TScalarVector<float, _Dim> LinearStep(const TScalarBoundingBox<T, _Dim>& aabb, const TScalarVector<T, _Dim>& value);
//----------------------------------------------------------------------------
template <typename U, typename T, u32 _Dim>
TScalarVector<U, _Dim> QuantizeCeil(const TScalarBoundingBox<T, _Dim>& aabb, const TScalarVector<T, _Dim>& value);
//----------------------------------------------------------------------------
template <typename U, typename T, u32 _Dim>
TScalarVector<U, _Dim> QuantizeFloor(const TScalarBoundingBox<T, _Dim>& aabb, const TScalarVector<T, _Dim>& value);
//----------------------------------------------------------------------------
template <typename U, typename T, u32 _Dim>
TScalarVector<U, _Dim> QuantizeRound(const TScalarBoundingBox<T, _Dim>& aabb, const TScalarVector<T, _Dim>& value);
//----------------------------------------------------------------------------
template <typename U, typename T, u32 _Dim>
TScalarVector<T, _Dim> Unquantize(const TScalarBoundingBox<T, _Dim>& aabb, const TScalarVector<U, _Dim>& quantized);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Dim>
TScalarBoundingBox<T, _Dim> MakeBoundingBox(const TMemoryView<TScalarVector<T, _Dim>>& points);
//----------------------------------------------------------------------------
template <typename T, u32 _Dim>
TScalarBoundingBox<T, _Dim> MakeBoundingBox(const TMemoryView<const TScalarVector<T, _Dim>>& points);
//----------------------------------------------------------------------------
template <typename T, u32 _Dim>
TScalarBoundingBox<T, _Dim> MakeBoundingBox(const TMemoryView<TScalarBoundingBox<T, _Dim>>& aabbs);
//----------------------------------------------------------------------------
template <typename T, u32 _Dim>
TScalarBoundingBox<T, _Dim> MakeBoundingBox(const TMemoryView<const TScalarBoundingBox<T, _Dim>>& aabbs);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Dim>
void SwapEndiannessInPlace(TScalarBoundingBox<T, _Dim>* value) {
    SwapEndiannessInPlace(value->Min());
    SwapEndiannessInPlace(value->Max());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/ScalarBoundingBoxHelpers-inl.h"
