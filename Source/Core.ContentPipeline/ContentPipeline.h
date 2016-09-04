#pragma once

#include "Core/Core.h"

#ifdef EXPORT_CORE_CONTENTPIPELINE
#   define CORE_CONTENTPIPELINE_API DLL_EXPORT
#else
#   define CORE_CONTENTPIPELINE_API DLL_IMPORT
#endif

#include "Core/Allocator/PoolAllocatorTag.h"
#include "Core/IO/FileSystem_fwd.h"
#include "Core/IO/FS/Filename.h"

#include "Core.RTTI/RTTI_Tag.h"

namespace Core {
namespace ContentPipeline {
POOL_TAG_DECL(ContentPipeline);
RTTI_TAG_DECL(ContentPipeline);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_CONTENTPIPELINE_API ContentPipelineStartup {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    ContentPipelineStartup() { Start(); }
    ~ContentPipelineStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
