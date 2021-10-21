#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Container/Vector.h"
#include "IO/ConstChar.h"
#include "Misc/DynamicLibrary.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static constexpr VkQueueFlagBits VK_QUEUE_PRESENT_BIT = VkQueueFlagBits(0x80000000u);
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanInstance final : FVulkanInstanceFunctions, Meta::FNonCopyableNorMovable {
public:
    struct FQueueCreateInfo {
        VkQueueFlags Flags{ 0 };
        float Priority{ 0.0f };
#if USE_PPE_RHIDEBUG
        FVulkanDebugName DebugName;
#endif

        FQueueCreateInfo() = default;
        FQueueCreateInfo(
            VkQueueFlags flags,
            float priority = 0.0f
            ARGS_IF_RHIDEBUG(FStringView debugName = Default) ) NOEXCEPT
        :   Flags(flags), Priority(priority)
#if USE_PPE_RHIDEBUG
        ,   DebugName(debugName)
#endif
        {}
    };

    struct FPhysicalDeviceInfo {
        VkPhysicalDevice vkPhysicalDevice{ VK_NULL_HANDLE };
        VkPhysicalDeviceProperties Properties{};
        VkPhysicalDeviceFeatures Features{};
        VkPhysicalDeviceMemoryProperties Memory{};

        float PerfRating() const NOEXCEPT;
    };
    using FPhysicalDeviceInfoRef = TPtrRef<const FPhysicalDeviceInfo>;

    FVulkanInstance();
    ~FVulkanInstance();

    bool Valid() const { return (VK_NULL_HANDLE != _vkInstance); }

    EVulkanVersion Version() const { return _instanceAPI.version_; }
    const FVulkanInstanceExtensionSet& Extensions() const { return _instanceAPI.instance_extensions_; }
    TMemoryView<const FPhysicalDeviceInfo> PhysicalDevices() const { return _physicalDevices.MakeView(); }

    bool HasExtension(EVulkanInstanceExtension ext) const {
        return (_instanceAPI.instance_extensions_ & ext);
    }

    NODISCARD bool Construct(
        FConstChar applicationName,
        FConstChar engineName,
        EVulkanVersion version = EVulkanVersion::API_version_latest,
        const TMemoryView<const FConstChar>& instanceLayers = RecommendedInstanceLayers(EVulkanVersion::API_version_latest),
        FVulkanInstanceExtensionSet requiredInstanceExtensions = RecommendedInstanceExtensions(EVulkanVersion::API_version_latest),
        FVulkanInstanceExtensionSet optionalInstanceExtensions = RecommendedInstanceExtensions(EVulkanVersion::API_version_latest),
        FVulkanDeviceExtensionSet requiredDeviceExtensions = RequiredDeviceExtensions(EVulkanVersion::API_version_latest),
        FVulkanDeviceExtensionSet optionalDeviceExtensions = RecommendedDeviceExtensions(EVulkanVersion::API_version_latest) );
    void TearDown();

    NODISCARD bool CreateSurface(
        VkSurfaceKHR* pSurface,
        FVulkanWindowHandle window ) const;
    void DestroySurface(VkSurfaceKHR vkSurface) const;

    NODISCARD FPhysicalDeviceInfoRef PickHighPerformanceDevice() const NOEXCEPT;
    NODISCARD FPhysicalDeviceInfoRef PickPhysicalDeviceByName(FStringView deviceName) const NOEXCEPT;

    NODISCARD bool CreateDevice( // back-buffer
        FVulkanDeviceInfo* pDevice,
        VkSurfaceKHR vkSurface,
        const FVulkanDeviceExtensionSet& requiredDeviceExtensions,
        const FVulkanDeviceExtensionSet& optionalDeviceExtensions,
        FPhysicalDeviceInfoRef physicalDevice = nullptr,
        const TMemoryView<const FQueueCreateInfo>& queues = Default ) const;
    NODISCARD bool CreateDevice( // headless
        FVulkanDeviceInfo* pDevice,
        const FVulkanDeviceExtensionSet& requiredDeviceExtensions,
        const FVulkanDeviceExtensionSet& optionalDeviceExtensions,
        FPhysicalDeviceInfoRef physicalDevice = nullptr,
        const TMemoryView<const FQueueCreateInfo>& queues = Default ) const;
    void DestroyDevice(FVulkanDeviceInfo* pDevice) const;

    static TMemoryView<const FConstChar> RecommendedInstanceLayers(EVulkanVersion version);
    static TMemoryView<const FQueueCreateInfo> RecommendedDeviceQueues(EVulkanVersion version);

    static FVulkanInstanceExtensionSet RecommendedInstanceExtensions(EVulkanVersion version);
    static FVulkanInstanceExtensionSet RequiredInstanceExtensions(EVulkanVersion version);
    static FVulkanInstanceExtensionSet RequiredInstanceExtensions(EVulkanVersion version, FWindowHandle window);

    static FVulkanDeviceExtensionSet RecommendedDeviceExtensions(EVulkanVersion version);
    static FVulkanDeviceExtensionSet RequiredDeviceExtensions(EVulkanVersion version);

private:
    VkInstance _vkInstance{ VK_NULL_HANDLE };
    const VkAllocationCallbacks _vkAllocationCallbacks;

    FVulkanExportedAPI _exportedAPI;
    FVulkanGlobalAPI _globalAPI;
    FVulkanInstanceAPI _instanceAPI;

    VECTOR(RHIInstance, FPhysicalDeviceInfo) _physicalDevices;

    FDynamicLibrary _vulkanLib;

#if USE_PPE_RHIDEBUG
    VkDebugUtilsMessengerEXT _vkDebugUtilsMessenger{ VK_NULL_HANDLE };
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
