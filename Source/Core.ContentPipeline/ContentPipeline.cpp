#include "stdafx.h"

#include "ContentPipeline.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

#include "Core.RTTI/RTTI_Tag-impl.h"

#ifdef OS_WINDOWS
#   pragma warning(disable: 4073) // initialiseurs placés dans la zone d'initialisation d'une bibliothèque
#   pragma init_seg(lib)
#else
#   error "missing compiler specific command"
#endif

namespace Core {
namespace ContentPipeline {
POOL_TAG_DEF(ContentPipeline);
RTTI_TAG_DEF(ContentPipeline);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ContentPipelineStartup::Start() {
    POOL_TAG(ContentPipeline)::Start();
    RTTI_TAG(ContentPipeline)::Start();
}
//----------------------------------------------------------------------------
void ContentPipelineStartup::Shutdown() {
    RTTI_TAG(ContentPipeline)::Shutdown();
    POOL_TAG(ContentPipeline)::Shutdown();
}
//----------------------------------------------------------------------------
void ContentPipelineStartup::ClearAll_UnusedMemory() {
    POOL_TAG(ContentPipeline)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
