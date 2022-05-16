#pragma once

#include "Core.h"

#include "Maths/SHVector_fwd.h"

#ifndef EXPORT_PPE_RUNTIME_CORE_SHVECTOR
namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TSHVector<1>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TSHVector<2>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TSHVector<3>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TSHVector<4>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
#endif
