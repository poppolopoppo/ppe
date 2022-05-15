
#include "stdafx.h"

#include "Vulkan/Pipeline/VulkanRayTracingPipeline.h"

#include "Vulkan/Instance/VulkanResourceManager.h"
#include "Vulkan/Pipeline/VulkanShaderModule.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FVulkanRayTracingPipeline::~FVulkanRayTracingPipeline() {
    ONLY_IF_RHIDEBUG(Assert_NoAssume(not _pipeline.LockExclusive()->BaseLayoutId));
}
#endif
//----------------------------------------------------------------------------
bool FVulkanRayTracingPipeline::Construct(const FRayTracingPipelineDesc& desc, FRawPipelineLayoutID layoutId ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(layoutId);

    const auto exclusive = _pipeline.LockExclusive();

    exclusive->BaseLayoutId = FPipelineLayoutID{ layoutId };
    ONLY_IF_RHIDEBUG(exclusive->DebugModeBits = Default);
    ONLY_IF_RHIDEBUG(_debugName = debugName);

    for (const auto& stage : desc.Shaders) {
        const VkShaderStageFlagBits vkStage = VkCast(stage.second.Type);

        for (const auto& src : stage.second.Data) {
            const PShaderModule& moduleRef = std::get<PShaderModule>(src.second);
            Assert(moduleRef);

            bool added = false;
            const auto it = exclusive->Shaders.FindOrAdd({
                stage.first,
                checked_cast<FVulkanShaderModule>(moduleRef),
                vkStage,
                stage.second.Specializations
                ARGS_IF_RHIDEBUG(EShaderDebugMode_From(src.first)) }, &added);
            Assert_NoAssume(added);
            Unused(it);
            ONLY_IF_RHIDEBUG(exclusive->DebugModeBits.set(static_cast<u32>(it->DebugMode)));
        }
    }

    std::sort(exclusive->Shaders.begin(), exclusive->Shaders.end(),
        [](const FShaderModule& lhs, const FShaderModule& rhs) NOEXCEPT {
            return (lhs.ShaderId < rhs.ShaderId);
        });

    return (not exclusive->Shaders.empty());
}
//----------------------------------------------------------------------------
void FVulkanRayTracingPipeline::TearDown(FVulkanResourceManager& resources) {
    const auto exclusive = _pipeline.LockExclusive();

    if (exclusive->BaseLayoutId)
        resources.ReleaseResource(exclusive->BaseLayoutId.Release());

    exclusive->Shaders.clear();
    ONLY_IF_RHIDEBUG(_debugName.Clear());

    exclusive->BaseLayoutId = Default;
    ONLY_IF_RHIDEBUG(exclusive->DebugModeBits = Default);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
