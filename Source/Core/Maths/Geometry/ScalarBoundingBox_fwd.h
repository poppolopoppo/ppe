#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class ScalarBoundingBox;
//----------------------------------------------------------------------------
typedef ScalarBoundingBox<i32, 1> AABB1i;
typedef ScalarBoundingBox<i32, 2> AABB2i;
typedef ScalarBoundingBox<i32, 3> AABB3i;
typedef ScalarBoundingBox<i32, 4> AABB4i;
//----------------------------------------------------------------------------
typedef ScalarBoundingBox<u32, 1> AABB1u;
typedef ScalarBoundingBox<u32, 2> AABB2u;
typedef ScalarBoundingBox<u32, 3> AABB3u;
typedef ScalarBoundingBox<u32, 4> AABB4u;
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
} //!namespace Core
