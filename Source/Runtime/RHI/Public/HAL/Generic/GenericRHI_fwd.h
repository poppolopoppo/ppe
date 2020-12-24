#pragma once

#include "HAL/RHI_fwd.h"

#include "Meta/StronglyTyped.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// GenericRHIDevice.h
//----------------------------------------------------------------------------
enum class EGenericPresentMode : u32;
struct FGenericShaderModule;
class FGenericDevice;
//----------------------------------------------------------------------------
// GenericRHIFixedFunctionState.h
//----------------------------------------------------------------------------
enum class EGenericBlendFactor : u32;
enum class EGenericBlendOp : u32;
enum class EGenericColorComponentMask : u32;
enum class EGenericLogicOp : u32;
struct FGenericBlendAttachmentState;
struct FGenericBlendState;
//----------------------------------------------------------------------------
enum class EGenericCompareOp : u32;
enum class EGenericStencilOp : u32;
struct FGenericStencilOpState;
struct FGenericDepthStencilState;
//----------------------------------------------------------------------------
struct FGenericMultisampleState;
//----------------------------------------------------------------------------
enum class EGenericCullMode : u32;
enum class EGenericFrontFace : u32;
enum class EGenericPolygonMode : u32;
enum class EGenericConservativeRasterizationMode : u32;
struct FGenericRasterizerState;
//----------------------------------------------------------------------------
enum class EGenericDynamicState : u32;
struct FGenericFixedFunctionState;
//----------------------------------------------------------------------------
// GenericRHIFormat.h
//----------------------------------------------------------------------------
enum class EGenericColorSpace : u32;
enum class EGenericFormat : u32;
using EGenericPixelFormat = EGenericFormat;
using EGenericVertexFormat = EGenericFormat;
//----------------------------------------------------------------------------
// GenericRHIInstance.h
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FGenericWindowHandle);
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FGenericWindowSurface);
//----------------------------------------------------------------------------
enum class EGenericPhysicalDeviceFlags : u32;
class FGenericInstance;
//----------------------------------------------------------------------------
// GenericRHIInputAssembly.h
//----------------------------------------------------------------------------
enum class EGenericPrimitiveTopology : u32;
enum class EGenericVertexInputRate : u32;
struct FGenericVertexBinding;
struct FGenericVertexAttribute;
struct FGenericInputAssembly;
//----------------------------------------------------------------------------
// GenericRHIMemoryAllocator.h
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FGenericDeviceMemory);
//----------------------------------------------------------------------------
enum class EGenericMemoryTypeFlags : u32;
struct FGenericMemoryBlock;
class FGenericMemoryAllocator;
//----------------------------------------------------------------------------
// GenericRHIPipelineLayout.h
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FGenericDescriptorSetLayoutHandle);
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FGenericPipelineLayoutHandle);
//----------------------------------------------------------------------------
struct FGenericPushConstantRange;
enum class EGenericDescriptorFlags : u32;
enum class EGenericDescriptorType : u32;
struct FGenericDescriptorBinding;
enum class EGenericDescriptorSetFlags : u32;
struct FGenericDescriptorSetLayout;
struct FGenericPipelineLayout;
//----------------------------------------------------------------------------
// GenericRHIShaderStage.h
//----------------------------------------------------------------------------
enum class EGenericShaderStageFlags : u32;
enum class EGenericShaderStageCreateFlags : u32;
struct FGenericShaderSpecialization;
struct FGenericShaderStage;
//----------------------------------------------------------------------------
// GenericRHISurfaceFormat.h
//----------------------------------------------------------------------------
struct FGenericPixelInfo;
struct FGenericSurfaceFormat;
//----------------------------------------------------------------------------
// GenericRHISwapChain.h
//----------------------------------------------------------------------------
class FGenericSwapChain;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
