#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class TScalarBoundingBox;
//----------------------------------------------------------------------------
typedef TScalarBoundingBox<int, 1> AABB1i;
typedef TScalarBoundingBox<int, 2> AABB2i;
typedef TScalarBoundingBox<int, 3> AABB3i;
typedef TScalarBoundingBox<int, 4> AABB4i;
//----------------------------------------------------------------------------
typedef TScalarBoundingBox<unsigned int, 1> AABB1u;
typedef TScalarBoundingBox<unsigned int, 2> AABB2u;
typedef TScalarBoundingBox<unsigned int, 3> AABB3u;
typedef TScalarBoundingBox<unsigned int, 4> AABB4u;
//----------------------------------------------------------------------------
typedef TScalarBoundingBox<float, 1> AABB1f;
typedef TScalarBoundingBox<float, 2> AABB2f;
typedef TScalarBoundingBox<float, 3> AABB3f;
typedef TScalarBoundingBox<float, 4> AABB4f;
//----------------------------------------------------------------------------
typedef TScalarBoundingBox<double, 1> AABB1d;
typedef TScalarBoundingBox<double, 2> AABB2d;
typedef TScalarBoundingBox<double, 3> AABB3d;
typedef TScalarBoundingBox<double, 4> AABB4d;
//----------------------------------------------------------------------------
typedef AABB3f BoundingBox;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class TScalarBoxWExtent;
//----------------------------------------------------------------------------
typedef TScalarBoxWExtent<int, 1> BoxWExtent1i;
typedef TScalarBoxWExtent<int, 2> BoxWExtent2i;
typedef TScalarBoxWExtent<int, 3> BoxWExtent3i;
typedef TScalarBoxWExtent<int, 4> BoxWExtent4i;
//----------------------------------------------------------------------------
typedef TScalarBoxWExtent<unsigned int, 1> BoxWExtent1u;
typedef TScalarBoxWExtent<unsigned int, 2> BoxWExtent2u;
typedef TScalarBoxWExtent<unsigned int, 3> BoxWExtent3u;
typedef TScalarBoxWExtent<unsigned int, 4> BoxWExtent4u;
//----------------------------------------------------------------------------
typedef TScalarBoxWExtent<float, 1> BoxWExtent1f;
typedef TScalarBoxWExtent<float, 2> BoxWExtent2f;
typedef TScalarBoxWExtent<float, 3> BoxWExtent3f;
typedef TScalarBoxWExtent<float, 4> BoxWExtent4f;
//----------------------------------------------------------------------------
typedef TScalarBoxWExtent<double, 1> BoxWExtent1d;
typedef TScalarBoxWExtent<double, 2> BoxWExtent2d;
typedef TScalarBoxWExtent<double, 3> BoxWExtent3d;
typedef TScalarBoxWExtent<double, 4> BoxWExtent4d;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
