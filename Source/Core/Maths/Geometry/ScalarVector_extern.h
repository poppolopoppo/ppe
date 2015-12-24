#pragma once

#include "Core/Core.h"

#include "Core/Maths/Geometry/ScalarVector_fwd.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DECL_EXTERN_TEMPLATE_SCALARVECTOR(_Scalar) \
    extern template class ScalarVector<_Scalar, 1>; \
    extern template class ScalarVector<_Scalar, 2>; \
    extern template class ScalarVector<_Scalar, 3>; \
    extern template class ScalarVector<_Scalar, 4>;
//----------------------------------------------------------------------------
DECL_EXTERN_TEMPLATE_SCALARVECTOR(i8);
DECL_EXTERN_TEMPLATE_SCALARVECTOR(i16);
DECL_EXTERN_TEMPLATE_SCALARVECTOR(i32);
DECL_EXTERN_TEMPLATE_SCALARVECTOR(i64);
DECL_EXTERN_TEMPLATE_SCALARVECTOR(u8);
DECL_EXTERN_TEMPLATE_SCALARVECTOR(u16);
DECL_EXTERN_TEMPLATE_SCALARVECTOR(u32);
DECL_EXTERN_TEMPLATE_SCALARVECTOR(u64);
DECL_EXTERN_TEMPLATE_SCALARVECTOR(float);
DECL_EXTERN_TEMPLATE_SCALARVECTOR(double);
//----------------------------------------------------------------------------
#undef DECL_EXTERN_TEMPLATE_SCALARVECTOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
