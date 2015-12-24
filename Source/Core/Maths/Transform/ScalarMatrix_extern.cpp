#include "stdafx.h"

#include "ScalarMatrix.h"
#include "ScalarMatrix_extern.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_EXTERN_TEMPLATE_SCALARMATRIX(_Scalar) \
    template class ScalarMatrix<_Scalar, 2, 2>; \
    template class ScalarMatrix<_Scalar, 3, 3>; \
    template class ScalarMatrix<_Scalar, 4, 3>; \
    template class ScalarMatrix<_Scalar, 3, 4>; \
    template class ScalarMatrix<_Scalar, 4, 4>;
//----------------------------------------------------------------------------
DEF_EXTERN_TEMPLATE_SCALARMATRIX(i8);
DEF_EXTERN_TEMPLATE_SCALARMATRIX(i16);
DEF_EXTERN_TEMPLATE_SCALARMATRIX(i32);
DEF_EXTERN_TEMPLATE_SCALARMATRIX(i64);
DEF_EXTERN_TEMPLATE_SCALARMATRIX(float);
DEF_EXTERN_TEMPLATE_SCALARMATRIX(double);
//----------------------------------------------------------------------------
typedef float4x4 Matrix;
//----------------------------------------------------------------------------
#undef DEF_EXTERN_TEMPLATE_SCALARMATRIX
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
