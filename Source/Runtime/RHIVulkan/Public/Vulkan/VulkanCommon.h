#pragma once

#include "Vulkan/Vulkan_fwd.h"

#include "Vulkan/VulkanIncludes_fwd.h"

#include "Vulkan/Common/VulkanEnums.h"
#include "Vulkan/Common/VulkanError.h"

#include "RHI/FrameDebug.h"
#include "RHI/ResourceId.h"
#include "RHI/ResourceTypes.h"

#include "Container/Stack.h"
#include "Diagnostic/Logger_fwd.h"
#include "IO/TextWriter_fwd.h"
#include "Maths/ScalarMatrix.h"

#include "vulkan-exports.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using EVulkanVersion = vk::api_version;
using FVulkanExportedAPI = vk::exported_api;
using FVulkanGlobalAPI = vk::global_api;
//----------------------------------------------------------------------------
using EVulkanInstanceExtension = vk::instance_extension;
using FVulkanInstanceExtensionSet = vk::instance_extension_set;
using FVulkanInstanceAPI = vk::instance_api;
using FVulkanInstanceFunctions = vk::instance_fn;
//----------------------------------------------------------------------------
using EVulkanDeviceExtension = vk::device_extension;
using FVulkanDeviceExtensionSet = vk::device_extension_set;
using FVulkanDeviceAPI = vk::device_api;
using FVulkanDeviceFunctions = vk::device_fn;
//----------------------------------------------------------------------------
// https://vulkan.gpuinfo.org/
enum class EVulkanVendor : u32 {
    AMD         = 0x1002,
    ImgTec      = 0x1010,
    NVIDIA      = 0x10DE,
    ARM         = 0x13B5,
    Qualcomm    = 0x5143,
    INTEL       = 0x8086,

    Unknown     = 0,
};
PPE_RHIVULKAN_API FStringView EVulkanVendor_Name(EVulkanVendor vendor);
PPE_RHIVULKAN_API FTextWriter& operator <<(FTextWriter& oss, EVulkanVendor vendor);
PPE_RHIVULKAN_API FWTextWriter& operator <<(FWTextWriter& oss, EVulkanVendor vendor);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FVulkanDescriptorSets = TFixedSizeStack<VkDescriptorSet, MaxDescriptorSets>;
//----------------------------------------------------------------------------
struct FVulkanPipelineResourceSet {
    struct FResource {
        FDescriptorSetID DescriptorSetId;
        const FVulkanPipelineResources* PipelineResources;
        u32 OffsetIndex{ UMax };
        u32 OffsetCount{ 0 };
    };

    TFixedSizeStack<FResource, MaxDescriptorSets> Resources;
    mutable TFixedSizeStack<u32, MaxBufferDynamicOffsets> DynamicOffsets;
};
//----------------------------------------------------------------------------
FWD_REFPTR(VulkanShaderModule);
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG || USE_PPE_RHITASKNAME
using FVulkanDebugName = TStaticString<64>;
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
enum class EShaderDebugIndex : u32 {
    Unknown = ~0u
};
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Defined in PPE to fix ADL (those are aliases of vk::* namespace)
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API FTextWriter& operator <<(FTextWriter& oss, RHI::EVulkanVersion);
PPE_RHIVULKAN_API FWTextWriter& operator <<(FWTextWriter& oss, RHI::EVulkanVersion);
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API FTextWriter& operator <<(FTextWriter& oss, RHI::EVulkanInstanceExtension);
PPE_RHIVULKAN_API FWTextWriter& operator <<(FWTextWriter& oss, RHI::EVulkanInstanceExtension);
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API FTextWriter& operator <<(FTextWriter& oss, const RHI::FVulkanInstanceExtensionSet&);
PPE_RHIVULKAN_API FWTextWriter& operator <<(FWTextWriter& oss, const RHI::FVulkanInstanceExtensionSet&);
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API FTextWriter& operator <<(FTextWriter& oss, RHI::EVulkanDeviceExtension);
PPE_RHIVULKAN_API FWTextWriter& operator <<(FWTextWriter& oss, RHI::EVulkanDeviceExtension);
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API FTextWriter& operator <<(FTextWriter& oss, const RHI::FVulkanDeviceExtensionSet&);
PPE_RHIVULKAN_API FWTextWriter& operator <<(FWTextWriter& oss, const RHI::FVulkanDeviceExtensionSet&);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
