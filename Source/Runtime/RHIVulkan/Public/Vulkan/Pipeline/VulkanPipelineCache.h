#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Pipeline/VulkanComputePipeline.h"
#include "Vulkan/Pipeline/VulkanGraphicsPipeline.h"
#include "Vulkan/Pipeline/VulkanMeshPipeline.h"
#include "Vulkan/Pipeline/VulkanRayTracingPipeline.h"

#include "RHI/RayTracingTask.h"

#include "Container/Stack.h"
#include "Container/Vector.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanPipelineCache final : Meta::FNonCopyable {
public:
    using FDynamicStates = TFixedSizeStack<VkDynamicState, 32>;
    using FViewports = TFixedSizeStack<VkViewport, MaxViewports>;
    using FScissors = TFixedSizeStack<VkRect2D, 16>;
    using FVertexInputAttributes = TFixedSizeStack<VkVertexInputAttributeDescription, MaxVertexAttribs>;
    using FVertexInputBindings = TFixedSizeStack<VkVertexInputBindingDescription, MaxVertexBuffers>;
    using FColorAttachments = TFixedSizeStack<VkPipelineColorBlendAttachmentState, MaxColorBuffers>;
    using FSpecializationInfos = VECTOR(RHIPipeline, VkSpecializationInfo, 1);
    using FSpecializationEntries = VECTOR(RHIPipeline, VkSpecializationMapEntry, 1);
    using FSpecializationDatas = VECTOR(RHIPipeline, u32, 1);
    using FShaderStages = VECTOR(RHIPipeline, VkPipelineShaderStageCreateInfo, 3);
    using FRTShaderGroups = VECTOR(RHIPipeline, VkRayTracingShaderGroupCreateInfoKHR, 3);

    using EGroupType = FUpdateRayTracingShaderTable::EGroupType;
    using FRayGenShader = FUpdateRayTracingShaderTable::FRayGenShader;
    using FRTShaderGroup = FUpdateRayTracingShaderTable::FShaderGroup;

    friend bool operator ==(const FRTShaderGroup& lhs, const FRTShaderGroup& rhs) {
        return (lhs.Type == rhs.Type && lhs.MainShader == rhs.MainShader &&
            lhs.AnyHitShader == rhs.AnyHitShader && lhs.IntersectionShader == rhs.IntersectionShader );
    }
    friend hash_t hash_value(const FRTShaderGroup& grp) {
        return hash_tuple(grp.Type, grp.MainShader, grp.AnyHitShader, grp.IntersectionShader);
    }
    using FRTShaderGroupMap = THashMap<TPtrRef<const FRTShaderGroup>, u32, Meta::THash<FRTShaderGroup>, Meta::TEqualTo<FRTShaderGroup>, ALLOCATOR(RHIPipeline)>;

    struct FRTShaderSpecialization {
        FSpecializationID Id;
        u32 Offset{ UMax };
    };
    using FRTShaderSpecializations = TFixedSizeStack<FRTShaderSpecialization, 32>;

    using FShaderModule = FVulkanGraphicsPipeline::FShaderModule;

    struct FBufferCopyRegion {
        FVulkanLocalBuffer* const SrcBuffer{ nullptr };
        FVulkanLocalBuffer* const DstBuffer{ nullptr };
        VkBufferCopy Region{};
    };
    using FBufferCopyRegions = TFixedSizeStack<FBufferCopyRegion, 8>;

    FVulkanPipelineCache();
    ~FVulkanPipelineCache();

    bool MergeWith(FVulkanPipelineCache& other);

    bool CreatePipelineCache(const FVulkanDevice& device);

    bool CreatePipelineInstance(
        VkPipeline* pppipeline,
        FVulkanPipelineLayout const** playout,
        FVulkanCommandBuffer& commandBuffer,
        const FVulkanLogicalRenderPass& logicalRenderPass,
        const FVulkanGraphicsPipeline& pipeline,
        const FVertexInputState& vertexInput,
        const FRenderState& renderState,
        EPipelineDynamicState dynamicStates
        ARGS_IF_RHIDEBUG(EShaderDebugIndex debugModeIndex) );

    bool CreatePipelineInstance(
        VkPipeline* pppipeline,
        FVulkanPipelineLayout const** playout,
        FVulkanCommandBuffer& commandBuffer,
        const FVulkanLogicalRenderPass& logicalRenderPass,
        const FVulkanMeshPipeline& pipeline,
        const FRenderState& renderState,
        EPipelineDynamicState dynamicStates
        ARGS_IF_RHIDEBUG(EShaderDebugIndex debugModeIndex) );

    bool CreatePipelineInstance(
        VkPipeline* pppipeline,
        FVulkanPipelineLayout const** playout,
        FVulkanCommandBuffer& commandBuffer,
        const FVulkanLogicalRenderPass& logicalRenderPass,
        const FVulkanComputePipeline& pipeline,
        const Meta::TOptional<uint3>& localGroupSize,
        VkPipelineCreateFlags createFlags,
        EPipelineDynamicState dynamicStates
        ARGS_IF_RHIDEBUG(EShaderDebugIndex debugModeIndex) );

    void CreateShaderTable(
        FVulkanRayTracingShaderTable* pshaders,
        FBufferCopyRegions* pregions,
        FVulkanCommandBuffer& commandBuffer,
        FRawRTPipelineID pipelineId,
        const FVulkanRayTracingPipeline& scene,
        const FRayGenShader& rayGenShader,
        TMemoryView<const FRTShaderGroup> shaderGroups,
        const u32 maxRecursionDepth );

    void TearDown(const FVulkanDevice& device);

