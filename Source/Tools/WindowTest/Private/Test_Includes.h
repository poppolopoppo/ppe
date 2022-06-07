#pragma once

#include "WindowTestApp.h"

#include "RHI/FrameGraph.h"

#include "RHI/BufferDesc.h"
#include "RHI/DrawContext.h"
#include "RHI/EnumToString.h"
#include "RHI/ImageView.h"
#include "RHI/MemoryDesc.h"
#include "RHI/PipelineDesc.h"
#include "RHI/SamplerDesc.h"

#include "Container/RawStorage.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformNotification.h"
#include "HAL/RHIService.h"
#include "IO/Format.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "Maths/PackedVectors.h"
#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Memory/RefPtr.h"
#include "Meta/Assert.h"
#include "Meta/Utility.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EXTERN_LOG_CATEGORY(, WindowTest)
//----------------------------------------------------------------------------
struct FExpectAssertInScope : Meta::FNonCopyable {
    FExpectAssertInScope() NOEXCEPT
    :   AssertHandler(SetAssertionHandler(&OnAssert))
    ,   AssertReleaseHandler(SetAssertionReleaseHandler(&OnAssert)) {
        NumAssertsTLS_() = 0;
    }

    ~FExpectAssertInScope() {
        SetAssertionHandler(AssertHandler);
        SetAssertionReleaseHandler(AssertReleaseHandler);
        Assert_NoAssume(NumAsserts() > 0);
    }

    u32 NumAsserts() const {
        return NumAssertsTLS_();
    }

private:
    FAssertionHandler AssertHandler;
    FAssertionHandler AssertReleaseHandler;

    static u32& NumAssertsTLS_() {
        static THREAD_LOCAL u32 GNumAssertsTLS{ 0 };
        return GNumAssertsTLS;
    }

    static bool OnAssert(const wchar_t* msg, const wchar_t* file, unsigned line) {
        Unused(msg, file, line);
        LOG(WindowTest, Info, L"Expected assert: '{0}' in {1}:{2}", MakeCStringView(msg), MakeCStringView(file), line);
        ++NumAssertsTLS_();
        return false; // no failure
    }
};
//----------------------------------------------------------------------------
inline bool TestTextureUniform(const RHI::FDescriptorSet& ds, const RHI::FUniformID& id,
    RHI::EImageSampler textureType,
    u32 bindingIndex, RHI::EShaderStages stageFlags, u32 arraySize = 1) {
    using namespace RHI;
    if (auto [un, ptr] = ds.Uniform<FPipelineDesc::FTexture>(id); !!ptr) {
        const EResourceState state = (EResourceState::ShaderSample | EResourceState_FromShaders(stageFlags));
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
        const EResourceState state = (EResourceState::InputAttachment | EResourceState_FromShaders(stageFlags));
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
        const EResourceState state = (EResourceState::UniformRead | EResourceState_FromShaders(stageFlags) |
            (dynamicOffsetIndex == RHI::FPipelineDesc::StaticOffset ? EResourceState::Unknown : EResourceState::_BufferDynamicOffset));
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
            (dynamicOffsetIndex == RHI::FPipelineDesc::StaticOffset ? EResourceState::Unknown : EResourceState::_BufferDynamicOffset));
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
        const EResourceState state = (EResourceState::_RayTracingShader | EResourceState::ShaderRead);
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
        return (it->second.Offset == checked_cast<u16>(offset) &&
                it->second.Offset == checked_cast<u16>(size) &&
                it->second.StageFlags == stageFlags );
    }
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
