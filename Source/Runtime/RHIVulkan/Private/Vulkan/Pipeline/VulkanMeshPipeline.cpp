
#include "stdafx.h"

#include "Vulkan/Pipeline/VulkanMeshPipeline.h"

#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Instance/VulkanResourceManager.h"
#include "Vulkan/Pipeline/VulkanShaderModule.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FVulkanMeshPipeline::FPipelineInstance::Invalidate() {
    HashValue = hash_tuple(
        LayoutId,
        RenderPassId,
        RenderState,
        DynamicState,
        SubpassIndex,
        ViewportCount
        ARGS_IF_RHIDEBUG(DebugMode) );
}
//----------------------------------------------------------------------------
FVulkanMeshPipeline::~FVulkanMeshPipeline() {
    ONLY_IF_RHIDEBUG(Assert_NoAssume(_pipeline.LockExclusive()->Shaders.empty()));
    ONLY_IF_RHIDEBUG(FReadWriteLock::FScopeLockWrite scopeWrite{ _instanceRWLock });
    ONLY_IF_RHIDEBUG(Assert_NoAssume(_instanceMap.empty()));
}
//----------------------------------------------------------------------------
bool FVulkanMeshPipeline::Construct(const FMeshPipelineDesc& desc, FRawPipelineLayoutID layoutId, FConstChar debugName) {
    Assert(layoutId);

    const auto exclusive = _pipeline.LockExclusive();

    exclusive->BaseLayoutId = FPipelineLayoutID{ layoutId };
    exclusive->Topology = desc.Topology;
    exclusive->EarlyFragmentTests = desc.EarlyFragmentTests;

    ONLY_IF_RHIDEBUG(_debugName = debugName);

    for (const auto& stage : desc.Shaders) {
        const VkShaderStageFlagBits vkStage = VkCast(stage.first);

        for (const auto& src : stage.second.Sources) {
            const PShaderModule& moduleRef = std::get<PShaderModule>(*src.second->Data());
            Assert(moduleRef);

            exclusive->Shaders.Push(
                checked_cast<FVulkanShaderModule>(moduleRef),
                vkStage
                ARGS_IF_RHIDEBUG(EShaderDebugMode_From(src.first)) );
        }
    }

    return (not exclusive->Shaders.empty());
}
//----------------------------------------------------------------------------
void FVulkanMeshPipeline::TearDown(FVulkanResourceManager& resources) {
    const FVulkanDevice& device = resources.Device();

    const auto exclusive = _pipeline.LockExclusive();

    {
        const FReadWriteLock::FScopeLockWrite lockInstanceW{ _instanceRWLock };

        for (auto& ppln : _instanceMap) {
            device.vkDestroyPipeline(device.vkDevice(), ppln.second, device.vkAllocator());
            resources.ReleaseResource(ppln.first.LayoutId);
        }

        _instanceMap.clear();
    }


    if (exclusive->BaseLayoutId)
        resources.ReleaseResource(exclusive->BaseLayoutId.Release());

    exclusive->Shaders.clear();
    ONLY_IF_RHIDEBUG(_debugName.Clear());

    exclusive->BaseLayoutId = Default;
    exclusive->Topology = Default;
    exclusive->EarlyFragmentTests = false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
