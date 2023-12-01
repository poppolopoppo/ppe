#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Container/Vector.h"
#include "Diagnostic/Logger_fwd.h"
#include "IO/ConstChar.h"
#include "Misc/DynamicLibrary.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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

        bool IsDiscreteGPU() const NOEXCEPT;
        bool IsIntegratedGPU() const NOEXCEPT;

        float PerfRating() const NOEXCEPT;

        EVulkanVersion ApiVersion() const { return static_cast<EVulkanVersion>(Properties.apiVersion); }
        FStringView DeviceName() const { return MakeCStringView(Properties.deviceName); }
        EVulkanVendor VendorID() const { return static_cast<EVulkanVendor>(Properties.vendorID); }

        friend FTextWriter& operator <<(FTextWriter& oss, const FPhysicalDeviceInfo& info);
        friend FWTextWriter& operator <<(FWTextWriter& oss, const FPhysicalDeviceInfo& info);
    };
    using FPhysicalDeviceInfoRef = TPtrRef<const FPhysicalDeviceInfo>;

    FVulkanInstance();
    ~FVulkanInstance();

    bool Valid() const { return (VK_NULL_HANDLE != _vkInstance); }

    VkInstance vkInstance() const { return _vkInstance; }
    const VkAllocationCallbacks* vkAllocationCallbacks() const { return MakePtrRef(_vkAllocationCallbacks); }

    EVulkanVersion Version() const { return _instanceAPI.version_; }
    const FVulkanInstanceFunctions& Fn() const { return *this; }
    const FVulkanInstanceExtensionSet& Extensions() const { return _instanceAPI.instance_extensions_; }
    TMemoryView<const FPhysicalDeviceInfo> PhysicalDevices() const { return _physicalDevices.MakeView(); }

    bool HasExtension(EVulkanInstanceExtension ext) const {
        return (_instanceAPI.instance_extensions_ & ext);
    }
    bool AnyExtension(const FVulkanInstanceExtensionSet& exts) const {
        return exts.Contains(_instanceAPI.instance_extensions_ & exts);
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

    static TMemoryView<const FConstChar> DebuggingInstanceLayers(EVulkanVersion version) NOEXCEPT;
    static TMemoryView<const FConstChar> ProfilingInstanceLayers(EVulkanVersion version) NOEXCEPT;
    static TMemoryView<const FConstChar> RecommendedInstanceLayers(EVulkanVersion version) NOEXCEPT;
    static TMemoryView<const FQueueCreateInfo> RecommendedDeviceQueues(EVulkanVersion version) NOEXCEPT;

    static FVulkanInstanceExtensionSet DebuggingInstanceExtensions(EVulkanVersion version) NOEXCEPT;
    static FVulkanInstanceExtensionSet ProfilingInstanceExtensions(EVulkanVersion version) NOEXCEPT;
    static FVulkanInstanceExtensionSet RecommendedInstanceExtensions(EVulkanVersion version) NOEXCEPT;
    static FVulkanInstanceExtensionSet RequiredInstanceExtensions(EVulkanVersion version) NOEXCEPT;
    static FVulkanInstanceExtensionSet RequiredInstanceExtensions(EVulkanVersion version, FWindowHandle window) NOEXCEPT;

    static FVulkanDeviceExtensionSet DebuggingDeviceExtensions(EVulkanVersion version) NOEXCEPT;
    static FVulkanDeviceExtensionSet ProfilingDeviceExtensions(EVulkanVersion version) NOEXCEPT;
    static FVulkanDeviceExtensionSet RecommendedDeviceExtensions(EVulkanVersion version) NOEXCEPT;
    static FVulkanDeviceExtensionSet RequiredDeviceExtensions(EVulkanVersion version) NOEXCEPT;

private:
    VkInstance _vkInstance{ VK_NULL_HANDLE };
    const Meta::TOptional<VkAllocationCallbacks> _vkAllocationCallbacks;

    FVulkanExportedAPI _exportedAPI;
    FVulkanGlobalAPI _globalAPI;
    FVulkanInstanceAPI _instanceAPI;

    VECTOR(RHIInstance, FPhysicalDeviceInfo) _physicalDevices;

    FDynamicLibrary _vulkanLib;

#if USE_PPE_LOGGER
    VkDebugUtilsMessengerEXT _vkDebugUtilsMessenger{ VK_NULL_HANDLE };
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
