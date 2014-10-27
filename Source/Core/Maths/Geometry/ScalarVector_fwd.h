#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class ScalarVector;
//----------------------------------------------------------------------------
#define DECL_SCALARVECTOR_ALIAS(_Scalar, _ALIAS) \
    typedef ScalarVector<_Scalar, 2> CONCAT(_ALIAS, 2); \
    typedef ScalarVector<_Scalar, 3> CONCAT(_ALIAS, 3); \
    typedef ScalarVector<_Scalar, 4> CONCAT(_ALIAS, 4);
#define DECL_SCALARVECTOR(_Scalar) DECL_SCALARVECTOR_ALIAS(_Scalar, _Scalar)
//----------------------------------------------------------------------------
DECL_SCALARVECTOR(byte);
DECL_SCALARVECTOR(ubyte);
DECL_SCALARVECTOR_ALIAS(i16, short);
DECL_SCALARVECTOR(ushort);
DECL_SCALARVECTOR(word);
DECL_SCALARVECTOR(uword);
DECL_SCALARVECTOR(int)
DECL_SCALARVECTOR(unsigned)
DECL_SCALARVECTOR(float)
DECL_SCALARVECTOR(double)
//----------------------------------------------------------------------------
DECL_SCALARVECTOR(i32);
DECL_SCALARVECTOR(u32);
//----------------------------------------------------------------------------
#undef DECL_SCALARVECTOR
#undef DECL_SCALARVECTOR_ALIAS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef float3 Point;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
