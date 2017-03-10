#include "stdafx.h"

#include "ContentPipeline.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

#include "Core.RTTI/RTTI_Tag-impl.h"

PRAGMA_INITSEG_LIB

namespace Core {
namespace ContentPipeline {
POOL_TAG_DEF(ContentPipeline);
RTTI_TAG_DEF(ContentPipeline);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ContentPipelineStartup::Start() {
    CORE_MODULE_START(ContentPipeline);

    POOL_TAG(ContentPipeline)::Start();
    RTTI_TAG(ContentPipeline)::Start();
}
//----------------------------------------------------------------------------
void ContentPipelineStartup::Shutdown() {
    CORE_MODULE_SHUTDOWN(ContentPipeline);

    RTTI_TAG(ContentPipeline)::Shutdown();
    POOL_TAG(ContentPipeline)::Shutdown();
}
//----------------------------------------------------------------------------
void ContentPipelineStartup::ClearAll_UnusedMemory() {
    CORE_MODULE_CLEARALL(ContentPipeline);

    POOL_TAG(ContentPipeline)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
