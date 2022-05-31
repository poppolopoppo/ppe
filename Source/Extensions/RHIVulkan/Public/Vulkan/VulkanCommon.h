#pragma once

#include "Vulkan/Vulkan_fwd.h"

#if USE_PPE_RHITRACE && 0
#   define VKLOG_APICALL(_NAME, ...) do { \
        using namespace PPE::RHI; \
        RHI_TRACE(WSTRINGIZE(_NAME), __VA_ARGS__); \
    } while(0)
#endif

#include "Vulkan/VulkanIncludes_fwd.h"

#include "Vulkan/Common/VulkanEnums.h"
#include "Vulkan/Common/VulkanError.h"

#if USE_PPE_RHITRACE
#   include "Vulkan/Common/VulkanEnumToString.h"
#endif

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
        const FVulkanPipelineResources* PipelineResources{ nullptr };
        u32 DynamicOffsetIndex{ UMax };
        u32 DynamicOffsetCount{ 0 };
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
struct FVulkanExternalObject {
    u64 Id;

    FVulkanExternalObject() = default;
    CONSTEXPR FVulkanExternalObject(u64 vkObjectId) NOEXCEPT : Id(vkObjectId) {}
    FVulkanExternalObject(const void* vkHandle) NOEXCEPT : FVulkanExternalObject(bit_cast<uintptr_t>(vkHandle)) {}
    FVulkanExternalObject(const FExternalBuffer& externalBuffer) NOEXCEPT : FVulkanExternalObject(externalBuffer.Value) {}
    FVulkanExternalObject(const FExternalImage& externalImage) NOEXCEPT : FVulkanExternalObject(externalImage.Value) {}
    FVulkanExternalObject(const FWindowSurface& windowSurface) NOEXCEPT : FVulkanExternalObject(windowSurface.Value) {}

    bool Valid() const { return !!Id; }

    void* Pointer() const {
        return bit_cast<void*>(checked_cast<uintptr_t>(Id));
    }

    operator u64 () const NOEXCEPT { return Id; }
    u64 operator *() const NOEXCEPT { return Id; }

    FExternalBuffer ExternalBuffer() const { return FExternalBuffer{ Pointer() }; }
    FExternalImage ExternalImage() const { return FExternalImage{ Pointer() }; }
    FWindowSurface WindowSurface() const { return FWindowSurface{ Pointer() }; }

    template <typename _Vk>
    _Vk Cast() const NOEXCEPT {
        IF_CONSTEXPR(std::is_same_v<decltype(Id), _Vk>) {
            return Id;
        } else {
            return bit_cast<_Vk>(Pointer());
        }
    }
};
//----------------------------------------------------------------------------
struct FVulkanExternalImageDesc {
    VkImage Image{};
    VkImageType Type{};
    VkImageCreateFlagBits Flags{};
    VkImageUsageFlagBits Usage{};
    VkFormat Format{};
    VkImageLayout CurrentLayout{};
    VkImageLayout DefaultLayout{};
    VkSampleCountFlagBits Samples{};
    uint3 Dimensions{};
    u32 ArrayLayers{0};
    u32 MaxLevels{0};
    // queue family that owns image, you must specify this correctly
    // if image created with exclusive sharing mode and you need to
    // keep current content of the image, otherwise keep default value:
    u32 QueueFamily{UMax};
    // required if sharing mode is concurrent:
    TMemoryView<const u32> ConcurrentQueueFamilyIndices{};
};
//----------------------------------------------------------------------------
struct FVulkanExternalBufferDesc {
    VkBuffer Buffer{};
    VkBufferUsageFlagBits Usage{};
    u32 SizeInBytes{0};
    // queue family that owns buffer, you must specify this correctly
    // if buffer created with exclusive sharing mode and you need to
    // keep current content of the buffer, otherwise keep default value:
    u32 QueueFamily{UMax};
    // required if sharing mode is concurrent:
    TMemoryView<const u32> ConcurrentQueueFamilyIndices{};
};
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
