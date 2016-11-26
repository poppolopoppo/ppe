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
typedef TScalarBoxWExtent<int, 1> FBoxWExtent1i;
typedef TScalarBoxWExtent<int, 2> FBoxWExtent2i;
typedef TScalarBoxWExtent<int, 3> FBoxWExtent3i;
typedef TScalarBoxWExtent<int, 4> FBoxWExtent4i;
//----------------------------------------------------------------------------
typedef TScalarBoxWExtent<unsigned int, 1> FBoxWExtent1u;
typedef TScalarBoxWExtent<unsigned int, 2> FBoxWExtent2u;
typedef TScalarBoxWExtent<unsigned int, 3> FBoxWExtent3u;
typedef TScalarBoxWExtent<unsigned int, 4> FBoxWExtent4u;
//----------------------------------------------------------------------------
typedef TScalarBoxWExtent<float, 1> FBoxWExtent1f;
typedef TScalarBoxWExtent<float, 2> FBoxWExtent2f;
typedef TScalarBoxWExtent<float, 3> FBoxWExtent3f;
typedef TScalarBoxWExtent<float, 4> FBoxWExtent4f;
//----------------------------------------------------------------------------
typedef TScalarBoxWExtent<double, 1> FBoxWExtent1d;
typedef TScalarBoxWExtent<double, 2> FBoxWExtent2d;
typedef TScalarBoxWExtent<double, 3> FBoxWExtent3d;
typedef TScalarBoxWExtent<double, 4> FBoxWExtent4d;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
