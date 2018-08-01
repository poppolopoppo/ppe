#pragma once

#include "Core.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
class TScalarMatrix;
//----------------------------------------------------------------------------
#define DECL_SCALARMATRIX_ALIAS(_Scalar, _ALIAS) \
    typedef TScalarMatrix<_Scalar, 2, 2> CONCAT(_ALIAS, 2x2); \
    typedef TScalarMatrix<_Scalar, 3, 3> CONCAT(_ALIAS, 3x3); \
    typedef TScalarMatrix<_Scalar, 4, 3> CONCAT(_ALIAS, 4x3); \
    typedef TScalarMatrix<_Scalar, 3, 4> CONCAT(_ALIAS, 3x4); \
    typedef TScalarMatrix<_Scalar, 4, 4> CONCAT(_ALIAS, 4x4);
#define DECL_SCALARMATRIX(_Scalar) DECL_SCALARMATRIX_ALIAS(_Scalar, _Scalar)
//----------------------------------------------------------------------------
DECL_SCALARMATRIX(byte);
DECL_SCALARMATRIX(ubyte);
DECL_SCALARMATRIX_ALIAS(i16, short);
DECL_SCALARMATRIX(ushort);
DECL_SCALARMATRIX(word);
DECL_SCALARMATRIX(uword);
DECL_SCALARMATRIX(int)
DECL_SCALARMATRIX(unsigned)
DECL_SCALARMATRIX(float)
DECL_SCALARMATRIX(double)
//----------------------------------------------------------------------------
#undef DECL_SCALARMATRIX
#undef DECL_SCALARMATRIX_ALIAS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef float4x4 Matrix;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
