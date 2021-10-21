
#include "stdafx.h"

#include "Vulkan/Pipeline/VulkanPipelineCache.h"

#include "Vulkan/Command/VulkanCommandBuffer.h"
#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Pipeline/VulkanShaderModule.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static CONSTEXPR const uint2 GViewportDefaultSize_{ 1024u }; // arbitrary
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
static FPackedDebugMode PackDebugMode_(EShaderDebugMode mode, EShaderStages stages) {
    const FPackedDebugMode packed{ stages, mode };
    Assert(stages == packed.Stages);
    Assert(mode == packed.Mode);
    return packed;
}
#endif
//----------------------------------------------------------------------------
static void SetStencilOpState_(VkStencilOpState* pOpState, const FStencilBufferState::FFaceState& stencil) {
    Assert(pOpState);

    pOpState->failOp = VkCast(stencil.FailOp);
    pOpState->passOp = VkCast(stencil.PassOp);
    pOpState->depthFailOp = VkCast(stencil.DepthFailOp);
    pOpState->compareOp = VkCast(stencil.CompareOp);
    pOpState->compareMask = stencil.CompareMask;
    pOpState->writeMask = stencil.WriteMask;
    pOpState->reference = stencil.Reference;
}
//----------------------------------------------------------------------------
static void SetColorBlendAttachmentState_(
    VkPipelineColorBlendAttachmentState* pState,
    const FColorBufferState& state,
    const bool logicOpEnabled )
{
    pState->blendEnable = (logicOpEnabled ? false : state.EnableAlphaBlending);
    pState->srcColorBlendFactor = VkCast( state.SrcBlendFactor.Color );
    pState->srcAlphaBlendFactor = VkCast( state.SrcBlendFactor.Alpha );
    pState->dstColorBlendFactor = VkCast( state.DstBlendFactor.Color );
    pState->dstAlphaBlendFactor = VkCast( state.DstBlendFactor.Alpha );
    pState->colorBlendOp = VkCast( state.BlendOp.Color );
    pState->alphaBlendOp = VkCast( state.BlendOp.Alpha );
    pState->colorWriteMask =
        (state.ColorMask & EColorMask::R ? VK_COLOR_COMPONENT_R_BIT : 0 ) |
        (state.ColorMask & EColorMask::G ? VK_COLOR_COMPONENT_G_BIT : 0 ) |
        (state.ColorMask & EColorMask::B ? VK_COLOR_COMPONENT_B_BIT : 0 ) |
        (state.ColorMask & EColorMask::A ? VK_COLOR_COMPONENT_A_BIT : 0 );
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanPipelineCache::FVulkanPipelineCache()
:   _pipelineCache(VK_NULL_HANDLE) {
    _transientRTShaderGroups.reserve(MaxStages);
}
//----------------------------------------------------------------------------
FVulkanPipelineCache::~FVulkanPipelineCache() {
    Assert_NoAssume(VK_NULL_HANDLE == _pipelineCache); // must call TearDown()!
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCache::Construct(const FVulkanDevice& device) {
    return CreatePipelineCache_(device);
}
//----------------------------------------------------------------------------
void FVulkanPipelineCache::TearDown(const FVulkanDevice& device) {
    Assert_NoAssume(VK_NULL_HANDLE == _pipelineCache);

    if (_pipelineCache) {
        device.vkDestroyPipelineCache(device.vkDevice(), _pipelineCache, device.vkAllocator());
        _pipelineCache = VK_NULL_HANDLE;
    }
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCache::MergeWith(FVulkanPipelineCache& ) {
    AssertNotImplemented(); // #TODO: implement merge strategy
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCache::CreatePipelineCache_(const FVulkanDevice& device) {
    Assert(VK_NULL_HANDLE == _pipelineCache);

    VkPipelineCacheCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.initialDataSize = 0;
    info.pInitialData = nullptr;

    VK_CHECK( device.vkCreatePipelineCache(device.vkDevice(), &info, device.vkAllocator(), &_pipelineCache) );

    Assert_NoAssume(VK_NULL_HANDLE != _pipelineCache);
    return true;
}
//----------------------------------------------------------------------------
void FVulkanPipelineCache::ClearTransients_() {
    _transientStages.clear();
    _transientDynamicStates.clear();
    _transientViewports.clear();
    _transientScissors.clear();
    _transientVertexAttributes.clear();
    _transientVertexBindings.clear();
    _transientColorAttachments.clear();
    _transientSpecializationInfos.clear();
    _transientSpecializationEntries.clear();
    _transientSpecializationDatas.clear();
    _transientRTShaderGroups.clear();
    _transientRTShaderGraph.clear();
    _transientRTShaderSpecializations.clear();
}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
template <typename _Pipeline>
bool FVulkanPipelineCache::SetupShaderDebugging_(
    EShaderDebugMode* pDebugMode, EShaderStages* pDebuggableShaders, FRawPipelineLayoutID& layoutId,
    FVulkanCommandBuffer& workerCmd,
    const _Pipeline& ppln,
    EShaderDebugIndex debugModeIndex ) {
    Assert(pDebugMode);
    Assert(pDebuggableShaders);
    Assert_NoAssume(layoutId);

    if (Unlikely(not workerCmd.Batch()->FindModeInfoForDebug(pDebugMode, pDebuggableShaders, debugModeIndex)))
        return false;

    const VkShaderStageFlagBits stages = VkCast(*pDebuggableShaders);

    for (const auto& sh : ppln.Shaders) {
        IF_CONSTEXPR(std::is_same_v<FVulkanComputePipeline::FInternalPipeline, _Pipeline>) {
            if (sh.DebugMode != *pDebugMode)
                continue;
        }
        else {
            if (not (stages & sh.Stage) or (sh.DebugMode != *pDebugMode))
                continue;
        }

        workerCmd.Batch()->SetShaderModuleForDebug(debugModeIndex, sh.Module);
    }

    layoutId = workerCmd.ResourceManager().CreateDebugPipelineLayout(
        layoutId, *pDebugMode, *pDebuggableShaders, FDescriptorSetID{ "dbgStorage" });

    Assert_NoAssume(layoutId.Valid());
    return true;
}
#endif
//----------------------------------------------------------------------------
bool FVulkanPipelineCache::CreatePipelineInstance(
    VkPipeline* pPipeline,
    FVulkanPipelineLayout const** pLayout,
    FVulkanCommandBuffer& workerCmd,
    const FVulkanLogicalRenderPass& logicalRenderPass,
    const FVulkanGraphicsPipeline& gPipeline,
    const FVertexInputState& vertexInput,
    const FRenderState& renderState,
    EPipelineDynamicState dynamicStates
    ARGS_IF_RHIDEBUG(EShaderDebugIndex debugModeIndex) ) {
    Assert(pPipeline);
    Assert(pLayout);
    Assert_NoAssume(logicalRenderPass.RenderPassId());

    const FVulkanDevice& device = workerCmd.Device();

    const auto ppln = gPipeline.Read();

    const FVulkanRenderPass* const pRenderPass = workerCmd.AcquireTransient(logicalRenderPass.RenderPassId());
    Assert(pRenderPass);

    FRawPipelineLayoutID layoutId{ *ppln->BaseLayoutId };
    Assert(layoutId);

#if USE_PPE_RHIDEBUG
    EShaderDebugMode debugMode = Default;
    EShaderStages debugStages = Default;
    if (debugModeIndex != Default)
        Verify( SetupShaderDebugging_(&debugMode, &debugStages, layoutId, workerCmd, *ppln, debugModeIndex) );
#endif

    FVulkanGraphicsPipeline::FPipelineInstance inst;
    inst.LayoutId = layoutId;
    inst.DynamicState = dynamicStates;
    inst.RenderPassId = logicalRenderPass.RenderPassId();
    inst.SubpassIndex = checked_cast<u8>(logicalRenderPass.SubpassIndex());
    inst.VertexInput = vertexInput;
    inst.ViewportCount = checked_cast<u8>(logicalRenderPass.Viewports().size());
    inst.RenderState = renderState;
#if USE_PPE_RHIDEBUG
    inst.DebugMode = PackDebugMode_(debugMode, debugStages);
#endif

    if (ppln->PatchControlPoints)
        inst.RenderState.InputAssembly.Topology = EPrimitiveTopology::Patch;

    AssertMessage_NoAssume(L"unsupported topology", ppln->SupportedTopology.Get(static_cast<u32>(inst.RenderState.InputAssembly.Topology)));
    if (not inst.VertexInput.CopyAttributes(ppln->VertexAttributes.MakeView()))
        AssertNotReached(); // not handled

    ValidateRenderState_(&inst.RenderState, &inst.DynamicState, device, logicalRenderPass);

    inst.Invalidate();

    *pLayout = workerCmd.AcquireTransient(layoutId);

    // find already existing instance
    *pPipeline = gPipeline._sharedInstances.LockShared()->GetIFP(inst);
    if (*pPipeline != VK_NULL_HANDLE)
        return true; // cache hit

    // else create a new instance

    ClearTransients_();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    VkPipelineColorBlendStateCreateInfo blendInfo{};
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    VkPipelineMultisampleStateCreateInfo multisampleInfo{};
    VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
    VkPipelineTessellationStateCreateInfo tessellationInfo{};
    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    VkPipelineViewportStateCreateInfo viewportInfo{};

    VerifyRelease( SetShaderStages_(
        &_transientStages, &_transientSpecializationInfos, &_transientSpecializationEntries,
        ppln->Shaders.MakeView() ARGS_IF_RHIDEBUG(debugMode, debugStages)) );

    SetDynamicState_(&dynamicStateInfo, &_transientDynamicStates, inst.DynamicState);
    SetColorBlendState_(&blendInfo, &_transientColorAttachments, inst.RenderState.Blend, *pRenderPass, inst.SubpassIndex);
    SetMultisampleState_(&multisampleInfo, inst.RenderState.Multisample);
    SetTessellationState_(&tessellationInfo, ppln->PatchControlPoints);
    SetDepthStencilState_(&depthStencilInfo, inst.RenderState.Depth, inst.RenderState.Stencil);
    SetRasterizationState_(&rasterizationInfo, inst.RenderState.Rasterization);
    SetupPipelineInputAssemblyState_(&inputAssemblyInfo, inst.RenderState.InputAssembly);
    SetVertexInputState_(&vertexInputInfo, &_transientVertexAttributes, &_transientVertexBindings, inst.VertexInput );
    SetViewportState_(&viewportInfo, &_transientViewports, &_transientScissors, GViewportDefaultSize_, inst.ViewportCount, inst.DynamicState);

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0; //inst.flags;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pRasterizationState = &rasterizationInfo;
    pipelineInfo.pColorBlendState = &blendInfo;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
    pipelineInfo.pMultisampleState = &multisampleInfo;
    pipelineInfo.pTessellationState = (ppln->PatchControlPoints > 0 ? &tessellationInfo : nullptr);
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pDynamicState = (_transientDynamicStates.empty() ? nullptr : &dynamicStateInfo);
    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.layout = (*pLayout)->Handle();
    pipelineInfo.stageCount = checked_cast<u32>(_transientStages.size());
    pipelineInfo.pStages = _transientStages.data();
    pipelineInfo.renderPass = pRenderPass->Handle();
    pipelineInfo.subpass = inst.SubpassIndex;

    if (not rasterizationInfo.rasterizerDiscardEnable) {
        pipelineInfo.pViewportState = &viewportInfo;
    } else {
        pipelineInfo.pViewportState = nullptr;
        pipelineInfo.pMultisampleState = nullptr;
        pipelineInfo.pDepthStencilState = nullptr;
        pipelineInfo.pColorBlendState = nullptr;
    }

    *pPipeline = VK_NULL_HANDLE;

    VK_CHECK( device.vkCreateGraphicsPipelines(
        device.vkDevice(),
        _pipelineCache,
        1, &pipelineInfo,
        device.vkAllocator(),
        pPipeline) );

    LOG_CHECK(RHI, *pPipeline);

    ONLY_IF_RHIDEBUG( workerCmd.EditStatistics([](FFrameStatistics& stats) {
       stats.Resources.NumNewGraphicsPipeline++;
    }) );

    // try to insert new instance
    const auto[it, inserted] = gPipeline._sharedInstances.LockExclusive()
        ->insert({ std::move(inst), *pPipeline });

    if (not inserted) {
        Assert(it->second);
        // other thread already created this pipeline, need to destroy the one we created
        device.vkDestroyPipeline(device.vkDevice(), *pPipeline, device.vkAllocator());
        *pPipeline = it->second;
        return true;
    }

    if (Unlikely(not workerCmd.ResourceManager().AcquireResource(layoutId)) )
        AssertNotReached();

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCache::CreatePipelineInstance(
    VkPipeline* pPipeline,
    FVulkanPipelineLayout const** pLayout,
    FVulkanCommandBuffer& workerCmd,
    const FVulkanLogicalRenderPass& logicalRenderPass,
    const FVulkanMeshPipeline& mPipeline,
    const FRenderState& renderState,
    EPipelineDynamicState dynamicStates
    ARGS_IF_RHIDEBUG(EShaderDebugIndex debugModeIndex) ) {
    Assert(pPipeline);
    Assert(pLayout);
    Assert_NoAssume(logicalRenderPass.RenderPassId());

    const FVulkanDevice& device = workerCmd.Device();

    const auto ppln = mPipeline.Read();

    const FVulkanRenderPass* const pRenderPass = workerCmd.AcquireTransient(logicalRenderPass.RenderPassId());
    Assert(pRenderPass);

    FRawPipelineLayoutID layoutId{ *ppln->BaseLayoutId };
    Assert(layoutId);

#if USE_PPE_RHIDEBUG
    EShaderDebugMode debugMode = Default;
    EShaderStages debugStages = Default;
    if (debugModeIndex != Default)
        Verify( SetupShaderDebugging_(&debugMode, &debugStages, layoutId, workerCmd, *ppln, debugModeIndex) );
#endif

    FVulkanMeshPipeline::FPipelineInstance inst;
    inst.LayoutId = layoutId;
    inst.DynamicState = dynamicStates;
    inst.RenderPassId = logicalRenderPass.RenderPassId();
    inst.SubpassIndex = checked_cast<u8>(logicalRenderPass.SubpassIndex());
    inst.ViewportCount = checked_cast<u8>(logicalRenderPass.Viewports().size());
    inst.RenderState = renderState;
#if USE_PPE_RHIDEBUG
    inst.DebugMode = PackDebugMode_(debugMode, debugStages);
#endif

    inst.RenderState.InputAssembly.Topology = ppln->Topology;

    ValidateRenderState_(&inst.RenderState, &inst.DynamicState, device, logicalRenderPass);

    inst.Invalidate();

    *pLayout = workerCmd.AcquireTransient(layoutId);

    // find already existing instance
    *pPipeline = mPipeline._sharedInstances.LockShared()->GetIFP(inst);
    if (*pPipeline != VK_NULL_HANDLE)
        return true; // cache hit

    // else create a new instance

    ClearTransients_();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    VkPipelineColorBlendStateCreateInfo blendInfo{};
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    VkPipelineMultisampleStateCreateInfo multisampleInfo{};
    VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    VkPipelineViewportStateCreateInfo viewportInfo{};

    VerifyRelease( SetShaderStages_(
        &_transientStages, &_transientSpecializationInfos, &_transientSpecializationEntries,
        ppln->Shaders.MakeView() ARGS_IF_RHIDEBUG(debugMode, debugStages)) );

    SetDynamicState_(&dynamicStateInfo, &_transientDynamicStates, inst.DynamicState);
    SetColorBlendState_(&blendInfo, &_transientColorAttachments, inst.RenderState.Blend, *pRenderPass, inst.SubpassIndex);
    SetMultisampleState_(&multisampleInfo, inst.RenderState.Multisample);
    SetDepthStencilState_(&depthStencilInfo, inst.RenderState.Depth, inst.RenderState.Stencil);
    SetRasterizationState_(&rasterizationInfo, inst.RenderState.Rasterization);
    SetupPipelineInputAssemblyState_(&inputAssemblyInfo, inst.RenderState.InputAssembly);
    SetViewportState_(&viewportInfo, &_transientViewports, &_transientScissors, GViewportDefaultSize_, inst.ViewportCount, inst.DynamicState);

    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0; //inst.flags;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pRasterizationState = &rasterizationInfo;
    pipelineInfo.pColorBlendState = &blendInfo;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
    pipelineInfo.pMultisampleState = &multisampleInfo;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pDynamicState = (_transientDynamicStates.empty() ? nullptr : &dynamicStateInfo);
    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.layout = (*pLayout)->Handle();
    pipelineInfo.stageCount = checked_cast<u32>(_transientStages.size());
    pipelineInfo.pStages = _transientStages.data();
    pipelineInfo.renderPass = pRenderPass->Handle();
    pipelineInfo.subpass = inst.SubpassIndex;

    if (not rasterizationInfo.rasterizerDiscardEnable) {
        pipelineInfo.pViewportState = &viewportInfo;
    } else {
        pipelineInfo.pViewportState = nullptr;
        pipelineInfo.pMultisampleState = nullptr;
        pipelineInfo.pDepthStencilState = nullptr;
        pipelineInfo.pColorBlendState = nullptr;
    }

    *pPipeline = VK_NULL_HANDLE;

    VK_CHECK( device.vkCreateGraphicsPipelines(
        device.vkDevice(),
        _pipelineCache,
        1, &pipelineInfo,
        device.vkAllocator(),
        pPipeline) );

    LOG_CHECK(RHI, *pPipeline);

    ONLY_IF_RHIDEBUG( workerCmd.EditStatistics([](FFrameStatistics& stats) {
       stats.Resources.NumNewMeshPipeline++;
    }) );

    // try to insert new instance
    const auto[it, inserted] = mPipeline._sharedInstances.LockExclusive()
        ->insert({ std::move(inst), *pPipeline });

    if (not inserted) {
        Assert(it->second);
        // other thread already created this pipeline, need to destroy the one we created
        device.vkDestroyPipeline(device.vkDevice(), *pPipeline, device.vkAllocator());
        *pPipeline = it->second;
        return true;
    }

    if (Unlikely(not workerCmd.ResourceManager().AcquireResource(layoutId)) )
        AssertNotReached();

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCache::CreatePipelineInstance(
    VkPipeline* pPipeline,
    FVulkanPipelineLayout const** pLayout,
    FVulkanCommandBuffer& workerCmd,
    const FVulkanComputePipeline& cPipeline,
    const Meta::TOptional<uint3>& localGroupSize,
    VkPipelineCreateFlags createFlags
    ARGS_IF_RHIDEBUG(EShaderDebugIndex debugModeIndex) ) {
    Assert(pPipeline);
    Assert(pLayout);

    const FVulkanDevice& device = workerCmd.Device();

    const auto ppln = cPipeline.Read();

    FRawPipelineLayoutID layoutId{ *ppln->BaseLayoutId };
    Assert(layoutId);

#if USE_PPE_RHIDEBUG
    EShaderDebugMode debugMode = Default;
    EShaderStages debugStages = Default;
    if (debugModeIndex != Default)
        Verify( SetupShaderDebugging_(&debugMode, &debugStages, layoutId, workerCmd, *ppln, debugModeIndex) );
#endif

    AssertReleaseMessage_NoAssume(
        L"defined local group size but shader doesn't support variable local group size, you should use 'layout (local_size_x_id = 0, local_size_y_id = 1, local_size__idz = 2) in;'",
        not localGroupSize.has_value() or Any(NotEqualMask( ppln->LocalSizeSpecialization, uint3(FComputePipelineDesc::UndefinedSpecialization) )) );

    FVulkanComputePipeline::FPipelineInstance inst;
    inst.LayoutId = layoutId;
    inst.Flags = createFlags;
    inst.LocalGroupSize = localGroupSize.value_or(ppln->DefaultLocalGroupSize);
    inst.LocalGroupSize = Blend(
        ppln->DefaultLocalGroupSize,
        inst.LocalGroupSize,
        EqualMask(ppln->LocalSizeSpecialization, uint3{FComputePipelineDesc::UndefinedSpecialization}) );
#if USE_PPE_RHIDEBUG
    inst.DebugMode = PackDebugMode_(debugMode, debugStages);
#endif

    inst.Invalidate();

    Assert_NoAssume((inst.LocalGroupSize.x * inst.LocalGroupSize.y * inst.LocalGroupSize.z) <= device.Limits().maxComputeWorkGroupInvocations);
    Assert_NoAssume(inst.LocalGroupSize.x <= device.Limits().maxComputeWorkGroupSize[0]);
    Assert_NoAssume(inst.LocalGroupSize.y <= device.Limits().maxComputeWorkGroupSize[1]);
    Assert_NoAssume(inst.LocalGroupSize.z <= device.Limits().maxComputeWorkGroupSize[2]);

    *pLayout = workerCmd.AcquireTransient(layoutId);

    // find already existing instance
    *pPipeline = cPipeline._sharedInstances.LockShared()->GetIFP(inst);
    if (*pPipeline != VK_NULL_HANDLE)
        return true; // cache hit

    // else create a new instance

    ClearTransients_();

    VkSpecializationInfo specializationInfo{};
    VkComputePipelineCreateInfo pipelineInfo{};

    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = (*pLayout)->Handle();
    pipelineInfo.flags = inst.Flags;
    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineInfo.stage.flags = 0;
    pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;

#if USE_PPE_RHIDEBUG
    // find module with required debug mode
    for (auto& sh : ppln->Shaders) {
        if ( sh.DebugMode != debugMode)
            continue;

        pipelineInfo.stage.module = sh.Module->vkShaderModule();
        pipelineInfo.stage.pName = sh.Module->EntryPoint();
        break;
    }
    AssertRelease( pipelineInfo.stage.module );
#endif

    AddLocalGroupSizeSpecialization_(
        &_transientSpecializationEntries, &_transientSpecializationDatas,
        ppln->LocalSizeSpecialization, inst.LocalGroupSize );

    if (not _transientSpecializationEntries.empty()) {
        specializationInfo.mapEntryCount = checked_cast<u32>(_transientSpecializationEntries.size());
        specializationInfo.pMapEntries = _transientSpecializationEntries.data();
        specializationInfo.dataSize = checked_cast<size_t>(_transientSpecializationDatas.MakeView().SizeInBytes());
        specializationInfo.pData = _transientSpecializationDatas.data();

        pipelineInfo.stage.pSpecializationInfo = &specializationInfo;
    }

    *pPipeline = VK_NULL_HANDLE;

    VK_CHECK( device.vkCreateComputePipelines(
        device.vkDevice(),
        _pipelineCache,
        1, &pipelineInfo,
        device.vkAllocator(),
        pPipeline ) );


    LOG_CHECK(RHI, *pPipeline);

    ONLY_IF_RHIDEBUG( workerCmd.EditStatistics([](FFrameStatistics& stats) {
       stats.Resources.NumNewComputePipeline++;
    }) );

    // try to insert new instance
    const auto[it, inserted] = cPipeline._sharedInstances.LockExclusive()
        ->insert({ std::move(inst), *pPipeline });

    if (not inserted) {
        Assert(it->second);
        // other thread already created this pipeline, need to destroy the one we created
        device.vkDestroyPipeline(device.vkDevice(), *pPipeline, device.vkAllocator());
        *pPipeline = it->second;
        return true;
    }

    if (Unlikely(not workerCmd.ResourceManager().AcquireResource(layoutId)) )
        AssertNotReached();

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCache::CreateShaderTable(
    FVulkanRayTracingShaderTable* pShaderTable,
    FBufferCopyRegions* pCopyRegions,
    FVulkanCommandBuffer& workerCmd,
    FRawRTPipelineID pipelineId,
    const FVulkanRayTracingScene& scene,
    const FRayGenShader& rayGenShader,
    TMemoryView<const FRTShaderGroup> shaderGroups,
    const u32 maxRecursionDepth ) {
    Assert(pShaderTable);
    Assert(pCopyRegions);

    const auto sharedData = scene.SharedData();

    const FVulkanDevice& device = workerCmd.Device();
    FVulkanResourceManager& resources = workerCmd.ResourceManager();

    const u32 handleSize = checked_cast<u32>(device.Capabilities().RayTracingPropertiesNV.shaderGroupHandleSize);
    const u32 baseAlignment = checked_cast<u32>(device.Capabilities().RayTracingPropertiesNV.shaderGroupBaseAlignment);
    const u32 geometryStride = sharedData->HitShadersPerInstance;
    const u32 maxHitShaders = sharedData->MaxHitShaderCount;

    Assert_NoAssume(baseAlignment >= handleSize);
    Assert_NoAssume(Meta::IsAligned(baseAlignment, handleSize));

    auto* rtPipeline = workerCmd.AcquireTransient(pipelineId);
    LOG_CHECK(RHI, !!rtPipeline);

    const auto ppln = rtPipeline->Read();

    TFixedSizeStack<EShaderDebugMode, static_cast<u32>(EShaderDebugMode::_Count)> debugModes{ EShaderDebugMode::None };

#if USE_PPE_RHIDEBUG
    if (EnableShaderDebugging) {
        forrange(i, 1, ppln->DebugModeBits.size()) {
            if (ppln->DebugModeBits.test(i))
                debugModes.Push(static_cast<EShaderDebugMode>(i));
        }
    }
#endif

    ClearTransients_();

    _transientRTShaderSpecializations.Push(FSpecializationID{ "sbtRecordStride" }, 0u);
    _transientSpecializationDatas.push_back(geometryStride);
    _transientRTShaderSpecializations.Push(FSpecializationID{ "maxRecursionDepth" }, 4u);
    _transientSpecializationDatas.push_back(maxRecursionDepth);

    u32 missShaderCount = 0;
    u32 hitShaderCount = 0;
    u32 callableShaderCount = 0;

    for (auto& sh : shaderGroups) {
        _transientRTShaderGraph.insert({ &sh, checked_cast<u32>(_transientRTShaderGraph.size() + 1) });

        switch (sh.Type) {
        case ERayTracingGroupType::MissShader:
            missShaderCount = Max(sh.RecordOffset+1, missShaderCount);
            break;
        case ERayTracingGroupType::CallableShader:
            callableShaderCount++;
            break;
        case ERayTracingGroupType::TriangleHitShader:
        case ERayTracingGroupType::ProceduralHitShader:
            hitShaderCount++;
            break;

        case ERayTracingGroupType::Unknown: AssertNotImplemented();
        }
    }

    _transientRTShaderGroups.resize(1 + _transientRTShaderGraph.size());
    _transientSpecializationInfos.reserve(1 + missShaderCount + hitShaderCount);
    _transientSpecializationEntries.reserve(_transientSpecializationInfos.size() * _transientRTShaderSpecializations.size());

    // setup offsets

    u32 offset{ 0 };
    const auto exclusiveTable = pShaderTable->_data.LockExclusive();
    exclusiveTable->RayGenOffset = offset;
    exclusiveTable->RayMissOffset = offset = Meta::RoundToNext(offset + handleSize, baseAlignment);
    exclusiveTable->RayHitOffset = offset = Meta::RoundToNext(offset + (handleSize * missShaderCount), baseAlignment);
    exclusiveTable->CallableOffset = offset = Meta::RoundToNext(offset + (handleSize * maxHitShaders), baseAlignment);
    exclusiveTable->BlockSize = offset = Meta::RoundToNext(offset + (handleSize * callableShaderCount), baseAlignment);
    exclusiveTable->RayMissStride = checked_cast<u16>( handleSize );
    exclusiveTable->RayHitStride = checked_cast<u16>( handleSize );
    exclusiveTable->CallableStride = checked_cast<u16>( handleSize );

    exclusiveTable->AvailableShaders.set_if(0, missShaderCount > 0);
    exclusiveTable->AvailableShaders.set_if(1, maxHitShaders > 0);
    exclusiveTable->AvailableShaders.set_if(2, callableShaderCount > 0);

    Assert_NoAssume(Meta::IsAligned(baseAlignment, exclusiveTable->RayGenOffset));
    Assert_NoAssume(Meta::IsAligned(baseAlignment, exclusiveTable->RayMissOffset));
    Assert_NoAssume(Meta::IsAligned(exclusiveTable->RayMissStride, exclusiveTable->RayMissOffset));
    Assert_NoAssume(Meta::IsAligned(baseAlignment, exclusiveTable->RayHitOffset));
    Assert_NoAssume(Meta::IsAligned(exclusiveTable->RayHitStride, exclusiveTable->RayHitOffset));
    Assert_NoAssume(Meta::IsAligned(baseAlignment, exclusiveTable->CallableOffset));
    Assert_NoAssume(Meta::IsAligned(exclusiveTable->CallableStride, exclusiveTable->CallableOffset));

    const u32 requiredSize = checked_cast<u32>(exclusiveTable->BlockSize * debugModes.size());

    // recreate buffer

    if (exclusiveTable->BufferId) {
        if (resources.ResourceDescription(*exclusiveTable->BufferId).SizeInBytes < requiredSize)
            workerCmd.ReleaseResource(exclusiveTable->BufferId.Release());
    }

    if (not exclusiveTable->BufferId) {
        exclusiveTable->BufferId = workerCmd.FrameGraph()->CreateBuffer(
            FBufferDesc{ requiredSize, EBufferUsage::TransferDst | EBufferUsage::RayTracing },
            Default ARGS_IF_RHIDEBUG(pShaderTable->DebugName()) );

        LOG_CHECK(RHI, !!exclusiveTable->BufferId);
    }

    // acquire buffer

    if (exclusiveTable->PipelineId)
        workerCmd.ReleaseResource(exclusiveTable->PipelineId.Release());

    AssertRelease( resources.AcquireResource(pipelineId) );
    exclusiveTable->PipelineId = FRTPipelineID{ pipelineId };

    // destroy old pipelines

    for (auto& table : exclusiveTable->Tables) {
        if (table.LayoutId)
            resources.ReleaseResource(table.LayoutId.Release());

        if (table.Pipeline)
            workerCmd.Batch()->DestroyPostponed(VK_OBJECT_TYPE_PIPELINE, reinterpret_cast<uintptr_t>(table.Pipeline));
    }

    exclusiveTable->Tables.clear();

    // create shader table for each debug mode

    offset = 0;
    for (EShaderDebugMode mode : debugModes) {

        // create ray-gen shader

        VkPipelineShaderStageCreateInfo* const pShaderStage = _transientStages.push_back_Uninitialized();
        LOG_CHECK(RHI, CreateShaderStage_(pShaderStage, *ppln, rayGenShader.Shader ARGS_IF_RHIDEBUG(mode)));

        auto& groupCi = _transientRTShaderGroups[0];
        groupCi.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        groupCi.pNext = nullptr;
        groupCi.anyHitShader = VK_SHADER_UNUSED_KHR;
        groupCi.closestHitShader = VK_SHADER_UNUSED_KHR;
        groupCi.intersectionShader = VK_SHADER_UNUSED_KHR;
        groupCi.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        groupCi.generalShader = checked_cast<u32>(_transientStages.size() - 1);

        // create miss & hit shaders

        for (auto& it : _transientRTShaderGraph)
            LOG_CHECK(RHI, FindShaderGroup_(
                &_transientRTShaderGroups[it.second],
                *ppln, *it.first ARGS_IF_RHIDEBUG(mode)) );

        // acquire pipeline layout

        FRawPipelineLayoutID layoutId = *ppln->BaseLayoutId;

#if USE_PPE_RHIDEBUG
        if (mode != Default) {
            layoutId = resources.CreateDebugPipelineLayout(
                layoutId, mode,
                EShaderStages::AllRayTracing,
                FDescriptorSetID{ "dbgStorage" });

            LOG_CHECK(RHI, !!layoutId);
        }
#endif

        FVulkanRTShaderTable::FShaderTable& table = (*exclusiveTable->Tables.Push_Uninitialized());
        table.BufferOffset = offset;
        table.Mode = mode;
        Assert_NoAssume(Meta::IsAligned(baseAlignment, table.BufferOffset));

        // create pipeline

        VkRayTracingPipelineCreateInfoKHR pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipelineInfo.flags = 0;
        pipelineInfo.stageCount = checked_cast<u32>(_transientStages.size());
        pipelineInfo.pStages = _transientStages.data();
        pipelineInfo.groupCount = checked_cast<u32>(_transientRTShaderGroups.size());
        pipelineInfo.pGroups = _transientRTShaderGroups.data();
        pipelineInfo.maxPipelineRayRecursionDepth = maxRecursionDepth;
        pipelineInfo.layout = workerCmd.AcquireTransient(layoutId)->Handle();
        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        VK_CHECK( device.vkCreateRayTracingPipelinesKHR(
            device.vkDevice(),
            VK_NULL_HANDLE,
            _pipelineCache,
            1, &pipelineInfo,
            device.vkAllocator(),
            &table.Pipeline ));

        ONLY_IF_RHIDEBUG(workerCmd.EditStatistics([](FFrameStatistics& stats) {
            stats.Resources.NumNewRayTracingPipeline++;
        }) );

        VerifyRelease( resources.AcquireResource(layoutId) );
        table.LayoutId = FPipelineLayoutID{ layoutId };

        // allocate staging buffer

        const u32 stagingSize = exclusiveTable->BlockSize;

        FStagingBlock stagingBlock{};
        LOG_CHECK(RHI, workerCmd.StagingAlloc(&stagingBlock, stagingSize, baseAlignment) );

#if USE_PPE_RHIDEBUG
        CONSTEXPR u8 UninitializedPattern_ = 0xAE;
        FPlatformMemory::Memset(stagingBlock.Mapped, 0, stagingSize);
        FPlatformMemory::Memset(stagingBlock.Mapped + exclusiveTable->RayGenOffset, UninitializedPattern_, handleSize);
        FPlatformMemory::Memset(stagingBlock.Mapped + exclusiveTable->RayMissOffset, UninitializedPattern_, handleSize * missShaderCount);
        FPlatformMemory::Memset(stagingBlock.Mapped + exclusiveTable->RayHitOffset, UninitializedPattern_, handleSize * maxHitShaders);
        FPlatformMemory::Memset(stagingBlock.Mapped + exclusiveTable->CallableOffset, UninitializedPattern_, handleSize * callableShaderCount);
#endif

        // ray-gen shader

        VK_CALL( device.vkGetRayTracingShaderGroupHandlesKHR(
            device.vkDevice(),
            table.Pipeline,
            0, 1, handleSize,
            stagingBlock.Mapped + exclusiveTable->RayGenOffset ) );

        u32 callableShaderIndex = 0;
        for (const FUpdateRayTracingShaderTable::FShaderGroup& sh : shaderGroups) {
            const u32 group = _transientRTShaderGraph.Get(&sh);

            switch (sh.Type) {

            case EGroupType::MissShader: {
                const u32 dstOffset = exclusiveTable->RayMissOffset + handleSize * sh.RecordOffset;
                Assert_NoAssume(dstOffset + handleSize <= stagingSize);

                VK_CALL( device.vkGetRayTracingShaderGroupHandlesKHR(
                    device.vkDevice(), table.Pipeline, group, 1, handleSize,
                    stagingBlock.Mapped + dstOffset ));
                break;
            }

            case EGroupType::CallableShader: {
                const u32 dstOffset = exclusiveTable->CallableOffset + handleSize * callableShaderIndex++;
                Assert_NoAssume(dstOffset + handleSize <= stagingSize);

                VK_CALL( device.vkGetRayTracingShaderGroupHandlesKHR(
                    device.vkDevice(), table.Pipeline, group, 1, handleSize,
                    stagingBlock.Mapped + dstOffset ));
                break;
            }

            case EGroupType::TriangleHitShader:
            case EGroupType::ProceduralHitShader: {
                Assert_NoAssume(sh.RecordOffset < geometryStride);

                const auto instance = std::lower_bound(
                    sharedData->GeometryInstances.begin(),
                    sharedData->GeometryInstances.end(),
                    sh.InstanceId );
                AssertRelease(sharedData->GeometryInstances.end() != instance);
                AssertRelease(instance->InstanceId == sh.InstanceId);

                const auto* const pRTGeometry = workerCmd.AcquireTransient(*instance->GeometryId);
                Assert(pRTGeometry);

                if (sh.GeometryId == Default) {
                    const auto aabbs = pRTGeometry->Aabbs();
                    const auto triangles = pRTGeometry->Triangles();

                    forrange(i, 0, triangles.size()) {
                        const u32 index = checked_cast<u32>(instance->IndexOffset + i * geometryStride + sh.RecordOffset);
                        AssertRelease(index < maxHitShaders);

                        const u32 dstOffset = exclusiveTable->RayHitOffset + handleSize * index;
                        Assert_NoAssume(dstOffset + handleSize <= stagingSize);

                        VK_CALL( device.vkGetRayTracingShaderGroupHandlesKHR(
                            device.vkDevice(),
                            table.Pipeline, group, 1, handleSize,
                            stagingBlock.Mapped + dstOffset ));
                    }

                    forrange(i, 0, aabbs.size()) {
                        const u32 index = checked_cast<u32>(instance->IndexOffset + (triangles.size() + i) * geometryStride + sh.RecordOffset);
                        AssertRelease(index < maxHitShaders);

                        const u32 dstOffset = exclusiveTable->RayHitOffset + handleSize * index;
                        Assert_NoAssume(dstOffset + handleSize <= stagingSize);

                        VK_CALL( device.vkGetRayTracingShaderGroupHandlesKHR(
                            device.vkDevice(),
                            table.Pipeline, group, 1, handleSize,
                            stagingBlock.Mapped + dstOffset ));
                    }

                }
                else {
                    const u32 index = checked_cast<u32>(instance->IndexOffset +
                        pRTGeometry->GeometryIndex(sh.GeometryId) * geometryStride +
                        sh.RecordOffset );
                    AssertRelease(index < maxHitShaders);

                    const u32 dstOffset = exclusiveTable->RayHitOffset + handleSize * index;
                    Assert_NoAssume(dstOffset + handleSize <= stagingSize);

                    VK_CALL( device.vkGetRayTracingShaderGroupHandlesKHR(
                        device.vkDevice(),
                        table.Pipeline, group, 1, handleSize,
                        stagingBlock.Mapped + dstOffset ));
                }
                break;
            }

            case EGroupType::Unknown: AssertNotReached();
            }
        }


#if USE_PPE_RHIDEBUG
        // check if uninitialized shader handles

        for (u32 pos = 0; pos < stagingSize; pos += handleSize) {
            u32 matched = 0;
            forrange(i, 0, handleSize)
                matched += (*(stagingBlock.Mapped + i + pos) == UninitializedPattern_);

            AssertMessage_NoAssume(L"uninitialized RT handle found", matched < handleSize);
        }

#endif

        // copy from staging buffer to shader binding table

        pCopyRegions->Push(
            workerCmd.ToLocal(stagingBlock.RawBufferID),
            workerCmd.ToLocal(*exclusiveTable->BufferId),
            VkBufferCopy{
                checked_cast<VkDeviceSize>(stagingBlock.Offset),
                checked_cast<VkDeviceSize>(table.BufferOffset),
                checked_cast<VkDeviceSize>(exclusiveTable->BlockSize)
            });

        // clear temporary arrays

        _transientSpecializationEntries.clear();
        _transientSpecializationInfos.clear();
        _transientStages.clear();
        // *keep*: _transientRTShaderGroups, _transientRTShaderGraph, _transientRTShaderSpecializations

        // finally advance the offset to the next block

        offset = Meta::RoundToNext(offset + exclusiveTable->BlockSize, baseAlignment);
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCache::CreateShaderStage_(
    VkPipelineShaderStageCreateInfo* pStage,
    const FVulkanRayTracingPipeline::FInternalPipeline& pipeline, const FRTShaderID& id
    ARGS_IF_RHIDEBUG(EShaderDebugMode mode) ) {
    Assert(pStage);
    Assert(id.Valid());

    // find suitable shader module

    auto bestMatch = pipeline.Shaders.end();
    for (auto it = pipeline.Shaders.Find(id); it != pipeline.Shaders.end() && *it == id; ++it) {
#if USE_PPE_RHIDEBUG
        if (it->DebugMode == mode) {
            bestMatch = it;
            break;
        }

        if (bestMatch == pipeline.Shaders.end() && it->DebugMode == Default)
            bestMatch = it;

#else
        bestMatch = it;
        break;

#endif
    }

    if (pipeline.Shaders.end() == bestMatch)
        return false;

    const size_t entryCount = _transientSpecializationEntries.size();

    pStage->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pStage->pNext = nullptr;
    pStage->flags = 0;
    pStage->module = bestMatch->Module->vkShaderModule();
    pStage->pName = bestMatch->Module->EntryPoint();
    pStage->pSpecializationInfo = nullptr;

    // set specialization constants

    if (bestMatch->Stage == VK_SHADER_STAGE_RAYGEN_BIT_KHR ||
        bestMatch->Stage == VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR ||
        bestMatch->Stage == VK_SHADER_STAGE_MISS_BIT_KHR ) {


        for (auto& spec : _transientRTShaderSpecializations) {
            const auto it = bestMatch->SpecializationConstants.find(spec.Id);

            if (bestMatch->SpecializationConstants.end() != it) {
                VkSpecializationMapEntry* const pEntry = _transientSpecializationEntries.push_back_Uninitialized();
                pEntry->constantID = it->second;
                pEntry->offset = spec.Offset;
                pEntry->size = sizeof(u32);
            }
        }
    }

    if (_transientSpecializationEntries.size() > entryCount) {
        VkSpecializationInfo* const pSpecInfo = _transientSpecializationInfos.push_back_Uninitialized();
        pSpecInfo->mapEntryCount =checked_cast<u32>(_transientSpecializationEntries.size() - entryCount);
        pSpecInfo->pMapEntries = _transientSpecializationEntries.data() + entryCount;
        pSpecInfo->dataSize = _transientSpecializationDatas.MakeView().SizeInBytes();
        pSpecInfo->pData = _transientSpecializationDatas.data();

        pStage->pSpecializationInfo = pSpecInfo;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCache::FindShaderGroup_(
    VkRayTracingShaderGroupCreateInfoKHR* pInfo,
    const FVulkanRayTracingPipeline::FInternalPipeline& pipeline, const FRTShaderGroup& shaderGroup
    ARGS_IF_RHIDEBUG(EShaderDebugMode mode) ) {
    Assert(pInfo);

    pInfo->sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    pInfo->pNext = nullptr;
    pInfo->generalShader = VK_SHADER_UNUSED_KHR;
    pInfo->closestHitShader = VK_SHADER_UNUSED_KHR;
    pInfo->anyHitShader = VK_SHADER_UNUSED_KHR;
    pInfo->intersectionShader = VK_SHADER_UNUSED_KHR;

    switch (shaderGroup.Type) {
    case EGroupType::MissShader: {
        LOG_CHECK(RHI, CreateShaderStage_(
            _transientStages.push_back_Uninitialized(),
            pipeline, shaderGroup.MainShader ARGS_IF_RHIDEBUG(mode) ));

        pInfo->type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        pInfo->generalShader = checked_cast<u32>(_transientStages.size() - 1);

        return true;
    }
    case EGroupType::TriangleHitShader: {
        pInfo->type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;

        if (shaderGroup.MainShader.Valid()) {
            LOG_CHECK(RHI, CreateShaderStage_(
                _transientStages.push_back_Uninitialized(),
                pipeline, shaderGroup.MainShader ARGS_IF_RHIDEBUG(mode) ));

            pInfo->closestHitShader = checked_cast<u32>(_transientStages.size() - 1);
        }

        if (shaderGroup.AnyHitShader.Valid()) {
            LOG_CHECK(RHI, CreateShaderStage_(
                _transientStages.push_back_Uninitialized(),
                pipeline, shaderGroup.AnyHitShader ARGS_IF_RHIDEBUG(mode) ));

            pInfo->anyHitShader = checked_cast<u32>(_transientStages.size() - 1);
        }

        return true;
    }
    case EGroupType::ProceduralHitShader: {
        pInfo->type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;

        LOG_CHECK(RHI, CreateShaderStage_(
            _transientStages.push_back_Uninitialized(),
            pipeline, shaderGroup.IntersectionShader ARGS_IF_RHIDEBUG(mode) ));

        if (shaderGroup.AnyHitShader.Valid()) {
            LOG_CHECK(RHI, CreateShaderStage_(
                _transientStages.push_back_Uninitialized(),
                pipeline, shaderGroup.AnyHitShader ARGS_IF_RHIDEBUG(mode) ));

            pInfo->anyHitShader = checked_cast<u32>(_transientStages.size() - 1);
        }

        if (shaderGroup.MainShader.Valid()) {
            LOG_CHECK(RHI, CreateShaderStage_(
                _transientStages.push_back_Uninitialized(),
                pipeline, shaderGroup.MainShader ARGS_IF_RHIDEBUG(mode) ));

            pInfo->closestHitShader = checked_cast<u32>(_transientStages.size() - 1);
        }

        return true;
    }

    case EGroupType::CallableShader:
    case EGroupType::Unknown: AssertNotReached();
    }

    return false;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCache::SetShaderStages_(
    FShaderStages* pStages, FSpecializationInfos* pInfos, FSpecializationEntries* pEntries,
    TMemoryView<const FShaderModule> shaders
    ARGS_IF_RHIDEBUG(EShaderDebugMode debugMode, EShaderStages debuggableShaders) ) const {
    Assert(pStages);
    Assert(pInfos);
    Assert(pEntries);

#if USE_PPE_RHIDEBUG
    const VkShaderStageFlags debuggableStages = VkCast(debuggableShaders);
    VkShaderStageFlags existingStages = 0;
    VkShaderStageFlags usedStages = 0;
#endif

    for (const FShaderModule& sh : shaders) {
        Assert(sh.Module);

#if USE_PPE_RHIDEBUG
        existingStages |= sh.Stage;

        if (((debuggableStages & sh.Stage) == static_cast<VkFlags>(sh.Stage) and sh.DebugMode != debugMode) or
            ((debuggableStages & sh.Stage) != static_cast<VkFlags>(sh.Stage) and sh.DebugMode != Default) )
            continue;

        usedStages |= sh.Stage;
#endif

        VkPipelineShaderStageCreateInfo* const pInfo = pStages->push_back_Uninitialized();
        pInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pInfo->pNext = nullptr;
        pInfo->flags = 0;
        pInfo->module = sh.Module->vkShaderModule();
        pInfo->pName = sh.Module->EntryPoint();
        pInfo->stage = sh.Stage;
        pInfo->pSpecializationInfo = nullptr; // #TODO
    }

#if USE_PPE_RHIDEBUG
    // check if all shader stages were added

    if (existingStages != usedStages and debuggableStages != 0) {
        VkShaderStageFlags requiredStages = (existingStages & ~usedStages);

        for (const FShaderModule& sh : shaders) {
            if ((requiredStages & sh.Stage) == static_cast<VkFlags>(sh.Stage) and sh.DebugMode == Default) {
                usedStages |= sh.Stage;
                requiredStages &= ~sh.Stage;

                VkPipelineShaderStageCreateInfo* const pInfo = pStages->push_back_Uninitialized();
                pInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                pInfo->pNext = nullptr;
                pInfo->flags = 0;
                pInfo->module = sh.Module->vkShaderModule();
                pInfo->pName = sh.Module->EntryPoint();
                pInfo->stage = sh.Stage;
                pInfo->pSpecializationInfo = nullptr; // #TODO
            }
        }
    }

    AssertRelease_NoAssume(existingStages == usedStages);
#endif

    return true;
}
//----------------------------------------------------------------------------
void FVulkanPipelineCache::SetDynamicState_(
    VkPipelineDynamicStateCreateInfo* pInfo, FDynamicStates* pDynamicStates,
    const EPipelineDynamicState dynamicStates ) const {
    Assert(pInfo);
    Assert(pDynamicStates);

    pInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pInfo->pNext = nullptr;
    pInfo->flags = 0;

    for (u8 st = 1; st < static_cast<u8>(EPipelineDynamicState::_Last); st <<= 1) {
        if (not Meta::EnumHas(dynamicStates, st))
            continue;

        pDynamicStates->Push(VkCast(static_cast<EPipelineDynamicState>(st)));
    }

    pInfo->dynamicStateCount = checked_cast<u32>(pDynamicStates->size());
    pInfo->pDynamicStates = pDynamicStates->data();
}
//----------------------------------------------------------------------------
void FVulkanPipelineCache::SetMultisampleState_(VkPipelineMultisampleStateCreateInfo* pInfo, const FMultisampleState& multisample ) const {
    Assert(pInfo);

    pInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pInfo->pNext = nullptr;
    pInfo->flags = 0;
    pInfo->rasterizationSamples = VkCast(multisample.Samples);
    pInfo->sampleShadingEnable = multisample.EnableSampleShading;
    pInfo->minSampleShading = multisample.MinSampleShading;
    pInfo->pSampleMask = (multisample.Samples.Enabled() ? multisample.SampleMask.data : nullptr);
    pInfo->alphaToCoverageEnable = multisample.EnableAlphaToCoverage;
    pInfo->alphaToOneEnable = multisample.EnableAlphaToOne;
}
//----------------------------------------------------------------------------
void FVulkanPipelineCache::SetTessellationState_(VkPipelineTessellationStateCreateInfo* pInfo, u32 patchSize) const {
    Assert(pInfo);

    pInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    pInfo->pNext = nullptr;
    pInfo->flags = 0;
    pInfo->patchControlPoints = patchSize;
}
//----------------------------------------------------------------------------
void FVulkanPipelineCache::SetDepthStencilState_(
    VkPipelineDepthStencilStateCreateInfo* pInfo,
    const FDepthBufferState& depth, const FStencilBufferState& stencil ) const {
    Assert(pInfo);

    pInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pInfo->pNext = nullptr;
    pInfo->flags = 0;

    // depth

    pInfo->depthTestEnable = depth.EnableDepthTest;
    pInfo->depthWriteEnable = depth.EnableDepthWrite;
    pInfo->depthCompareOp = VkCast(depth.CompareOp);
    pInfo->depthBoundsTestEnable = depth.EnableBounds;
    pInfo->minDepthBounds = depth.Bounds.x;
    pInfo->maxDepthBounds = depth.Bounds.y;

    // stencil

    pInfo->stencilTestEnable = stencil.EnabledStencilTests;
    SetStencilOpState_(&pInfo->front, stencil.Front);
    SetStencilOpState_(&pInfo->back, stencil.Back);
}
//----------------------------------------------------------------------------
void FVulkanPipelineCache::SetRasterizationState_(
    VkPipelineRasterizationStateCreateInfo* pInfo,
    const FRasterizationState& rasterization ) const {
    Assert(pInfo);

    pInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pInfo->pNext = nullptr;
    pInfo->flags = 0;
    pInfo->cullMode = VkCast(rasterization.CullMode);
    pInfo->polygonMode = VkCast(rasterization.PolygonMode);
    pInfo->lineWidth = rasterization.LineWidth;
    pInfo->depthBiasConstantFactor = rasterization.DepthBiasConstantFactor;
    pInfo->depthBiasClamp = rasterization.DepthBiasClamp;
    pInfo->depthBiasSlopeFactor = rasterization.DepthBiasSlopeFactor;
    pInfo->depthBiasEnable = rasterization.EnableDepthBias;
    pInfo->depthBiasEnable = rasterization.EnableDepthClamp;
    pInfo->rasterizerDiscardEnable = rasterization.EnableDiscard;
    pInfo->frontFace = (rasterization.EnableFrontFaceCCW
        ? VK_FRONT_FACE_COUNTER_CLOCKWISE
        : VK_FRONT_FACE_CLOCKWISE );

}
//----------------------------------------------------------------------------
void FVulkanPipelineCache::SetupPipelineInputAssemblyState_(
    VkPipelineInputAssemblyStateCreateInfo* pInfo,
    const FInputAssemblyState& inputAssembly ) const {
    Assert(pInfo);

    pInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pInfo->pNext = nullptr;
    pInfo->flags = 0;
    pInfo->topology = VkCast(inputAssembly.Topology);
    pInfo->primitiveRestartEnable = inputAssembly.EnablePrimitiveRestart;
}
//----------------------------------------------------------------------------
void FVulkanPipelineCache::SetVertexInputState_(
    VkPipelineVertexInputStateCreateInfo* pInfo, FVertexInputAttributes* pAttributes, FVertexInputBindings* pBindings,
    const FVertexInputState& vertexInput) const {
    Assert(pInfo);
    Assert(pAttributes);
    Assert(pBindings);
    Assert_NoAssume(pAttributes->empty());
    Assert_NoAssume(pBindings->empty());
    Assert_NoAssume(vertexInput.Vertices.empty() == vertexInput.BufferBindings.empty());

    pInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pInfo->pNext = nullptr;
    pInfo->flags = 0;

    for (const auto& src : vertexInput.Vertices) {
        Assert(src.second.Index != UMax);

        VkVertexInputAttributeDescription dst{};
        dst.binding = src.second.BufferBinding;
        dst.location = src.second.Index;
        dst.offset = src.second.Offset;
        dst.format = VkCast(src.second.Format);

        pAttributes->Push(std::move(dst));
    }

    for (const auto& src : vertexInput.BufferBindings) {
        VkVertexInputBindingDescription dst{};
        dst.binding = src.second.Index;
        dst.stride = src.second.Stride;
        dst.inputRate = VkCast(src.second.Rate);

        pBindings->Push(std::move(dst));
    }

    pInfo->pVertexAttributeDescriptions = pAttributes->data();
    pInfo->vertexAttributeDescriptionCount = checked_cast<u32>(pAttributes->size());

    pInfo->pVertexBindingDescriptions = pBindings->data();
    pInfo->vertexBindingDescriptionCount = checked_cast<u32>(pBindings->size());
}
//----------------------------------------------------------------------------
void FVulkanPipelineCache::SetViewportState_(
    VkPipelineViewportStateCreateInfo* pInfo, FViewports* pViewports, FScissors* pScissors,
    const uint2& viewportSize, const u32 viewportCount, const EPipelineDynamicState dynamicStates ) const {
    Assert(pInfo);
    Assert(pViewports);
    Assert(pScissors);

    pViewports->Resize(viewportCount);
    pScissors->Resize(viewportCount);

    forrange(i, 0, viewportCount) {
        pViewports->at(i) = VkViewport{ 0, 0,
            static_cast<float>(viewportSize.x),
            static_cast<float>(viewportSize.y),
            0.0f, 1.0f };

        pScissors->at(i) = VkRect2D{
            VkOffset2D{ 0, 0 },
            VkExtent2D{ viewportSize.x, viewportSize.y } };
    }

    pInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pInfo->pNext = nullptr;
    pInfo->flags = 0;
    pInfo->pViewports = (dynamicStates & EPipelineDynamicState::Viewport ? nullptr : pViewports->data());
    pInfo->viewportCount = checked_cast<u32>(pViewports->size());
    pInfo->pScissors = (dynamicStates & EPipelineDynamicState::Scissor ? nullptr : pScissors->data());
    pInfo->scissorCount = checked_cast<u32>(pScissors->size());
}
//----------------------------------------------------------------------------
void FVulkanPipelineCache::SetColorBlendState_(
    VkPipelineColorBlendStateCreateInfo* pInfo, FColorAttachments* pAttachments,
    const FBlendState& blend, const FVulkanRenderPass& renderPass, const u32 subpassIndex) const {
    Assert(pInfo);
    Assert(pAttachments);
    Assert_NoAssume(subpassIndex < renderPass.Read()->CreateInfo.subpassCount);

    const bool enableLogicOp = (blend.LogicOp != ELogicOp::None);
    const VkSubpassDescription& subpass = renderPass.Read()->CreateInfo.pSubpasses[subpassIndex];

    forrange(i, 0, subpass.colorAttachmentCount) {
        VkPipelineColorBlendAttachmentState colorState{};
        colorState.colorWriteMask = (
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT );

        pAttachments->Push(std::move(colorState));
    }

    forrange(i, 0, Min(blend.Buffers.size(), pAttachments->size()))
        SetColorBlendAttachmentState_(&pAttachments->at(i), blend.Buffers[i], enableLogicOp);

    pInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pInfo->pNext = nullptr;
    pInfo->flags = 0;
    pInfo->pAttachments = pAttachments->data();
    pInfo->attachmentCount = checked_cast<u32>(pAttachments->size());
    pInfo->logicOpEnable = enableLogicOp;
    pInfo->logicOp = (enableLogicOp ? VkCast(blend.LogicOp) : VK_LOGIC_OP_CLEAR);

    pInfo->blendConstants[0] = blend.BlendColor.x;
    pInfo->blendConstants[1] = blend.BlendColor.y;
    pInfo->blendConstants[2] = blend.BlendColor.z;
    pInfo->blendConstants[3] = blend.BlendColor.w;
}
//----------------------------------------------------------------------------
void FVulkanPipelineCache::ValidateRenderState_(
    FRenderState* pRender, EPipelineDynamicState* pDynamicStates,
    const FVulkanDevice& device, const FVulkanLogicalRenderPass& logicalRenderPass ) const {
    Assert(pRender);
    Assert(pDynamicStates);

    if (pRender->Rasterization.EnableDiscard) {
        pRender->Blend = Default;
        pRender->Depth = Default;
        pRender->Stencil = Default;

        *pDynamicStates += EPipelineDynamicState::RasterizerMask;
    }

    // reset to default state if dynamic state enabled
    // -> important for hash function!
    for (u8 st = 1 << 0; st < static_cast<u8>(EPipelineDynamicState::_Last); st <<= 1) {
        if (not Meta::EnumHas(*pDynamicStates, st))
            continue;

        switch (static_cast<EPipelineDynamicState>(st)) {

        case EPipelineDynamicState::Viewport:
        case EPipelineDynamicState::Scissor:
            break;

        case EPipelineDynamicState::StencilCompareMask:
            Assert(pRender->Stencil.EnabledStencilTests);
            pRender->Stencil.Front.CompareMask = UMax;
            pRender->Stencil.Back.CompareMask = UMax;
            break;
        case EPipelineDynamicState::StencilWriteMask:
            Assert(pRender->Stencil.EnabledStencilTests);
            pRender->Stencil.Front.WriteMask = UMax;
            pRender->Stencil.Back.WriteMask = UMax;
            break;
        case EPipelineDynamicState::StencilReference:
            Assert(pRender->Stencil.EnabledStencilTests);
            pRender->Stencil.Front.Reference = UMax;
            pRender->Stencil.Back.Reference = UMax;
            break;

        case EPipelineDynamicState::ShadingRatePalette:
            break;


        case EPipelineDynamicState::All:
        case EPipelineDynamicState::Default:
        case EPipelineDynamicState::_Last:
        case EPipelineDynamicState::Unknown: AssertNotReached();
        }
    }

    // validate color buffer states
    {
        const bool dualSrcBlend = device.Features().dualSrcBlend;
        const auto hasDualSrcBlendFactor = [](EBlendFactor value) NOEXCEPT -> bool {
            switch (value) {
            case EBlendFactor::Src1Color:
            case EBlendFactor::OneMinusSrc1Color:
            case EBlendFactor::Src1Alpha:
            case EBlendFactor::OneMinusSrc1Alpha:
                return true;

            default: break;
            }

            return false;
        };

        for (FColorBufferState& cb : pRender->Blend.Buffers) {
            if (not cb.EnableAlphaBlending) {
                cb.SrcBlendFactor = { EBlendFactor::One, EBlendFactor::One };
                cb.DstBlendFactor = { EBlendFactor::Zero, EBlendFactor::Zero };
                cb.BlendOp = { EBlendOp::Add, EBlendOp::Add };
            }
            else if (not dualSrcBlend) {
                Assert_NoAssume(not hasDualSrcBlendFactor(cb.SrcBlendFactor.Color));
                Assert_NoAssume(not hasDualSrcBlendFactor(cb.SrcBlendFactor.Alpha));
                Assert_NoAssume(not hasDualSrcBlendFactor(cb.DstBlendFactor.Color));
                Assert_NoAssume(not hasDualSrcBlendFactor(cb.DstBlendFactor.Alpha));
            }
        }
    }

#if USE_PPE_ASSERT
    // check depth stencil attachment

    switch (logicalRenderPass.DepthStencilTarget().Layout) {
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // depth & stencil test are allowed
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
        AssertMessage_NoAssume(L"can't write to read-only stencil attachment", pRender->Stencil.ReadOnly());
        break;

    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
        AssertMessage_NoAssume(L"can't write to read-only depth attachment", not pRender->Depth.EnableDepthWrite);
        break;

    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
        AssertMessage_NoAssume(L"can't write to read-only depth attachment", not pRender->Depth.EnableDepthWrite);
        AssertMessage_NoAssume(L"can't write to read-only stencil attachment", pRender->Stencil.ReadOnly());
        break;

#ifdef VK_KHR_separate_depth_stencil_layouts
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR:
        AssertMessage_NoAssume(L"stencil attachment doesn't exists", not pRender->Stencil.EnabledStencilTests);
        break;

    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR:
        AssertMessage_NoAssume(L"can't write to read-only depth attachment", not pRender->Depth.EnableDepthWrite);
        break;

    case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR:
        AssertMessage_NoAssume(L"depth attachment doesn't exists", (not pRender->Depth.EnableDepthTest and not pRender->Depth.EnableDepthWrite));
        break;

    case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR:
        AssertMessage_NoAssume(L"depth attachment doesn't exists", (not pRender->Depth.EnableDepthTest and not pRender->Depth.EnableDepthWrite));
        AssertMessage_NoAssume(L"can't write to read-only stencil attachment", pRender->Stencil.ReadOnly());
        break;
#endif

    case VK_IMAGE_LAYOUT_UNDEFINED:
        AssertMessage_NoAssume(L"depth attachment doesn't exists", (not pRender->Depth.EnableDepthTest and not pRender->Depth.EnableDepthWrite));
        AssertMessage_NoAssume(L"stencil attachment doesn't exists", not pRender->Stencil.EnabledStencilTests);
        break;

    case VK_IMAGE_LAYOUT_GENERAL:
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
    case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:

#ifdef VK_NV_shading_rate_image
    case VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV:
#endif

#ifdef VK_EXT_fragment_density_map
    case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
#endif

#ifndef VK_VERSION_1_2
    case VK_IMAGE_LAYOUT_RANGE_SIZE :
#endif
    case VK_IMAGE_LAYOUT_MAX_ENUM :
        AssertMessage_NoAssume(L"unsupported depth stencil layout", false);
        break;

    default: AssertNotImplemented();
    }

#else
    UNUSED(logicalRenderPass)
#endif

    // validate depth states

    if (not pRender->Depth.EnableDepthTest)
        pRender->Depth.CompareOp = ECompareOp::LessEqual;

    if (not pRender->Depth.EnableBounds)
        pRender->Depth.Bounds = { 0.f, 1.f };

    // validate stencil states

    if (not pRender->Stencil.EnabledStencilTests)
        pRender->Stencil = Default;
}
//----------------------------------------------------------------------------
void FVulkanPipelineCache::AddLocalGroupSizeSpecialization_(
    FSpecializationEntries* pEntries, FSpecializationDatas* pDatas,
    const uint3& localSizeSpec, const uint3& localGroupSize ) const {
    Assert(pEntries);
    Assert(pDatas);

    forrange(i, 0, 3) {
        if (localSizeSpec[i] != FComputePipelineDesc::UndefinedSpecialization) {
            VkSpecializationMapEntry entry;
            entry.constantID = localSizeSpec[i];
            entry.offset = checked_cast<u32>(pEntries->MakeView().SizeInBytes());
            entry.size = sizeof(u32);

            pEntries->push_back(std::move(entry));
            pDatas->push_back(bit_cast<u32>(localGroupSize[i]));
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
