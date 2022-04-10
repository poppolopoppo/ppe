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
    STATIC_CONST_INTEGRAL(u32, MaxStages, 32);

    using FDynamicStates = TFixedSizeStack<VkDynamicState, MaxStages>;
    using FViewports = TFixedSizeStack<VkViewport, MaxViewports>;
    using FScissors = TFixedSizeStack<VkRect2D, MaxViewports>;
    using FVertexInputAttributes = TFixedSizeStack<VkVertexInputAttributeDescription, MaxVertexAttribs>;
    using FVertexInputBindings = TFixedSizeStack<VkVertexInputBindingDescription, MaxVertexBuffers>;
    using FColorAttachments = TFixedSizeStack<VkPipelineColorBlendAttachmentState, MaxColorBuffers>;
    using FSpecializationInfos = VECTORMINSIZE(RHIPipeline, VkSpecializationInfo, MaxStages * MaxSpecializationConstants);
    using FSpecializationEntries = VECTORMINSIZE(RHIPipeline, VkSpecializationMapEntry, MaxStages * MaxSpecializationConstants);
    using FSpecializationDatas = VECTOR(RHIPipeline, u32);
    using FShaderStages = VECTORMINSIZE(RHIPipeline, VkPipelineShaderStageCreateInfo, MaxStages);
    using FRTShaderGroups = VECTORMINSIZE(RHIPipeline, VkRayTracingShaderGroupCreateInfoKHR, MaxStages);

    using EGroupType = FUpdateRayTracingShaderTable::EGroupType;
    using FRayGenShader = FUpdateRayTracingShaderTable::FRayGenShader;
    using FRTShaderGroup = FUpdateRayTracingShaderTable::FShaderGroup;

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

    NODISCARD bool MergeWith(FVulkanPipelineCache& other);

    NODISCARD bool Construct(const FVulkanDevice& device);
    void TearDown(const FVulkanDevice& device);

    NODISCARD bool CreatePipelineInstance(
        VkPipeline* pPipeline,
        FVulkanPipelineLayout const** pLayout,
        FVulkanCommandBuffer& workerCmd,
        const FVulkanLogicalRenderPass& logicalRenderPass,
        const FVulkanGraphicsPipeline& gPipeline,
        const FVertexInputState& vertexInput,
        const FRenderState& renderState,
        EPipelineDynamicState dynamicStates
        ARGS_IF_RHIDEBUG(EShaderDebugIndex debugModeIndex) );

    NODISCARD bool CreatePipelineInstance(
        VkPipeline* pPipeline,
        FVulkanPipelineLayout const** pLayout,
        FVulkanCommandBuffer& workerCmd,
        const FVulkanLogicalRenderPass& logicalRenderPass,
        const FVulkanMeshPipeline& mPipeline,
        const FRenderState& renderState,
        EPipelineDynamicState dynamicStates
        ARGS_IF_RHIDEBUG(EShaderDebugIndex debugModeIndex) );

    NODISCARD bool CreatePipelineInstance(
        VkPipeline* pPipeline,
        FVulkanPipelineLayout const** pLayout,
        FVulkanCommandBuffer& workerCmd,
        const FVulkanComputePipeline& cPipeline,
        const Meta::TOptional<uint3>& localGroupSize,
        VkPipelineCreateFlags createFlags
        ARGS_IF_RHIDEBUG(EShaderDebugIndex debugModeIndex) );

    NODISCARD bool CreateShaderTable(
        FVulkanRayTracingShaderTable* pShaderTable,
        FBufferCopyRegions* pCopyRegions,
        FVulkanCommandBuffer& workerCmd,
        FRawRTPipelineID pipelineId,
        const FVulkanRayTracingScene& scene,
        const FRayGenShader& rayGenShader,
        TMemoryView<const FRTShaderGroup> shaderGroups,
        const u32 maxRecursionDepth );

private:
    NODISCARD bool CreatePipelineCache_(const FVulkanDevice& device);

#if USE_PPE_RHIDEBUG
    template <typename _Pipeline>
    NODISCARD bool SetupShaderDebugging_(
        EShaderDebugMode* pDebugMode, EShaderStages* pDebuggableShaders, FRawPipelineLayoutID& layoutId,
        FVulkanCommandBuffer& workerCmd,
        const _Pipeline& ppln,
        EShaderDebugIndex debugModeIndex );
