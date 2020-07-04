#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

#include "Container/Vector.h"
#include "Meta/Enum.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FGenericPushConstantRange {
    EGenericShaderStageFlags StageFlags{ 0 };
    u32 Offset{ 0 };
    u32 Size{ 0 };
};
//----------------------------------------------------------------------------
enum class EGenericDescriptorFlags : u32 {
    None                       = 0,
    UpdateAfterBind            = 1<<0,
    UpdateUnusedWhilePending   = 1<<1,
    PartiallyBound             = 1<<2,
    VariableDescriptorCount    = 1<<3,
};
ENUM_FLAGS(EGenericDescriptorFlags);
//----------------------------------------------------------------------------
enum class EGenericDescriptorType : u32 {
    Sampler = 0,
    CombinedImageSampler,
    SampledImage,
    StorageImage,
    UniformTexelBuffer,
    StorageTexelBuffer,
    UniformBuffer,
    StorageBuffer,
    UniformBufferDynamic,
    StorageBufferDynamic,
    InputAttachment,
    InlineUniformBlock,
    AccelerationStructure,
};
//----------------------------------------------------------------------------
struct FGenericDescriptorBinding {
    u32 BindingIndex;
    u32 NumDescriptors;
    EGenericDescriptorFlags BindingFlags;
    EGenericDescriptorType DescriptorType;
    EGenericShaderStageFlags StageFlags;
};
//----------------------------------------------------------------------------
enum class EGenericDescriptorSetFlags : u32 {
    None                        = 0,
    UpdateAfterBindPool         = 1<<0,
    PushDescriptor              = 1<<1,
};
ENUM_FLAGS(EGenericDescriptorSetFlags);
//----------------------------------------------------------------------------
struct FGenericDescriptorSetLayout {
    EGenericDescriptorSetFlags SetFlags;
    VECTORINSITU(RHIState, FGenericDescriptorBinding, 1) Bindings;
};
//----------------------------------------------------------------------------
struct FGenericPipelineLayout {
    VECTORINSITU(RHIState, FGenericDescriptorSetLayoutHandle, 2) SetLayouts;
    VECTORINSITU(RHIState, FGenericPushConstantRange, 2) PushConstantRanges;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
