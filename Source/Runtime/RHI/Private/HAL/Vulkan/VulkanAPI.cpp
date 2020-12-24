#include "stdafx.h"

#include "HAL/Vulkan/VulkanAPI.h"

#include "HAL/PlatformMaths.h" // popcnt
#include "HAL/Vulkan/VulkanError.h"

#include "Allocator/Alloca.h"
#include "Diagnostic/Logger.h"
#include "IO/ConstChar.h"
#include "Misc/DynamicLibrary.h"

#if USE_PPE_LOGGER
#   include "IO/FormatHelpers.h"
#endif

#include <algorithm>

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(, Vulkan)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _PFN>
NODISCARD static bool vkExportedProc_(
    FVulkanInstanceFunctions* pInstanceFuncs,
    const char* name, _PFN FVulkanInstanceFunctions::* (pMember),
    const FDynamicLibrary& vulkanLib) {
    Assert(vulkanLib.IsValid());
    _PFN const pfn = static_cast<_PFN>(vulkanLib.FunctionAddr( name ));

    if (not pfn) {
        LOG(Vulkan, Error, L"failed to bind exported function <{0}>, abort loader", MakeCStringView(name));
        return false;
    }

    pInstanceFuncs->*pMember = pfn;
    return true;
}
//----------------------------------------------------------------------------
template <typename _PFN>
NODISCARD static bool vkInstanceProc_(
    FVulkanInstanceFunctions* pInstanceFuncs,
    const char* name, _PFN FVulkanInstanceFunctions::* (pMember),
    VkInstance vkInstance ) {
    Assert(pInstanceFuncs->vkGetInstanceProcAddr);
    _PFN const pfn = reinterpret_cast<_PFN>(pInstanceFuncs->vkGetInstanceProcAddr( vkInstance, name ));

    if (not pfn) {
        LOG(Vulkan, Error, L"failed to bind instance function <{0}>, abort loader", MakeCStringView(name));
        return false;
    }

    pInstanceFuncs->*pMember = pfn;
    return true;
}
//----------------------------------------------------------------------------
template <typename _PFN>
NODISCARD static bool vkDeviceProc_(
    FVulkanDeviceFunctions* pDeviceFuncs,
    const char* name, _PFN FVulkanDeviceFunctions::* (pMember),
    const FVulkanInstanceFunctions& instanceFuncs, VkDevice vkDevice) {
    Assert(instanceFuncs.vkGetDeviceProcAddr);
    _PFN const pfn = reinterpret_cast<_PFN>(instanceFuncs.vkGetDeviceProcAddr(vkDevice, name));

    if (not pfn) {
        LOG(Vulkan, Error, L"failed to bind device function <{0}> for vkDevice <{1}>, abort loader", MakeCStringView(name), vkDevice);
        return false;
    }

    pDeviceFuncs->*pMember = pfn;
    return true;
}
//----------------------------------------------------------------------------
template <typename _vkEnumerateProperties, typename T, typename _Str>
NODISCARD static bool vkValidateNames_(
    const char* funcName,
    TArray<const char*>* pNeededNames,
    _vkEnumerateProperties&& vkEnumarateProperties,
    _Str T::* (pPropertyName) ) {
    Assert(pNeededNames);
    Assert(pPropertyName);
    UNUSED(funcName);

    if (not pNeededNames->empty()) {
        VkResult vkResult;

        u32 numProperties;
        vkResult = vkEnumarateProperties(&numProperties, nullptr);
        if (VK_SUCCESS != vkResult) {
            LOG(Vulkan, Error, L"{0}: failed with result = {1}", MakeCStringView(funcName), FVulkanError(vkResult));
            return false;
        }

        STACKLOCAL_POD_ARRAY(T, properties, numProperties);
        vkResult = vkEnumarateProperties(&numProperties, properties.data());
        if (VK_SUCCESS != vkResult) {
            LOG(Vulkan, Error, L"{0}: failed with result = {1}", MakeCStringView(funcName), FVulkanError(vkResult));
            return false;
        }

        LOG(RHI, Verbose, L"{0}: found {1} results = [ {2} ]",
            MakeCStringView(funcName),
            properties.size(),
            Fmt::Join(MakeOutputIterable(properties.begin(), properties.end(),
                [pPropertyName](const T& p) NOEXCEPT{
                    return MakeCStringView(p.*pPropertyName);
                }),
                MakeCStringView(L", ")) );

        for (auto it = pNeededNames->begin(); it != pNeededNames->end(); ++it) {
            const auto jt = std::find_if(properties.begin(), properties.end(),
                [pPropertyName, key{ FConstChar{ *it } }](const T& p) NOEXCEPT{
                    return key.EqualsI(p.*pPropertyName);
                });

            if (properties.end() == jt) {
                LOG(Vulkan, Error, L"{0}: failed to find required property <{1}>", MakeCStringView(funcName), MakeCStringView(*it));
                return false;
            }
        }
    }
    return true;
}
//----------------------------------------------------------------------------
static bool vkCheckDeviceFeatures_(const VkPhysicalDeviceFeatures& device, const VkPhysicalDeviceFeatures& needed) {
    CONSTEXPR size_t numFeatures = (sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32));
    forrange(f, 0, numFeatures) {
        VkBool32 const lhs = reinterpret_cast<const VkBool32*>(&device)[f];
        VkBool32 const rhs = reinterpret_cast<const VkBool32*>(&needed)[f];

        if (rhs && !lhs)
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FVulkanInstanceFunctions::LoadSharedLib(FDynamicLibrary* pVulkanLib) {
    Assert(pVulkanLib);

    const wchar_t* vkModulePath =
#if     defined(VK_USE_PLATFORM_WIN32_KHR)
        L"vulkan-1.dll";
#elif   defined(VK_USE_PLATFORM_XCB_KHR) or defined(VK_USE_PLATFORM_XLIB_KHR)
        L"libvulkan.so";
#else
#       error "unknown platform"
#endif

    return pVulkanLib->AttachOrLoad(vkModulePath);
}
//----------------------------------------------------------------------------
bool FVulkanInstanceFunctions::AttachGlobal(FVulkanInstanceFunctions* pInstanceFuncs, const FDynamicLibrary& vulkanLib) {
    Assert(pInstanceFuncs);
    Assert(vulkanLib.IsValid());

    bool avail = true;

    #define VK_EXPORTED_FUNCTION( name ) avail &= vkExportedProc_( pInstanceFuncs, STRINGIZE(name), &FVulkanInstanceFunctions::name, vulkanLib );
    #define VK_GLOBAL_LEVEL_FUNCTION( name ) avail &= vkInstanceProc_( pInstanceFuncs, STRINGIZE(name), &FVulkanInstanceFunctions::name, VK_NULL_HANDLE );
    #define VK_INSTANCE_LEVEL_FUNCTION( name )
    #define VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( name, extension )
    #define VK_DEVICE_LEVEL_FUNCTION( name )
    #define VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION( name, extension )

    #include "vulkan-exports.inl"

    #undef VK_EXPORTED_FUNCTION
    #undef VK_GLOBAL_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION
    #undef VK_DEVICE_LEVEL_FUNCTION
    #undef VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION

    return avail;
}
//----------------------------------------------------------------------------
bool FVulkanInstanceFunctions::AttachInstance(FVulkanInstanceFunctions* pInstanceFuncs, const FDynamicLibrary& vulkanLib, VkInstance vkInstance) {
    Assert(pInstanceFuncs);
    Assert(vulkanLib.IsValid());
    Assert(VK_NULL_HANDLE != vkInstance);

    bool avail = true;

    #define VK_EXPORTED_FUNCTION( name )
    #define VK_GLOBAL_LEVEL_FUNCTION( name )
    #define VK_INSTANCE_LEVEL_FUNCTION( name ) avail &= vkInstanceProc_( pInstanceFuncs, STRINGIZE(name), &FVulkanInstanceFunctions::name, vkInstance );
    #define VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( name, extension ) avail &= vkInstanceProc_( pInstanceFuncs, STRINGIZE(name), &FVulkanInstanceFunctions::name, vkInstance );
    #define VK_DEVICE_LEVEL_FUNCTION( name )
    #define VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION( name, extension )

    #include "vulkan-exports.inl"

    #undef VK_EXPORTED_FUNCTION
    #undef VK_GLOBAL_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION
    #undef VK_DEVICE_LEVEL_FUNCTION
    #undef VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION

    return avail;
}
//----------------------------------------------------------------------------
void FVulkanInstanceFunctions::Detach(FVulkanInstanceFunctions* pInstanceFuncs) {
    Assert(pInstanceFuncs);

    #define VK_EXPORTED_FUNCTION( name ) pInstanceFuncs->name = nullptr;
    #define VK_GLOBAL_LEVEL_FUNCTION( name ) pInstanceFuncs->name = nullptr;
    #define VK_INSTANCE_LEVEL_FUNCTION( name ) pInstanceFuncs->name = nullptr;
    #define VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( name, extension ) pInstanceFuncs->name = nullptr;
    #define VK_DEVICE_LEVEL_FUNCTION( name )
    #define VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION( name, extension )

    #include "vulkan-exports.inl"

    #undef VK_EXPORTED_FUNCTION
    #undef VK_GLOBAL_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION
    #undef VK_DEVICE_LEVEL_FUNCTION
    #undef VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION
}
//----------------------------------------------------------------------------
bool FVulkanInstanceFunctions::SetupExtensions(
    TArray<const char*>* pExtensions,
    bool headless, bool debug ) const {
    Assert(pExtensions);

    if (not headless) {
        pExtensions->push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        pExtensions->push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
#if     defined(VK_USE_PLATFORM_WIN32_KHR)
        pExtensions->push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif   defined(VK_USE_PLATFORM_ANDROID_KHR)
        pExtensions->push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif   defined(_DIRECT2DISPLAY)
        pExtensions->push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif   defined(VK_USE_PLATFORM_WAYLAND_KHR)
        pExtensions->push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif   defined(VK_USE_PLATFORM_XCB_KHR)
        pExtensions->push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif   defined(VK_USE_PLATFORM_IOS_MVK)
        pExtensions->push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif   defined(VK_USE_PLATFORM_MACOS_MVK)
        pExtensions->push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif
    }
#if USE_PPE_RHIDEBUG
    if (debug) {
        pExtensions->push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        pExtensions->push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        pExtensions->push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
#else
    UNUSED(debug);
#endif

    const bool validated = vkValidateNames_(
        "vkEnumerateInstanceExtensionProperties", pExtensions,
        [this](u32* pNumProperties, VkExtensionProperties* pProperties) {
            return vkEnumerateInstanceExtensionProperties(nullptr, pNumProperties, pProperties);
        },  &VkExtensionProperties::extensionName );

    if (validated) {
        return true;
    }
    else {
        LOG(Vulkan, Fatal, L"failed to validate instance extensions");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FVulkanInstanceFunctions::SetupLayers(
    TArray<const char*>* pLayers,
    bool debug ) const {
    Assert(pLayers);

#if USE_PPE_RHIDEBUG
    if (debug) {
        // other validation layers have been deprecated
        // https://vulkan.lunarg.com/doc/view/1.1.114.0/windows/validation_layers.html
        pLayers->push_back("VK_LAYER_KHRONOS_validation");
    }
#else
    UNUSED(debug);
#endif

    const bool validated = vkValidateNames_(
        "vkEnumerateInstanceLayerProperties", pLayers,
        [this](u32* pNumProperties, VkLayerProperties* pProperties) {
            return vkEnumerateInstanceLayerProperties(pNumProperties, pProperties);
        },  &VkLayerProperties::layerName );

    if (validated) {
        return true;
    }
    else {
        LOG(Vulkan, Fatal, L"failed to validate instance layers");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FVulkanInstanceFunctions::SetupSwapChain(
    TArray<VkPresentModeKHR>* pPresentModes,
    TArray<VkSurfaceFormatKHR>* pSurfaceFormats,
    VkPhysicalDevice vkPhysicalDevice,
    VkSurfaceKHR vkSurfaceIFN/* = nullptr */) const {
    Assert(VK_NULL_HANDLE != vkPhysicalDevice);

    u32 numPresentModes = 0;
    VkResult vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurfaceIFN, &numPresentModes, nullptr);
    if (VK_SUCCESS != vkResult) {
        LOG(Vulkan, Error, L"vkGetPhysicalDeviceSurfacePresentModesKHR: failed with result = {0}", FVulkanError(vkResult));
        return false;
    }

    if (pPresentModes && numPresentModes > 0) {
        pPresentModes->resize_Uninitialized(numPresentModes);
        vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurfaceIFN, &numPresentModes, pPresentModes->data());
        if (VK_SUCCESS != vkResult) {
            LOG(Vulkan, Error, L"vkGetPhysicalDeviceSurfacePresentModesKHR: failed with result = {0}", FVulkanError(vkResult));
            return false;
        }
    }

    u32 numSurfaceFormats = 0;
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurfaceIFN, &numSurfaceFormats, nullptr);
    if (VK_SUCCESS != vkResult) {
        LOG(Vulkan, Error, L"vkGetPhysicalDeviceSurfaceFormatsKHR: failed with result = {0}", FVulkanError(vkResult));
        return false;
    }

    if (pSurfaceFormats && numSurfaceFormats > 0) {
        pSurfaceFormats->resize_Uninitialized(numSurfaceFormats);
        vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurfaceIFN, &numSurfaceFormats, pSurfaceFormats->data());
        if (VK_SUCCESS != vkResult) {
            LOG(Vulkan, Error, L"vkGetPhysicalDeviceSurfaceFormatsKHR: failed with result = {0}", FVulkanError(vkResult));
            return false;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
void FVulkanInstanceFunctions::SetupQueueFamilies(
    TArray<VkDeviceQueueCreateInfo>* pDeviceQueues,
    TArray<VkQueueFamilyProperties>* pQueueFamilies,
    VkPhysicalDevice vkPhysicalDevice) const {
    Assert(pDeviceQueues);
    Assert(pQueueFamilies);
    Assert(VK_NULL_HANDLE != vkPhysicalDevice);

    u32 numQueueFamilies = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &numQueueFamilies, nullptr);

    pQueueFamilies->resize_Uninitialized(numQueueFamilies);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &numQueueFamilies, pQueueFamilies->data());

    static const float GDefaultQueuePriorities[] = {
        1.00f,0.95f,0.90f,0.85f,0.80f,0.75f,0.70f,
        0.65f,0.60f,0.55f,0.50f,0.45f,0.40f,0.35f,
        0.30f,0.25f,0.20f,0.15f,0.10f,0.05f,0.00f
    };

    pDeviceQueues->resize_Uninitialized(numQueueFamilies);
    forrange(i, 0, numQueueFamilies) {
        VkDeviceQueueCreateInfo& dst = pDeviceQueues->at(i);

        dst = VkDeviceQueueCreateInfo{};
        dst.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        dst.queueFamilyIndex = i;
        dst.queueCount = 0; // will be incremented later if this family is selected
        dst.pQueuePriorities = GDefaultQueuePriorities;
        dst.flags = 0; // only used for VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT
        dst.pNext = VK_NULL_HANDLE; // only used for VkDeviceQueueGlobalPriorityCreateInfoEXT
    }

    // sort device queues from least flags to most flags count (see SetupDeviceQueue)
    std::sort(pDeviceQueues->begin(), pDeviceQueues->end(),
        [&families{ *pQueueFamilies }](const VkDeviceQueueCreateInfo& lhs, const VkDeviceQueueCreateInfo& rhs) {
            return (FPlatformMaths::popcnt(families[lhs.queueFamilyIndex].queueFlags) <
                    FPlatformMaths::popcnt(families[rhs.queueFamilyIndex].queueFlags) );
        });
}
//----------------------------------------------------------------------------
bool FVulkanInstanceFunctions::SetupDeviceQueue(
    u32* pFamilyIndex,
    u32* pFamilyQueueIndex,
    VkPhysicalDevice vkPhysicalDevice,
    TArray<VkDeviceQueueCreateInfo>& deviceQueues,
    const TArray<VkQueueFamilyProperties>& queueFamilies,
    VkFlags vkQueueFlags,
    VkSurfaceKHR vkSurfaceIFN/* = VK_NULL_HANDLE */) const {
    Assert(pFamilyIndex);
    Assert(pFamilyQueueIndex);
    Assert(queueFamilies.size() == deviceQueues.size());

    u32 bestDeviceQueue = UINT32_MAX;
    forrange(i, 0, checked_cast<u32>(deviceQueues.size())) {
        const u32 queueFamily = deviceQueues[i].queueFamilyIndex;
        const VkQueueFamilyProperties& vkQueueFamily = queueFamilies[queueFamily];
        if ((vkQueueFamily.queueFlags & vkQueueFlags) != vkQueueFlags)
            continue;

        if (VK_NULL_HANDLE != vkSurfaceIFN) {
            VkBool32 supportForPresent = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurfaceIFN, &supportForPresent);

            if (not supportForPresent)
                continue;
        }

        bestDeviceQueue = i;
        if (0 == deviceQueues[i].queueCount)
            break; // spread queues across all families
    }

    if (UINT32_MAX == bestDeviceQueue) {
        *pFamilyIndex = UINT32_MAX;
        *pFamilyQueueIndex = UINT32_MAX;
        return false;
    }

    *pFamilyIndex = deviceQueues[bestDeviceQueue].queueFamilyIndex;
    *pFamilyQueueIndex = deviceQueues[bestDeviceQueue].queueCount++;
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanInstanceFunctions::PickPhysicalDevices(
    TArray<VkPhysicalDevice>* pPhysicalDevices,
    VkInstance vkInstance,
    const VkPhysicalDeviceFeatures& vkDeviceFeatures,
    VkSurfaceKHR vkSurfaceIFN/* = VK_NULL_HANDLE */) const {
    Assert(VK_NULL_HANDLE != vkInstance);
    Assert(pPhysicalDevices);

    u32 numPhysicalDevices = 0;
    VkResult result = vkEnumeratePhysicalDevices(vkInstance, &numPhysicalDevices, nullptr);
    if (VK_SUCCESS != result)
        PPE_THROW_IT(FVulkanInstanceException("vkEnumeratePhysicalDevices", result));

    pPhysicalDevices->resize_Uninitialized(numPhysicalDevices);
    result = vkEnumeratePhysicalDevices(vkInstance, &numPhysicalDevices, pPhysicalDevices->data());
    if (VK_SUCCESS != result)
        PPE_THROW_IT(FVulkanInstanceException("vkEnumeratePhysicalDevices", result));

    for (auto it = pPhysicalDevices->begin(); it != pPhysicalDevices->end(); ) {
        bool validated = true;

        // validate device features
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(*it, &features);
        if (not vkCheckDeviceFeatures_(features, vkDeviceFeatures))
            validated = false;

        // validate swap-chain (if surface provided)
        if (VK_NULL_HANDLE != vkSurfaceIFN && not SetupSwapChain(nullptr, nullptr, *it, vkSurfaceIFN))
            validated = false;

        // remove device if not validated
        if (validated)
            ++it;
        else
            pPhysicalDevices->erase_DontPreserveOrder(it);
    }

    // sort from best performance score to worst
    auto vkDeviceScore = [this](VkPhysicalDevice vkPhysicalDevice) -> u64 {
        VkPhysicalDeviceProperties vkProperties;
        vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vkProperties);

        u64 score = 0;
        if (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == vkProperties.deviceType)
            score += 1000ul * 1000ul;

        score += vkProperties.limits.maxImageDimension2D * vkProperties.limits.maxImageArrayLayers;
        score += vkProperties.limits.maxImageDimension3D * vkProperties.limits.maxImageArrayLayers;
        score += vkProperties.limits.maxImageDimensionCube * vkProperties.limits.maxImageArrayLayers;

        return score;
    };
    std::sort(pPhysicalDevices->begin(), pPhysicalDevices->end(),
        [&vkDeviceScore](VkPhysicalDevice lhs, VkPhysicalDevice rhs) {
            return (vkDeviceScore(lhs) > vkDeviceScore(rhs));
        });

    return (not pPhysicalDevices->empty());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FVulkanDeviceFunctions::AttachDevice(
    FVulkanDeviceFunctions* pDeviceFuncs,
    const FVulkanInstanceFunctions& instanceFuncs,
    VkDevice vkDevice ) {
    Assert(pDeviceFuncs);
    Assert(vkDevice != VK_NULL_HANDLE);

    bool avail = true;

    #define VK_EXPORTED_FUNCTION( name )
    #define VK_GLOBAL_LEVEL_FUNCTION( name )
    #define VK_INSTANCE_LEVEL_FUNCTION( name )
    #define VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( name, extension )
    #define VK_DEVICE_LEVEL_FUNCTION( name ) avail &= vkDeviceProc_( pDeviceFuncs, STRINGIZE(name), &FVulkanDeviceFunctions::name, instanceFuncs, vkDevice );
    #define VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION( name, extension )

    #include "vulkan-exports.inl"

    #undef VK_EXPORTED_FUNCTION
    #undef VK_GLOBAL_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION
    #undef VK_DEVICE_LEVEL_FUNCTION
    #undef VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION

    return avail;
}
//----------------------------------------------------------------------------
bool FVulkanDeviceFunctions::AttachExtensions(
    FVulkanDeviceFunctions* pDeviceFuncs,
    const FVulkanInstanceFunctions& instanceFuncs,
    VkDevice vkDevice ) {
    Assert(pDeviceFuncs);
    Assert(vkDevice != VK_NULL_HANDLE);

    bool avail = true;

    #define VK_EXPORTED_FUNCTION( name )
    #define VK_GLOBAL_LEVEL_FUNCTION( name )
    #define VK_INSTANCE_LEVEL_FUNCTION( name )
    #define VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( name, extension )
    #define VK_DEVICE_LEVEL_FUNCTION( name )
    #define VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION( name, extension ) avail &= vkDeviceProc_( pDeviceFuncs, STRINGIZE(name), &FVulkanDeviceFunctions::name, instanceFuncs, vkDevice );

    #include "vulkan-exports.inl"

    #undef VK_EXPORTED_FUNCTION
    #undef VK_GLOBAL_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION
    #undef VK_DEVICE_LEVEL_FUNCTION
    #undef VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION

    return avail;
}
//----------------------------------------------------------------------------
void FVulkanDeviceFunctions::Detach(FVulkanDeviceFunctions* pDeviceFuncs) {
    Assert(pDeviceFuncs);

    #define VK_EXPORTED_FUNCTION( name )
    #define VK_GLOBAL_LEVEL_FUNCTION( name )
    #define VK_INSTANCE_LEVEL_FUNCTION( name )
    #define VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( name, extension )
    #define VK_DEVICE_LEVEL_FUNCTION( name ) pDeviceFuncs->name = nullptr;
    #define VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION( name, extension ) pDeviceFuncs->name = nullptr;

    #include "vulkan-exports.inl"

    #undef VK_EXPORTED_FUNCTION
    #undef VK_GLOBAL_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION
    #undef VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION
    #undef VK_DEVICE_LEVEL_FUNCTION
    #undef VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION
}
//----------------------------------------------------------------------------
bool FVulkanDeviceFunctions::SetupExtensions(
    TArray<const char*>* pExtensions,
    const FVulkanInstanceFunctions& instanceFuncs,
    VkPhysicalDevice vkPysicalDevice,
    bool swapchain ) {
    Assert(pExtensions);

    if (swapchain) {
        pExtensions->push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        //pExtensions->push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
    }

#if VK_EXT_conservative_rasterization
    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_EXT_conservative_rasterization.html
    pExtensions->push_back(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
#endif

    const bool validated = vkValidateNames_(
        "vkEnumerateDeviceExtensionProperties", pExtensions,
        [&instanceFuncs, vkPysicalDevice](u32* pNumProperties, VkExtensionProperties* pProperties) {
            return instanceFuncs.vkEnumerateDeviceExtensionProperties(vkPysicalDevice, nullptr, pNumProperties, pProperties);
        },  &VkExtensionProperties::extensionName );

    if (validated) {
        return true;
    }
    else {
        LOG(Vulkan, Fatal, L"failed to validate device extensions");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FVulkanDeviceFunctions::SetupLayers(
    TArray<const char*>* pLayers,
    const FVulkanInstanceFunctions& instanceFuncs,
    VkPhysicalDevice vkPysicalDevice,
    bool debug) {
    Assert(pLayers);

#if USE_PPE_RHIDEBUG
    if (debug) {
        // other validation layers have been deprecated
        // https://vulkan.lunarg.com/doc/view/1.1.114.0/windows/validation_layers.html
        pLayers->push_back("VK_LAYER_KHRONOS_validation");
    }
#else
    UNUSED(debug);
#endif

    const bool validated = vkValidateNames_(
        "vkEnumerateDeviceLayerProperties", pLayers,
        [&instanceFuncs, vkPysicalDevice](u32* pNumProperties, VkLayerProperties* pProperties) {
            return instanceFuncs.vkEnumerateDeviceLayerProperties(vkPysicalDevice, pNumProperties, pProperties);
        },  &VkLayerProperties::layerName );

    if (validated) {
        return true;
    }
    else {
        LOG(Vulkan, Fatal, L"failed to validate device layers");
        return false;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
