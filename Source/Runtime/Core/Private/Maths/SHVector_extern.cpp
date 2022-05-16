#include "stdafx.h"

#define EXPORT_PPE_RUNTIME_CORE_SHVECTOR

#include "Maths/SHVector.h"
#include "Maths/SHVector_extern.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TSHVector<1>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TSHVector<2>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TSHVector<3>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TSHVector<4>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
