#pragma once

#include "Core/Maths/ScalarBoundingBoxHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T Area(const TScalarBoundingBox<T, 2>& aabb) {
    const TScalarVector<T, 2> extents = aabb.Extents();
    return (extents.x() * extents.y());
}
//----------------------------------------------------------------------------
template <typename T>
T Area(const TScalarBoxWExtent<T, 2>& aabb) {
    const TScalarVector<T, 2> halfExtents = aabb.HalfExtents();
    return (halfExtents.x() * halfExtents.y() * 4);
}
//----------------------------------------------------------------------------
template <typename T>
T Volume(const TScalarBoundingBox<T, 3>& aabb) {
    const TScalarVector<T, 3> extents = aabb.Extents();
    return (extents.x() * extents.y() * extents.z());
}
//----------------------------------------------------------------------------
template <typename T>
T Volume(const TScalarBoxWExtent<T, 3>& aabb) {
    const TScalarVector<T, 3> halfExtents = aabb.HalfExtents();
    return (halfExtents.x() * halfExtents.y() * halfExtents.z() * 8);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<float, _Dim> LinearStep(const TScalarBoundingBox<T, _Dim>& aabb, const TScalarVector<T, _Dim>& value) {
    return LinearStep(value, aabb.Min(), aabb.Max());
}
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
TScalarVector<U, _Dim> QuantizeCeil(const TScalarBoundingBox<T, _Dim>& aabb, const TScalarVector<T, _Dim>& value) {
    TScalarVector<float, _Dim> normalized = LinearStep(aabb, value);
    normalized += 1 / static_cast<float>(TNumericLimits<U>::MaxValue() - TNumericLimits<U>::MinValue());
    normalized = Saturate(normalized);
    return TLerp(TScalarVector<U, _Dim>::MinValue(), TScalarVector<U, _Dim>::MaxValue(), normalized);
}
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
TScalarVector<U, _Dim> QuantizeFloor(const TScalarBoundingBox<T, _Dim>& aabb, const TScalarVector<T, _Dim>& value) {
    TScalarVector<float, _Dim> normalized = LinearStep(aabb, value);
    normalized -= 1 / static_cast<float>(TNumericLimits<U>::MaxValue() - TNumericLimits<U>::MinValue());
    normalized = Saturate(normalized);
    return TLerp(TScalarVector<U, _Dim>::MinValue(), TScalarVector<U, _Dim>::MaxValue(), normalized);
}
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
TScalarVector<U, _Dim> QuantizeRound(const TScalarBoundingBox<T, _Dim>& aabb, const TScalarVector<T, _Dim>& value) {
    TScalarVector<float, _Dim> normalized = LinearStep(aabb, value);
    normalized += 0.5f / static_cast<float>(TNumericLimits<U>::MaxValue() - TNumericLimits<U>::MinValue());
    normalized = Saturate(normalized);
    return TLerp(TScalarVector<U, _Dim>::MinValue(), TScalarVector<U, _Dim>::MaxValue(), normalized);
}
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
TScalarVector<T, _Dim> Unquantize(const TScalarBoundingBox<T, _Dim>& aabb, const TScalarVector<U, _Dim>& quantized) {
    return aabb.Lerp( LinearStep(TScalarBoundingBox<U, _Dim>::MinMax(), quantized) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarBoundingBox<T, _Dim> MakeBoundingBox(const TMemoryView<const TScalarVector<T, _Dim>>& points) {
    TScalarBoundingBox<T, _Dim> result;
    result.AddRange(points.begin(), points.end());
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
