#include "stdafx.h"

#include "ScalarVector.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
    /*
#define DEF_SCALARVECTOR(_Scalar) \
    template class TScalarVector<_Scalar, 2>; \
    template class TScalarVector<_Scalar, 3>; \
    template class TScalarVector<_Scalar, 4>;
//----------------------------------------------------------------------------
DEF_SCALARVECTOR(int)
DEF_SCALARVECTOR(unsigned)
DEF_SCALARVECTOR(float)
DEF_SCALARVECTOR(double)
//----------------------------------------------------------------------------
#undef DEF_SCALARVECTOR
*/
//----------------------------------------------------------------------------
STATIC_ASSERT(Meta::has_forceinit_constructor<uint2>::value);
STATIC_ASSERT(Meta::has_forceinit_constructor<word3>::value);
STATIC_ASSERT(Meta::has_forceinit_constructor<float4>::value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
