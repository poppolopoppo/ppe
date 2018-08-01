#include "stdafx.h"

#include "ContentPipeline.h"

#include "Allocator/PoolAllocatorTag-impl.h"

#include "RTTI_Namespace-impl.h"

PRAGMA_INITSEG_LIB

namespace PPE {
namespace ContentPipeline {
POOL_TAG_DEF(ContentPipeline);
RTTI_NAMESPACE_DEF(PPE_CONTENTPIPELINE_API, ContentPipeline);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FContentPipelineModule::Start() {
    PPE_MODULE_START(ContentPipeline);

    POOL_TAG(ContentPipeline)::Start();
    RTTI_NAMESPACE(ContentPipeline).Start();
}
//----------------------------------------------------------------------------
void FContentPipelineModule::Shutdown() {
    PPE_MODULE_SHUTDOWN(ContentPipeline);

    RTTI_NAMESPACE(ContentPipeline).Shutdown();
    POOL_TAG(ContentPipeline)::Shutdown();
}
//----------------------------------------------------------------------------
void FContentPipelineModule::ClearAll_UnusedMemory() {
    PPE_MODULE_CLEARALL(ContentPipeline);

    POOL_TAG(ContentPipeline)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
