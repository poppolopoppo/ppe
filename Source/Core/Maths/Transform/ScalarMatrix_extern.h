#pragma once

#include "Core/Core.h"

#include "Core/Maths/Transform/ScalarMatrix_fwd.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DECL_EXTERN_TEMPLATE_SCALARMATRIX(_Scalar) \
    extern template class ScalarMatrix<_Scalar, 2, 2>; \
    extern template class ScalarMatrix<_Scalar, 3, 3>; \
    extern template class ScalarMatrix<_Scalar, 4, 3>; \
    extern template class ScalarMatrix<_Scalar, 3, 4>; \
    extern template class ScalarMatrix<_Scalar, 4, 4>;
//----------------------------------------------------------------------------
DECL_EXTERN_TEMPLATE_SCALARMATRIX(i8);
DECL_EXTERN_TEMPLATE_SCALARMATRIX(i16);
DECL_EXTERN_TEMPLATE_SCALARMATRIX(i32);
DECL_EXTERN_TEMPLATE_SCALARMATRIX(i64);
DECL_EXTERN_TEMPLATE_SCALARMATRIX(float);
DECL_EXTERN_TEMPLATE_SCALARMATRIX(double);
//----------------------------------------------------------------------------
typedef float4x4 Matrix;
//----------------------------------------------------------------------------
#undef DECL_EXTERN_TEMPLATE_SCALARMATRIX
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
