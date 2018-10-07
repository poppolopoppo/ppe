#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_BUILDGRAPH
#   define PPE_BUILDGRAPH_API DLL_EXPORT
#else
#   define PPE_BUILDGRAPH_API DLL_IMPORT
#endif

#include "Memory/RefPtr.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(BuildNode);
FWD_REFPTR(BuildGraph);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
