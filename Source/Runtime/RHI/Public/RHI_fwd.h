#pragma once

#include "Core_fwd.h"

#ifdef EXPORT_PPE_RUNTIME_RHI
#   define PPE_RHI_API DLL_EXPORT
#else
#   define PPE_RHI_API DLL_IMPORT
#endif

#define USE_PPE_RHIDEBUG (USE_PPE_ASSERT || USE_PPE_MEMORY_DEBUGGING)

#include "HAL/RHI_fwd.h"
#include "Diagnostic/Logger_fwd.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FRHIException;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE