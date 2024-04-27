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
bool IFrameGraph::RecreateResourceIFN(FBufferID& id,
    const FBufferDesc& newDesc,
    const FMemoryDesc& mem/* = Default*/
    ARGS_IF_RHIDEBUG(FConstChar debugName/* = Default */)) {
    if (Likely(id)) {
        const FBufferDesc& desc = Description(id);
        if (desc == newDesc)
            return false;
    }

    Unused(ReleaseResource(id));
    id = CreateBuffer(newDesc, mem ARGS_IF_RHIDEBUG(debugName));
    return true;
}
//----------------------------------------------------------------------------
bool IFrameGraph::RecreateResourceIFN(FImageID& id,
    FImageDesc&& newDesc,
    const FMemoryDesc& mem/* = Default*/
    ARGS_IF_RHIDEBUG(FConstChar debugName/* = Default */)) {
    if (Likely(id)) {
        newDesc.Validate();

        const FImageDesc& desc = Description(id);
        if (desc == newDesc)
            return false;
    }

    Unused(ReleaseResource(id));
    id = CreateImage(newDesc, mem ARGS_IF_RHIDEBUG(debugName));
    return true;
}
//----------------------------------------------------------------------------
bool IFrameGraph::RecreateResourceIFN(TAutoResource<FBufferID>& resource,
    const FBufferDesc& newDesc,
    const FMemoryDesc& mem/* = Default*/
    ARGS_IF_RHIDEBUG(FConstChar debugName/* = Default */)) {
    FBufferID id{ resource.Release() };
    const bool created = RecreateResourceIFN(id, newDesc, mem ARGS_IF_RHIDEBUG(debugName));
    resource.Reset(*this, std::move(id));
    return created;
}
//----------------------------------------------------------------------------
bool IFrameGraph::RecreateResourceIFN(TAutoResource<FImageID>& resource,
    FImageDesc&& newDesc,
    const FMemoryDesc& mem/* = Default*/
    ARGS_IF_RHIDEBUG(FConstChar debugName/* = Default */)) {
    FImageID id{ resource.Release() };
    const bool created = RecreateResourceIFN(id, std::move(newDesc), mem ARGS_IF_RHIDEBUG(debugName));
    resource.Reset(*this, std::move(id));
    return created;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
