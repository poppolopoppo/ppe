#pragma once

#include "Core/Core.h"

#include "Core/Maths/Geometry/ScalarBoundingBox_fwd.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern template class ScalarBoundingBox<i32, 1>;
extern template class ScalarBoundingBox<i32, 2>;
extern template class ScalarBoundingBox<i32, 3>;
extern template class ScalarBoundingBox<i32, 4>;
//----------------------------------------------------------------------------
extern template class ScalarBoundingBox<u32, 1>;
extern template class ScalarBoundingBox<u32, 2>;
extern template class ScalarBoundingBox<u32, 3>;
extern template class ScalarBoundingBox<u32, 4>;
//----------------------------------------------------------------------------
extern template class ScalarBoundingBox<float, 1>;
extern template class ScalarBoundingBox<float, 2>;
extern template class ScalarBoundingBox<float, 3>;
extern template class ScalarBoundingBox<float, 4>;
//----------------------------------------------------------------------------
extern template class ScalarBoundingBox<double, 1>;
extern template class ScalarBoundingBox<double, 2>;
extern template class ScalarBoundingBox<double, 3>;
extern template class ScalarBoundingBox<double, 4>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
#pragma once
