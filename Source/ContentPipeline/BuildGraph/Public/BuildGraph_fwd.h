#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_RUNTIME_BUILDGRAPH
#   define PPE_BUILDGRAPH_API DLL_EXPORT
#else
#   define PPE_BUILDGRAPH_API DLL_IMPORT
#endif

#include "Memory/RefPtr.h"
#include "Memory/UniquePtr.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FBuildFingerpint = u128;
using FBuildRevision = size_t;
using FBuildUniqueId = size_t;
//----------------------------------------------------------------------------
enum class EBuildFlags;
enum class EBuildResult;
//----------------------------------------------------------------------------
FWD_REFPTR(BuildNode);
FWD_REFPTR(FileNode);
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FBuildEnvironment;
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FPipelineContext;
    class PPE_BUILDGRAPH_API FBuildContext;
    class PPE_BUILDGRAPH_API FCleanContext;
    class PPE_BUILDGRAPH_API FScanContext;
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(BuildCache);
FWD_INTEFARCE_UNIQUEPTR(BuildExecutor);
FWD_INTEFARCE_UNIQUEPTR(BuildLog);
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FBuildGraph;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
