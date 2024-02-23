#pragma once

#include "Test_Includes.h"
#include "RHI/EnumHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline bool TestVertexInput(const RHI::FGraphicsPipelineDesc& ppln, const RHI::FVertexID& id, RHI::EVertexFormat format, u32 index) {
    if (const RHI::FVertexAttribute* attribute = ppln.VertexInput(id)) {
        return (attribute->Index == index &&
                attribute->Format == format );
    }
    return false;
}
//----------------------------------------------------------------------------
inline bool TestFragmentOutput(const RHI::FGraphicsPipelineDesc& ppln, RHI::EFragmentOutput type, u32 index) {
    for (const RHI::FPipelineFragmentOutput& frag : ppln.FragmentOutputs) {
        if (frag.Index == index)
            return (frag.Type == type);
    }
    return false;
}
//----------------------------------------------------------------------------
inline bool TestTextureUniform(const RHI::FDescriptorSet& ds, const RHI::FUniformID& id,
    RHI::EImageSampler textureType,
    u32 bindingIndex, RHI::EShaderStages stageFlags, u32 arraySize = 1) {
    using namespace RHI;
    if (auto [un, ptr] = ds.Uniform<FPipelineDesc::FTexture>(id); !!ptr) {
        const EResourceState state = (EResourceState_ShaderSample | EResourceState_FromShaders(stageFlags));
        return (ptr->Type == textureType &&
                ptr->State == state &&
                un->Index.VKBinding() == bindingIndex &&
                un->StageFlags == stageFlags &&
                un->ArraySize == arraySize );
    }
    return false;
}
//----------------------------------------------------------------------------
inline bool TestImageUniform(const RHI::FDescriptorSet& ds, const RHI::FUniformID& id,
    RHI::EImageSampler imageType, RHI::EShaderAccess access,
    u32 bindingIndex, RHI::EShaderStages stageFlags, u32 arraySize = 1) {
    using namespace RHI;
    if (auto [un, ptr] = ds.Uniform<FPipelineDesc::FImage>(id); !!ptr) {
        const EResourceState state = (EResourceState_FromShaderAccess(access) | EResourceState_FromShaders(stageFlags));
        return (ptr->Type == imageType &&
                ptr->State == state &&
                un->Index.VKBinding() == bindingIndex &&
                un->StageFlags == stageFlags &&
                un->ArraySize == arraySize);
    }
    return false;
}
//----------------------------------------------------------------------------
inline bool TestSamplerUniform(const RHI::FDescriptorSet& ds, const RHI::FUniformID& id,
    u32 bindingIndex, RHI::EShaderStages stageFlags, u32 arraySize = 1) {
    using namespace RHI;
    if (auto [un, ptr] = ds.Uniform<FPipelineDesc::FSampler>(id); !!ptr) {
        return (un->Index.VKBinding() == bindingIndex &&
                un->StageFlags == stageFlags &&
                un->ArraySize == arraySize);
    }
    return false;
}
//----------------------------------------------------------------------------
inline bool TestSubpassInputUniform(const RHI::FDescriptorSet& ds, const RHI::FUniformID& id,
    u32 attachmentIndex, bool isMultisample,
    u32 bindingIndex, RHI::EShaderStages stageFlags, u32 arraySize = 1) {
    using namespace RHI;
    if (auto [un, ptr] = ds.Uniform<FPipelineDesc::FSubpassInput>(id); !!ptr) {
        const EResourceState state = (EResourceState_InputAttachment | EResourceState_FromShaders(stageFlags));
        return (ptr->AttachmentIndex == attachmentIndex &&
                ptr->IsMultiSample == isMultisample &&
                ptr->State == state &&
                un->Index.VKBinding() == bindingIndex &&
                un->StageFlags == stageFlags &&
                un->ArraySize == arraySize);
    }
    return false;
}
//----------------------------------------------------------------------------
inline bool TestBufferUniform(const RHI::FDescriptorSet& ds, const RHI::FUniformID& id,
    size_t size,
    u32 bindingIndex, RHI::EShaderStages stageFlags, u32 arraySize = 1,
    u32 dynamicOffsetIndex = RHI::FPipelineDesc::StaticOffset ) {
    using namespace RHI;
    if (auto [un, ptr] = ds.Uniform<FPipelineDesc::FUniformBuffer>(id); !!ptr) {
        const EResourceState state = (EResourceState_UniformRead | EResourceState_FromShaders(stageFlags) |
            (dynamicOffsetIndex == RHI::FPipelineDesc::StaticOffset ? EResourceState_Unknown : EResourceFlags::BufferDynamicOffset));
        return (ptr->Size == size &&
                ptr->DynamicOffsetIndex == dynamicOffsetIndex &&
                ptr->State == state &&
                un->Index.VKBinding() == bindingIndex &&
                un->StageFlags == stageFlags &&
                un->ArraySize == arraySize );
    }
    return false;
}
//----------------------------------------------------------------------------
inline bool TestStorageBuffer(const RHI::FDescriptorSet& ds, const RHI::FUniformID& id,
    size_t staticSize, size_t arrayStride, RHI::EShaderAccess access,
    u32 bindingIndex, RHI::EShaderStages stageFlags, u32 arraySize = 1,
    u32 dynamicOffsetIndex = RHI::FPipelineDesc::StaticOffset ) {
    using namespace RHI;
    if (auto [un, ptr] = ds.Uniform<FPipelineDesc::FStorageBuffer>(id); !!ptr) {
        const EResourceState state = (EResourceState_FromShaderAccess(access) | EResourceState_FromShaders(stageFlags) |
            (dynamicOffsetIndex == RHI::FPipelineDesc::StaticOffset ? EResourceState_Unknown : EResourceFlags::BufferDynamicOffset));
        return (ptr->StaticSize == staticSize &&
                ptr->ArrayStride == arrayStride &&
                ptr->DynamicOffsetIndex == dynamicOffsetIndex &&
                ptr->State == state &&
                un->Index.VKBinding() == bindingIndex &&
                un->StageFlags == stageFlags &&
                un->ArraySize == arraySize );
    }
    return false;
}
//----------------------------------------------------------------------------
inline bool TestRaytracingScene(const RHI::FDescriptorSet& ds, const RHI::FUniformID& id,
    u32 bindingIndex, RHI::EShaderStages stageFlags, u32 arraySize = 1 ) {
    using namespace RHI;
    if (auto [un, ptr] = ds.Uniform<FPipelineDesc::FRayTracingScene>(id); !!ptr) {
        const EResourceState state = (EResourceFlags::RayTracingShader | EResourceState_ShaderRead);
        return (ptr->State == state &&
                un->Index.VKBinding() == bindingIndex &&
                un->StageFlags == stageFlags &&
                un->ArraySize == arraySize );
    }
    return false;
}
//----------------------------------------------------------------------------
inline bool TestSpecializationConstant(const RHI::FPipelineDesc::FShader& shader, const RHI::FSpecializationID& id, u32 index) {
    using namespace RHI;
    if (auto it = shader.Specializations.find(id); shader.Specializations.end() != it) {
        return (it->second == index);
    }
    return false;
}
//----------------------------------------------------------------------------
inline bool TestPushConstant(const RHI::FPipelineDesc& desc, const RHI::FPushConstantID& id,
    RHI::EShaderStages stageFlags, size_t offset, size_t size ) {
    using namespace RHI;
    if (auto it = desc.PipelineLayout.PushConstants.find(id); desc.PipelineLayout.PushConstants.end() != it) {
        const FPipelinePushConstant& pc = it->second;
        return (pc.Offset == checked_cast<u16>(offset) &&
                pc.Size == checked_cast<u16>(size) &&
                pc.StageFlags == stageFlags );
    }
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
