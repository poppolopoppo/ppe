#include "stdafx.h"

#include "ContentPipeline.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

namespace Core {
namespace ContentPipeline {
POOLTAG_DEF(ContentPipeline);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ContentPipelineStartup::Start() {
    POOLTAG(ContentPipeline)::Start();
}
//----------------------------------------------------------------------------
void ContentPipelineStartup::Shutdown() {
    POOLTAG(ContentPipeline)::Shutdown();
}
//----------------------------------------------------------------------------
void ContentPipelineStartup::ClearAll_UnusedMemory() {
    POOLTAG(ContentPipeline)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
