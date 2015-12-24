#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
class ScalarMatrix;
//----------------------------------------------------------------------------
#define DECL_SCALARMATRIX_ALIAS(_Scalar, _ALIAS) \
    typedef ScalarMatrix<_Scalar, 2, 2> CONCAT(_ALIAS, 2x2); \
    typedef ScalarMatrix<_Scalar, 3, 3> CONCAT(_ALIAS, 3x3); \
    typedef ScalarMatrix<_Scalar, 4, 3> CONCAT(_ALIAS, 4x3); \
    typedef ScalarMatrix<_Scalar, 3, 4> CONCAT(_ALIAS, 3x4); \
    typedef ScalarMatrix<_Scalar, 4, 4> CONCAT(_ALIAS, 4x4);
#define DECL_SCALARMATRIX(_Scalar) DECL_SCALARMATRIX_ALIAS(_Scalar, _Scalar)
//----------------------------------------------------------------------------
DECL_SCALARMATRIX(byte);
DECL_SCALARMATRIX_ALIAS(i16, short);
DECL_SCALARMATRIX(word);
DECL_SCALARMATRIX_ALIAS(i32, int)
DECL_SCALARMATRIX(float)
DECL_SCALARMATRIX(double)
//----------------------------------------------------------------------------
typedef float4x4 Matrix;
//----------------------------------------------------------------------------
#undef DECL_SCALARMATRIX
#undef DECL_SCALARMATRIX_ALIAS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
