#include "stdafx.h"

#include "ScalarVector.h"
#include "ScalarVector_extern.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_EXTERN_TEMPLATE_SCALARVECTOR(_Scalar) \
    template class ScalarVector<_Scalar, 1>; \
    template class ScalarVector<_Scalar, 2>; \
    template class ScalarVector<_Scalar, 3>; \
    template class ScalarVector<_Scalar, 4>;
//----------------------------------------------------------------------------
DEF_EXTERN_TEMPLATE_SCALARVECTOR(i8);
DEF_EXTERN_TEMPLATE_SCALARVECTOR(i16);
DEF_EXTERN_TEMPLATE_SCALARVECTOR(i32);
DEF_EXTERN_TEMPLATE_SCALARVECTOR(i64);
DEF_EXTERN_TEMPLATE_SCALARVECTOR(u8);
DEF_EXTERN_TEMPLATE_SCALARVECTOR(u16);
DEF_EXTERN_TEMPLATE_SCALARVECTOR(u32);
DEF_EXTERN_TEMPLATE_SCALARVECTOR(u64);
DEF_EXTERN_TEMPLATE_SCALARVECTOR(float);
DEF_EXTERN_TEMPLATE_SCALARVECTOR(double);
//----------------------------------------------------------------------------
#undef DEF_EXTERN_TEMPLATE_SCALARVECTOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
