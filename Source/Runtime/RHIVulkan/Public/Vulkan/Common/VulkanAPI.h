#pragma once

#include "RHIVulkan_fwd.h"

#include "Vulkan/VulkanIncludes.h"

#include "IO/TextWriter_fwd.h"

// API without Secrets: Introduction to Vulkan* Part 1: The Beginning
// https://software.intel.com/content/www/us/en/develop/articles/api-without-secrets-introduction-to-vulkan-part-1.html

#define USE_PPE_RHIEXCEPTIONHOOK (0)

#if USE_PPE_RHIEXCEPTIONHOOK
#   include "Misc/Function.h"
#   define VK_PFN_BINDING( name ) TFunction<CONCAT(PFN_, name)> name;
#else
#   define VK_PFN_BINDING( name ) CONCAT(PFN_, name) name{ nullptr };
#endif

namespace PPE {
class FDynamicLibrary;
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
FTextWriter& operator <<(FTextWriter& oss, EVulkanVendor vendor);
FWTextWriter& operator <<(FWTextWriter& oss, EVulkanVendor vendor);
//----------------------------------------------------------------------------
struct FVulkanInstanceFunctions {
protected:
    static EVulkanVendor IdentifyVendorId(u32 vendorId) NOEXCEPT;

    static bool LoadSharedLib(FDynamicLibrary* pVulkanLib);

    static bool AttachGlobal(FVulkanInstanceFunctions* pInstanceFuncs, const FDynamicLibrary& vulkanLib);

    bool SetupExtensions(TArray<const char*>* pExtensions, bool headless, bool debug) const;
    bool SetupLayers(TArray<const char*>* pLayers, bool debug) const;

    static bool AttachInstance(FVulkanInstanceFunctions* pInstanceFuncs, const FDynamicLibrary& vulkanLib, VkInstance vkInstance);

    bool SetupSwapChain(
        TArray<VkPresentModeKHR>* pPresentModes,
        TArray<VkSurfaceFormatKHR>* pSurfaceFormats,
        VkPhysicalDevice vkPhysicalDevice,
        VkSurfaceKHR vkSurfaceIFN = VK_NULL_HANDLE ) const;

    void SetupQueueFamilies(
        TArray<VkDeviceQueueCreateInfo>* pDeviceQueues,
        TArray<VkQueueFamilyProperties>* pQueueFamilies,
        VkPhysicalDevice vkPhysicalDevice ) const;

    bool SetupDeviceQueue(
        u32* pFamilyIndex,
        u32* pFamilyQueueIndex,
        VkPhysicalDevice vkPhysicalDevice,
        TArray<VkDeviceQueueCreateInfo>& deviceQueues,
        const TArray<VkQueueFamilyProperties>& queueFamilies,
        VkFlags vkQueueFlags,
        VkSurfaceKHR vkSurfaceIFN = VK_NULL_HANDLE ) const;

    bool PickPhysicalDevices(
        TArray<VkPhysicalDevice>* pPhysicalDevices,
        VkInstance vkInstance,
        const VkPhysicalDeviceFeatures& vkDeviceFeatures,
        VkSurfaceKHR vkSurfaceIFN = VK_NULL_HANDLE ) const;

    static void Detach(FVulkanInstanceFunctions* pInstanceFuncs);

public:
    #define VK_EXPORTED_FUNCTION( name ) VK_PFN_BINDING( name )
    #define VK_GLOBAL_LEVEL_FUNCTION( name ) VK_PFN_BINDING( name )
    #define VK_INSTANCE_LEVEL_FUNCTION( name ) VK_PFN_BINDING( name )
    #define VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( name, extension ) VK_PFN_BINDING( name )
    #define VK_DEVICE_LEVEL_FUNCTION( name )
    #define VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION( name, extension )

    #include "External/vulkan/Public/vulkan-exports.inl"

    #undef VK_EXPORTED_FUNCTION
    #undef VK_GLOBAL_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION
    #undef VK_DEVICE_LEVEL_FUNCTION
    #undef VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION
};
//----------------------------------------------------------------------------
struct FVulkanDeviceFunctions {
protected:
    friend class FVulkanInstance;

    static bool SetupExtensions(TArray<const char*>* pExtensions, const FVulkanInstanceFunctions& instanceFuncs, VkPhysicalDevice vkPysicalDevice, bool swapchain);
    static bool SetupLayers(TArray<const char*>* pLayers, const FVulkanInstanceFunctions& instanceFuncs, VkPhysicalDevice vkPysicalDevice, bool debug);

    static bool AttachDevice(FVulkanDeviceFunctions* pDeviceFuncs, const FVulkanInstanceFunctions& instanceFuncs, VkDevice vkDevice);

    static void Detach(FVulkanDeviceFunctions* pDeviceFuncs);

public:
    #define VK_EXPORTED_FUNCTION( name )
    #define VK_GLOBAL_LEVEL_FUNCTION( name )
    #define VK_INSTANCE_LEVEL_FUNCTION( name )
    #define VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( name, extension )
    #define VK_DEVICE_LEVEL_FUNCTION( name ) VK_PFN_BINDING( name )
    #define VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION( name, extension ) VK_PFN_BINDING( name )

    #include "External/vulkan/Public/vulkan-exports.inl"

    #undef VK_EXPORTED_FUNCTION
    #undef VK_GLOBAL_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION
    #undef VK_DEVICE_LEVEL_FUNCTION
    #undef VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#undef VK_PFN_BINDING
