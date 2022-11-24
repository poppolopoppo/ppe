// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Vulkan/Pipeline/VulkanGraphicsPipeline.h"

#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Instance/VulkanResourceManager.h"
#include "Vulkan/Pipeline/VulkanShaderModule.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FVulkanGraphicsPipeline::FPipelineInstance::Invalidate() {
    HashValue = hash_tuple(
        LayoutId,
        RenderPassId,
        RenderState,
        VertexInput,
        DynamicState,
        SubpassIndex,
        ViewportCount
        ARGS_IF_RHIDEBUG(DebugMode) );
}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FVulkanGraphicsPipeline::~FVulkanGraphicsPipeline() {
    ONLY_IF_RHIDEBUG(Assert_NoAssume(_pipeline.LockExclusive()->Shaders.empty()));
    Assert_NoAssume(_sharedInstances.LockExclusive()->empty());
}
#endif
//----------------------------------------------------------------------------
bool FVulkanGraphicsPipeline::Construct(const FGraphicsPipelineDesc& desc, FRawPipelineLayoutID layoutId ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    Assert(layoutId);

    const auto exclusive = _pipeline.LockExclusive();

    exclusive->BaseLayoutId = FPipelineLayoutID{ layoutId };
    exclusive->SupportedTopology = desc.SupportedTopology;
    exclusive->VertexAttributes = desc.VertexAttributes;
    exclusive->PatchControlPoints = desc.PatchControlPoints;
    exclusive->EarlyFragmentTests = desc.EarlyFragmentTests;

    ONLY_IF_RHIDEBUG(_debugName = debugName);

    for (const auto& stage : desc.Shaders) {
        const VkShaderStageFlagBits vkStage = VkCast(stage.first);

        for (const auto& src : stage.second.Data) {
            const PShaderModule& moduleRef = std::get<PShaderModule>(src.second);
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
void FVulkanGraphicsPipeline::TearDown(FVulkanResourceManager& resources) {
    const FVulkanDevice& device = resources.Device();

    const auto exclusive = _pipeline.LockExclusive();

    {
        const auto exclusiveInstances = _sharedInstances.LockExclusive();

        for (auto& ppln : *exclusiveInstances) {
            device.vkDestroyPipeline(device.vkDevice(), ppln.second, device.vkAllocator());
            resources.ReleaseResource(ppln.first.LayoutId);
        }

        exclusiveInstances->clear();
    }

    if (exclusive->BaseLayoutId)
        resources.ReleaseResource(exclusive->BaseLayoutId.Release());

    exclusive->Shaders.clear();
    exclusive->VertexAttributes.clear();
    ONLY_IF_RHIDEBUG(_debugName.Clear());

    exclusive->BaseLayoutId = Default;
    exclusive->SupportedTopology = Default;
    exclusive->PatchControlPoints = 0;
    exclusive->EarlyFragmentTests = false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
