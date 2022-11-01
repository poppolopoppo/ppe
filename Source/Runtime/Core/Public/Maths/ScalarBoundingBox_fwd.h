#pragma once

#include "Core.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Dim>
class TScalarBoundingBox;
//----------------------------------------------------------------------------
typedef TScalarBoundingBox<int, 1> FAabb1i;
typedef TScalarBoundingBox<int, 2> FAabb2i;
typedef TScalarBoundingBox<int, 3> FAabb3i;
typedef TScalarBoundingBox<int, 4> FAabb4i;
//----------------------------------------------------------------------------
typedef TScalarBoundingBox<unsigned int, 1> FAabb1u;
typedef TScalarBoundingBox<unsigned int, 2> FAabb2u;
typedef TScalarBoundingBox<unsigned int, 3> FAabb3u;
typedef TScalarBoundingBox<unsigned int, 4> FAabb4u;
//----------------------------------------------------------------------------
typedef TScalarBoundingBox<float, 1> FAabb1f;
typedef TScalarBoundingBox<float, 2> FAabb2f;
typedef TScalarBoundingBox<float, 3> FAabb3f;
typedef TScalarBoundingBox<float, 4> FAabb4f;
//----------------------------------------------------------------------------
typedef TScalarBoundingBox<double, 1> FAabb1d;
typedef TScalarBoundingBox<double, 2> FAabb2d;
typedef TScalarBoundingBox<double, 3> FAabb3d;
typedef TScalarBoundingBox<double, 4> FAabb4d;
//----------------------------------------------------------------------------
typedef FAabb3f FBoundingBox;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Dim>
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
} //!namespace PPE