private:
    bool CreatePipelineCache_(const FVulkanDevice& device);

#if USE_PPE_RHIDEBUG
    template <typename _Pipeline>
    bool SetupShaderDebugging_(
        EShaderDebugMode* pdebugMode, EShaderStages* pdebuggableShaders, FRawPipelineLayoutID* playoutId,
        FVulkanCommandBuffer& commandBuffer,
        const _Pipeline& ppln,
        EShaderDebugIndex debugModeIndex );
#endif

    void ClearTemp_();

    void SetColorBlendState_(
        VkPipelineColorBlendStateCreateInfo* pstate,
        FColorAttachments* pattachments,
        const FBlendState& blend,
        const FVulkanRenderPass& renderPass,
        const u32 subpassIndex ) const;
    bool SetShaderStages_(
        FShaderStages* pstages,
        FSpecializationInfos* pinfos,
        FSpecializationEntries* pentries,
        TMemoryView<const FShaderModule> shaders
        ARGS_IF_RHIDEBUG(EShaderDebugMode debugMode, EShaderStages debuggableShaders) ) const;
    void SetDynamicState_(
        VkPipelineDynamicStateCreateInfo* pinfo,
        FDynamicStates* pdynamicStates,
        const EPipelineDynamicState dynamicStates ) const;
    void SetMultisampleState_(
        VkPipelineMultisampleStateCreateInfo* pinfo,
        const FMultisampleState& multisample ) const;
    void SetTessellationState_(
        VkPipelineTessellationStateCreateInfo* pinfo, u32 patchSize) const;
    void SetDepthStencilState_(
        VkPipelineDepthStencilStateCreateInfo* pinfo,
        const FDepthBufferState& depth,
        const FStencilBufferState& stencil) const;
    void SetRasterizationState_(
        VkPipelineRasterizationStateCreateInfo* pstate,
        const FRasterizationState& rasterization ) const;
    void SetupPipelineInputAssemblyState_(
        VkPipelineInputAssemblyStateCreateInfo* pstate,
        const FInputAssemblyState& inputAssembly ) const;
    void SetVertexInputState_(
        VkPipelineVertexInputStateCreateInfo* pstate,
        FVertexInputAttributes* pattributes,
        FVertexInputBindings* pbindings,
        const FVertexInputState& vertexInput ) const;
    void SetViewportState_(
        VkPipelineViewportStateCreateInfo* pstate,
        FViewports* pviewports,
        FScissors* pscissors,
        const uint2& viewportSize,
        const u32 viewportCount,
        const EPipelineDynamicState dynamicStates ) const;
    void AddLocalGroupSizeSpecialization_(
        FSpecializationEntries* pentries,
        FSpecializationDatas* pdatas,
        const uint3& localSizeSpec,
        const uint3& localGroupSize ) const;

    bool CreateShaderStage_(
        VkPipelineShaderStageCreateInfo* pstage,
        const FVulkanRayTracingPipeline* pipeline, const FRTShaderID& id, EShaderDebugMode mode );

    bool FindShaderGroup_(
        VkRayTracingShaderGroupCreateInfoKHR* pinfo,
        const FVulkanRayTracingPipeline* pipeline,
        const FRTShaderGroup& shaderGroup,
        const EShaderDebugMode mode );

    void ValidateRenderState_(
        FRenderState* prender,
        EPipelineDynamicState* pdynamicStates,
        const FVulkanDevice& dev ) const;

    VkPipelineCache _pipelineCache;

    // temp containers:
    FShaderStages _tempStages;
    FDynamicStates _tempDynamicStates;
    FViewports _tempViewports;
    FScissors _tempScissors;
    FVertexInputAttributes _tempVertexAttributes;
    FVertexInputBindings _tempVertexBindings;
    FColorAttachments _tempColorAttachments;
    FSpecializationInfos _tempSpecializationInfos;
    FSpecializationEntries _tempSpecializationEntries;
    FSpecializationDatas _tempSpecializationDatas;
    FRTShaderGroups _tempShaderGroups;
    FRTShaderGroupMap _tempShaderGraph;
    FRTShaderSpecializations _tempShaderSpecializations;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
