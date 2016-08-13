#pragma once

#include "Core/Maths/ScalarBoundingBoxHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T Area(const ScalarBoundingBox<T, 2>& aabb) {
    const ScalarVector<T, 2> extents = aabb.Extents();
    return (extents.x() * extents.y());
}
//----------------------------------------------------------------------------
template <typename T>
T Area(const ScalarBoxWExtent<T, 2>& aabb) {
    const ScalarVector<T, 2> halfExtents = aabb.HalfExtents();
    return (halfExtents.x() * halfExtents.y() * 4);
}
//----------------------------------------------------------------------------
template <typename T>
T Volume(const ScalarBoundingBox<T, 3>& aabb) {
    const ScalarVector<T, 3> extents = aabb.Extents();
    return (extents.x() * extents.y() * extents.z());
}
//----------------------------------------------------------------------------
template <typename T>
T Volume(const ScalarBoxWExtent<T, 3>& aabb) {
    const ScalarVector<T, 3> halfExtents = aabb.HalfExtents();
    return (halfExtents.x() * halfExtents.y() * halfExtents.z() * 8);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<float, _Dim> LinearStep(const ScalarBoundingBox<T, _Dim>& aabb, const ScalarVector<T, _Dim>& value) {
    return LinearStep(value, aabb.Min(), aabb.Max());
}
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
ScalarVector<U, _Dim> QuantizeCeil(const ScalarBoundingBox<T, _Dim>& aabb, const ScalarVector<T, _Dim>& value) {
    ScalarVector<float, _Dim> normalized = LinearStep(aabb, value);
    normalized += 1 / static_cast<float>(NumericLimits<U>::MaxValue() - NumericLimits<U>::MinValue());
    normalized = Saturate(normalized);
    return Lerp(ScalarVector<U, _Dim>::MinValue(), ScalarVector<U, _Dim>::MaxValue(), normalized);
}
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
ScalarVector<U, _Dim> QuantizeFloor(const ScalarBoundingBox<T, _Dim>& aabb, const ScalarVector<T, _Dim>& value) {
    ScalarVector<float, _Dim> normalized = LinearStep(aabb, value);
    normalized -= 1 / static_cast<float>(NumericLimits<U>::MaxValue() - NumericLimits<U>::MinValue());
    normalized = Saturate(normalized);
    return Lerp(ScalarVector<U, _Dim>::MinValue(), ScalarVector<U, _Dim>::MaxValue(), normalized);
}
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
ScalarVector<U, _Dim> QuantizeRound(const ScalarBoundingBox<T, _Dim>& aabb, const ScalarVector<T, _Dim>& value) {
    ScalarVector<float, _Dim> normalized = LinearStep(aabb, value);
    normalized += 0.5f / static_cast<float>(NumericLimits<U>::MaxValue() - NumericLimits<U>::MinValue());
    normalized = Saturate(normalized);
    return Lerp(ScalarVector<U, _Dim>::MinValue(), ScalarVector<U, _Dim>::MaxValue(), normalized);
}
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
ScalarVector<T, _Dim> Unquantize(const ScalarBoundingBox<T, _Dim>& aabb, const ScalarVector<U, _Dim>& quantized) {
    return aabb.Lerp( LinearStep(ScalarBoundingBox<U, _Dim>::MinMax(), quantized) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
