#pragma once

#include "Core/Core.h"

#include "Core/Allocator/PoolAllocatorTag.h"
#include "Core/IO/FileSystem_fwd.h"
#include "Core/IO/FS/Filename.h"

namespace Core {
namespace ContentPipeline {
POOLTAG_DECL(ContentPipeline);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ContentPipelineStartup {
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
