#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class ScalarBoundingBox;
//----------------------------------------------------------------------------
typedef ScalarBoundingBox<int, 1> AABB1i;
typedef ScalarBoundingBox<int, 2> AABB2i;
typedef ScalarBoundingBox<int, 3> AABB3i;
typedef ScalarBoundingBox<int, 4> AABB4i;
//----------------------------------------------------------------------------
typedef ScalarBoundingBox<unsigned int, 1> AABB1u;
typedef ScalarBoundingBox<unsigned int, 2> AABB2u;
typedef ScalarBoundingBox<unsigned int, 3> AABB3u;
typedef ScalarBoundingBox<unsigned int, 4> AABB4u;
//----------------------------------------------------------------------------
typedef ScalarBoundingBox<float, 1> AABB1f;
typedef ScalarBoundingBox<float, 2> AABB2f;
typedef ScalarBoundingBox<float, 3> AABB3f;
typedef ScalarBoundingBox<float, 4> AABB4f;
//----------------------------------------------------------------------------
typedef ScalarBoundingBox<double, 1> AABB1d;
typedef ScalarBoundingBox<double, 2> AABB2d;
typedef ScalarBoundingBox<double, 3> AABB3d;
typedef ScalarBoundingBox<double, 4> AABB4d;
//----------------------------------------------------------------------------
typedef AABB3f BoundingBox;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class ScalarBoxWExtent;
//----------------------------------------------------------------------------
typedef ScalarBoxWExtent<int, 1> BoxWExtent1i;
typedef ScalarBoxWExtent<int, 2> BoxWExtent2i;
typedef ScalarBoxWExtent<int, 3> BoxWExtent3i;
typedef ScalarBoxWExtent<int, 4> BoxWExtent4i;
//----------------------------------------------------------------------------
typedef ScalarBoxWExtent<unsigned int, 1> BoxWExtent1u;
typedef ScalarBoxWExtent<unsigned int, 2> BoxWExtent2u;
typedef ScalarBoxWExtent<unsigned int, 3> BoxWExtent3u;
typedef ScalarBoxWExtent<unsigned int, 4> BoxWExtent4u;
//----------------------------------------------------------------------------
typedef ScalarBoxWExtent<float, 1> BoxWExtent1f;
typedef ScalarBoxWExtent<float, 2> BoxWExtent2f;
typedef ScalarBoxWExtent<float, 3> BoxWExtent3f;
typedef ScalarBoxWExtent<float, 4> BoxWExtent4f;
//----------------------------------------------------------------------------
typedef ScalarBoxWExtent<double, 1> BoxWExtent1d;
typedef ScalarBoxWExtent<double, 2> BoxWExtent2d;
typedef ScalarBoxWExtent<double, 3> BoxWExtent3d;
typedef ScalarBoxWExtent<double, 4> BoxWExtent4d;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
