#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_CONTENTPIPELINE
#   define PPE_CONTENTPIPELINE_API DLL_EXPORT
#else
#   define PPE_CONTENTPIPELINE_API DLL_IMPORT
#endif

#include "Allocator/PoolAllocatorTag.h"
#include "IO/FileSystem_fwd.h"
#include "IO/FS/Filename.h"

#include "RTTI_Namespace.h"

namespace PPE {
namespace ContentPipeline {
POOL_TAG_DECL(ContentPipeline);
RTTI_NAMESPACE_DECL(PPE_CONTENTPIPELINE_API, ContentPipeline);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CONTENTPIPELINE_API FContentPipelineModule {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    FContentPipelineModule() { Start(); }
    ~FContentPipelineModule() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
