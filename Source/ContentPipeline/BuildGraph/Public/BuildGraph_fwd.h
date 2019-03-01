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
enum class EBuildResult;
class PPE_BUILDGRAPH_API FBuildContext;
class PPE_BUILDGRAPH_API IBuildExecutor;
FWD_REFPTR(BuildGraph);
FWD_REFPTR(BuildNode);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
