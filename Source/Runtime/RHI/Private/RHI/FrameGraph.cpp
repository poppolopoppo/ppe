// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RHI/FrameGraph.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <u32 _Id>
NODISCARD PPipelineResources CreatePipelineResources_(
    const IFrameGraph& fg, details::TResourceId<_Id> pipeline, const FDescriptorSetID& id) {
    auto resources = NEW_REF(RHIPipeline, FPipelineResources);
    PPE_LOG_CHECK(RHI, fg.InitPipelineResources(resources.get(), pipeline, id));
    return resources;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPipelineResources IFrameGraph::CreatePipelineResources(FRawGPipelineID pipeline, const FDescriptorSetID& id) const {
    return CreatePipelineResources_(*this, pipeline, id);
}
//----------------------------------------------------------------------------
PPipelineResources IFrameGraph::CreatePipelineResources(FRawCPipelineID pipeline, const FDescriptorSetID& id) const {
    return CreatePipelineResources_(*this, pipeline, id);
}
//----------------------------------------------------------------------------
PPipelineResources IFrameGraph::CreatePipelineResources(FRawMPipelineID pipeline, const FDescriptorSetID& id) const {
    return CreatePipelineResources_(*this, pipeline, id);
}
//----------------------------------------------------------------------------
PPipelineResources IFrameGraph::CreatePipelineResources(FRawRTPipelineID pipeline, const FDescriptorSetID& id) const {
    return CreatePipelineResources_(*this, pipeline, id);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