#endif

    void ClearTransients_();

    void SetColorBlendState_(
        VkPipelineColorBlendStateCreateInfo* pInfo,
        FColorAttachments* pAttachments,
        const FBlendState& blend,
        const FVulkanRenderPass& renderPass,
        const u32 subpassIndex ) const;
    NODISCARD bool SetShaderStages_(
        FShaderStages* pStages,
        FSpecializationInfos* pInfos,
        FSpecializationEntries* pEntries,
        TMemoryView<const FShaderModule> shaders
        ARGS_IF_RHIDEBUG(EShaderDebugMode debugMode, EShaderStages debuggableShaders) ) const;
    void SetDynamicState_(
        VkPipelineDynamicStateCreateInfo* pInfo,
        FDynamicStates* pDynamicStates,
        const EPipelineDynamicState dynamicStates ) const;
    void SetMultisampleState_(
        VkPipelineMultisampleStateCreateInfo* pInfo,
        const FMultisampleState& multisample ) const;
    void SetTessellationState_(
        VkPipelineTessellationStateCreateInfo* pInfo, u32 patchSize) const;
    void SetDepthStencilState_(
        VkPipelineDepthStencilStateCreateInfo* pInfo,
        const FDepthBufferState& depth,
        const FStencilBufferState& stencil) const;
    void SetRasterizationState_(
        VkPipelineRasterizationStateCreateInfo* pInfo,
        const FRasterizationState& rasterization ) const;
    void SetupPipelineInputAssemblyState_(
        VkPipelineInputAssemblyStateCreateInfo* pInfo,
        const FInputAssemblyState& inputAssembly ) const;
    void SetVertexInputState_(
        VkPipelineVertexInputStateCreateInfo* pInfo,
        FVertexInputAttributes* pAttributes,
        FVertexInputBindings* pBindings,
        const FVertexInputState& vertexInput ) const;
    void SetViewportState_(
        VkPipelineViewportStateCreateInfo* pInfo,
        FViewports* pViewports,
        FScissors* pScissors,
        const uint2& viewportSize,
        const u32 viewportCount,
        const EPipelineDynamicState dynamicStates ) const;
    void AddLocalGroupSizeSpecialization_(
        FSpecializationEntries* pEntries,
        FSpecializationDatas* pDatas,
        const uint3& localSizeSpec,
        const uint3& localGroupSize ) const;

    NODISCARD bool CreateShaderStage_(
        VkPipelineShaderStageCreateInfo* pStage,
        const FVulkanRayTracingPipeline::FInternalPipeline& pipeline, const FRTShaderID& id
        ARGS_IF_RHIDEBUG(EShaderDebugMode mode) );

    template <typename _Pipeline>
    NODISCARD bool FindCachedPipeline_(
        VkPipeline* pInCache,
        const _Pipeline& ppln,
        const typename _Pipeline::FPipelineInstance& inst) const;

    NODISCARD bool FindShaderGroup_(
        VkRayTracingShaderGroupCreateInfoKHR* pInfo,
        const FVulkanRayTracingPipeline::FInternalPipeline& pipeline,
        const FRTShaderGroup& shaderGroup
        ARGS_IF_RHIDEBUG(EShaderDebugMode mode) );

    void ValidateRenderState_(
        FRenderState* pRender,
        EPipelineDynamicState* pDynamicStates,
        const FVulkanDevice& device,
        const FVulkanLogicalRenderPass& logicalRenderPass ) const;

    VkPipelineCache _pipelineCache;

    // transient containers:
    FShaderStages _transientStages;
    FDynamicStates _transientDynamicStates;
    FViewports _transientViewports;
    FScissors _transientScissors;
    FVertexInputAttributes _transientVertexAttributes;
    FVertexInputBindings _transientVertexBindings;
    FColorAttachments _transientColorAttachments;
    FSpecializationInfos _transientSpecializationInfos;
    FSpecializationEntries _transientSpecializationEntries;
    FSpecializationDatas _transientSpecializationDatas;
    FRTShaderGroups _transientRTShaderGroups;
    FRTShaderGroupMap _transientRTShaderGraph;
    FRTShaderSpecializations _transientRTShaderSpecializations;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
