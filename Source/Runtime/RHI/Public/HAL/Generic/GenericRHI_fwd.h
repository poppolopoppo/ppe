#pragma once

#include "RHI_fwd.h"

#include "Meta/StronglyTyped.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericColorSpace : u32;
enum class EGenericFormat : u32;
using EGenericPixelFormat = EGenericFormat;
using EGenericVertexFormat = EGenericFormat;
//----------------------------------------------------------------------------
struct FGenericPixelInfo;
struct FGenericSurfaceFormat;
//----------------------------------------------------------------------------
struct FGenericInstance;
enum class EGenericPhysicalDeviceFlags : u32;
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FGenericWindowHandle);
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FGenericWindowSurface);
//----------------------------------------------------------------------------
enum class EGenericPresentMode : u32;
class FGenericSwapChain;
//----------------------------------------------------------------------------
enum class EGenericMemoryTypeFlags : u32;
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FGenericDeviceMemory);
struct FGenericMemoryBlock;
class FGenericMemoryAllocator;
//----------------------------------------------------------------------------
class FGenericDevice;
//----------------------------------------------------------------------------
struct FGenericShaderModule;
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
struct FGenericMultisampleState;
//----------------------------------------------------------------------------
enum class EGenericCullMode : u32;
enum class EGenericFrontFace : u32;
enum class EGenericPolygonMode : u32;
enum class EGenericConservativeRasterizationMode : u32;
struct FGenericRasterizerState;
//----------------------------------------------------------------------------
enum class EGenericDynamicState : u32;
struct FGenericFixedFunctionState;;
//----------------------------------------------------------------------------
enum class EGenericPrimitiveTopology : u32;
enum class EGenericVertexInputRate : u32;
struct FGenericVertexBinding;
struct FGenericVertexAttribute;
struct FGenericInputAssembly;
//----------------------------------------------------------------------------
enum class EGenericShaderStageFlags : u32;
enum class EGenericShaderStageCreateFlags : u32;
struct FGenericShaderSpecialization;
struct FGenericShaderStage;
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FGenericDescriptorSetLayoutHandle);
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FGenericPipelineLayoutHandle);
enum class EGenericDescriptorFlags : u32;
enum class EGenericDescriptorType : u32;
enum class EGenericDescriptorSetFlags : u32;
struct FGenericPushConstantRange;
struct FGenericDescriptorBinding;
struct FGenericDescriptorSetLayout;
struct FGenericPipelineLayout;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
