#include "stdafx.h"

#include "ScalarBoundingBox.h"
#include "ScalarBoundingBox_extern.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template class ScalarBoundingBox<i32, 1>;
template class ScalarBoundingBox<i32, 2>;
template class ScalarBoundingBox<i32, 3>;
template class ScalarBoundingBox<i32, 4>;
//----------------------------------------------------------------------------
template class ScalarBoundingBox<u32, 1>;
template class ScalarBoundingBox<u32, 2>;
template class ScalarBoundingBox<u32, 3>;
template class ScalarBoundingBox<u32, 4>;
//----------------------------------------------------------------------------
template class ScalarBoundingBox<float, 1>;
template class ScalarBoundingBox<float, 2>;
template class ScalarBoundingBox<float, 3>;
template class ScalarBoundingBox<float, 4>;
//----------------------------------------------------------------------------
template class ScalarBoundingBox<double, 1>;
template class ScalarBoundingBox<double, 2>;
template class ScalarBoundingBox<double, 3>;
template class ScalarBoundingBox<double, 4>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
