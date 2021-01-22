#include "stdafx.h"

#ifdef RHI_VULKAN

#include "Vulkan/VulkanInstance.h"

#include "Vulkan/VulkanDebug.h"
#include "Vulkan/VulkanError.h"
#include "Vulkan/VulkanDevice.h"

#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"
#include "Memory/MemoryDomain.h"
#include "Meta/Optional.h"
#include "Modular/ModularDomain.h"
#include "RHIModule.h"

#if USE_PPE_LOGGER
#    include "IO/FormatHelpers.h"
#endif

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI)
LOG_CATEGORY(, Vulkan)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
static FMemoryTracking& AllocationTrackingData_(VkSystemAllocationScope vkScope) {
    switch (vkScope) {
    case VK_SYSTEM_ALLOCATION_SCOPE_COMMAND: return MEMORYDOMAIN_TRACKING_DATA(RHICommand);
    case VK_SYSTEM_ALLOCATION_SCOPE_OBJECT: return MEMORYDOMAIN_TRACKING_DATA(RHIObject);
    case VK_SYSTEM_ALLOCATION_SCOPE_CACHE: return MEMORYDOMAIN_TRACKING_DATA(RHICache);
    case VK_SYSTEM_ALLOCATION_SCOPE_DEVICE: return MEMORYDOMAIN_TRACKING_DATA(RHIDevice);
    case VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE: return MEMORYDOMAIN_TRACKING_DATA(RHIInstance);
    default:
        AssertNotImplemented();
    }
}
#endif
//----------------------------------------------------------------------------
static VkAllocationCallbacks MakeAllocatorCallbacks_() {
    PFN_vkAllocationFunction vkAllocation = [](void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
        UNUSED(pUserData);
#if USE_PPE_MEMORYDOMAINS
        return PPE::tracking_aligned_malloc(AllocationTrackingData_(allocationScope), size, alignment);
#else
        UNUSED(alignment);
        UNUSED(allocationScope);
        Assert_NoAssume(alignment < ALLOCATION_BOUNDARY);
        void* const p = PPE::malloc(size);
        Assert_NoAssume(Meta::IsAligned(alignment, p));
        return p;
#endif
    };
    PFN_vkReallocationFunction vkReallocation = [](void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
        UNUSED(pUserData);
#if USE_PPE_MEMORYDOMAINS
        return PPE::tracking_aligned_realloc(AllocationTrackingData_(allocationScope), pOriginal, size, alignment);
#else
        UNUSED(alignment);
        UNUSED(allocationScope);
        Assert_NoAssume(alignment < ALLOCATION_BOUNDARY);
        void* const p = PPE::realloc(pOriginal, size);
        Assert_NoAssume(Meta::IsAligned(alignment, p));
        return p;
#endif
    };
    PFN_vkFreeFunction vkFree = [](void* pUserData, void* pMemory) {
        UNUSED(pUserData);
#if USE_PPE_MEMORYDOMAINS
        PPE::tracking_free(pMemory);
#else
        PPE::free(pMemory);
#endif
    };

#if USE_PPE_MEMORYDOMAINS
    // Notified when Vulkan make an internal allocation without our own allocator,
    // this is intended only for tracking purpose (and shouldn't be a common need).
    PFN_vkInternalAllocationNotification vkInternalAllocationNotif = [](void*, size_t size,
        VkInternalAllocationType allocationType,
        VkSystemAllocationScope allocationScope) {
        UNUSED(allocationType);
        UNUSED(allocationScope);
        MEMORYDOMAIN_TRACKING_DATA(RHIInternal).Allocate(size, size);
    };
    PFN_vkInternalFreeNotification vkInternalFreeNotif = [](void*, size_t size,
        VkInternalAllocationType allocationType,
        VkSystemAllocationScope allocationScope) {
            UNUSED(allocationType);
            UNUSED(allocationScope);
            MEMORYDOMAIN_TRACKING_DATA(RHIInternal).Deallocate(size, size);
    };
#endif

    void* const pUserData = nullptr;
    return VkAllocationCallbacks{
        pUserData,
        vkAllocation, vkReallocation, vkFree,
#if USE_PPE_MEMORYDOMAINS
        vkInternalAllocationNotif, vkInternalFreeNotif
#else
        nullptr, nullptr
#endif
    };
}
//----------------------------------------------------------------------------
#if USE_PPE_VULKAN_DEBUGLAYER
VkDebugUtilsMessengerEXT CreateDebugLogger_(
    VkInstance vkInstance,
    const VkAllocationCallbacks* vkAllocator,
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT ) {
#if USE_PPE_LOGGER
    PFN_vkDebugUtilsMessengerCallbackEXT debugCallback = [](
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) -> VkBool32 {
            UNUSED(pUserData);

            ELoggerVerbosity level;
            switch (messageSeverity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: level = ELoggerVerbosity::Verbose; break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: level = ELoggerVerbosity::Info; break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: level = ELoggerVerbosity::Warning; break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: level = ELoggerVerbosity::Error; break;
            default:
                AssertNotImplemented();
            }

            auto fmtMessageType = [messageType](FWTextWriter& oss) {
                if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
                    oss << L"[GENERAL] ";
                if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
                    oss << L"[VALIDATION] ";
                if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
                    oss << L"[PERFORMANCE] ";
            };
            auto fmtCallbackData = [pCallbackData](FWTextWriter& oss) {
                // QUEUES
                const TMemoryView<const VkDebugUtilsLabelEXT> queues{ pCallbackData->pQueueLabels, pCallbackData->queueLabelCount };
                for (const VkDebugUtilsLabelEXT& queue : queues) {
                    oss << Eol;
                    Format(oss, L" --QUEUE-- \"{0}\"", MakeCStringView(queue.pLabelName));
                }
                // COMMAND-BUFFERS
                const TMemoryView<const VkDebugUtilsLabelEXT> cmdBufs{ pCallbackData->pCmdBufLabels, pCallbackData->cmdBufLabelCount };
                for (const VkDebugUtilsLabelEXT& cmd : cmdBufs) {
                    oss << Eol;
                    Format(oss, L" --CMDBUF-- \"{0}\"", MakeCStringView(cmd.pLabelName));
                }
                // OBJECTS
                const TMemoryView<const VkDebugUtilsObjectNameInfoEXT> objects{ pCallbackData->pObjects, pCallbackData->objectCount };
                for (const VkDebugUtilsObjectNameInfoEXT& obj : objects) {
                    oss << Eol;

                    FWStringView objectTypeName;
                    switch (obj.objectType) {
                    case VK_OBJECT_TYPE_UNKNOWN: objectTypeName = L"UNKNOWN"; break;
                    case VK_OBJECT_TYPE_INSTANCE: objectTypeName = L"INSTANCE"; break;
                    case VK_OBJECT_TYPE_PHYSICAL_DEVICE: objectTypeName = L"PHYSICAL_DEVICE"; break;
                    case VK_OBJECT_TYPE_DEVICE: objectTypeName = L"DEVICE"; break;
                    case VK_OBJECT_TYPE_QUEUE: objectTypeName = L"QUEUE"; break;
                    case VK_OBJECT_TYPE_SEMAPHORE: objectTypeName = L"SEMAPHORE"; break;
                    case VK_OBJECT_TYPE_COMMAND_BUFFER: objectTypeName = L"COMMAND_BUFFER"; break;
                    case VK_OBJECT_TYPE_FENCE: objectTypeName = L"FENCE"; break;
                    case VK_OBJECT_TYPE_DEVICE_MEMORY: objectTypeName = L"DEVICE_MEMORY"; break;
                    case VK_OBJECT_TYPE_BUFFER: objectTypeName = L"BUFFER"; break;
                    case VK_OBJECT_TYPE_IMAGE: objectTypeName = L"IMAGE"; break;
                    case VK_OBJECT_TYPE_EVENT: objectTypeName = L"EVENT"; break;
                    case VK_OBJECT_TYPE_QUERY_POOL: objectTypeName = L"QUERY_POOL"; break;
                    case VK_OBJECT_TYPE_BUFFER_VIEW: objectTypeName = L"BUFFER_VIEW"; break;
                    case VK_OBJECT_TYPE_IMAGE_VIEW: objectTypeName = L"IMAGE_VIEW"; break;
                    case VK_OBJECT_TYPE_SHADER_MODULE: objectTypeName = L"SHADER_MODULE"; break;
                    case VK_OBJECT_TYPE_PIPELINE_CACHE: objectTypeName = L"PIPELINE_CACHE"; break;
                    case VK_OBJECT_TYPE_PIPELINE_LAYOUT: objectTypeName = L"PIPELINE_LAYOUT"; break;
                    case VK_OBJECT_TYPE_RENDER_PASS: objectTypeName = L"RENDER_PASS"; break;
                    case VK_OBJECT_TYPE_PIPELINE: objectTypeName = L"PIPELINE"; break;
                    case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT: objectTypeName = L"DESCRIPTOR_SET_LAYOUT"; break;
                    case VK_OBJECT_TYPE_SAMPLER: objectTypeName = L"SAMPLER"; break;
                    case VK_OBJECT_TYPE_DESCRIPTOR_POOL: objectTypeName = L"DESCRIPTOR_POOL"; break;
                    case VK_OBJECT_TYPE_DESCRIPTOR_SET: objectTypeName = L"DESCRIPTOR_SET"; break;
                    case VK_OBJECT_TYPE_FRAMEBUFFER: objectTypeName = L"FRAMEBUFFER"; break;
                    case VK_OBJECT_TYPE_COMMAND_POOL: objectTypeName = L"COMMAND_POOL"; break;
                    case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION: objectTypeName = L"SAMPLER_YCBCR_CONVERSION"; break;
                    case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE: objectTypeName = L"DESCRIPTOR_UPDATE_TEMPLATE"; break;
                    case VK_OBJECT_TYPE_SURFACE_KHR: objectTypeName = L"SURFACE_KHR"; break;
                    case VK_OBJECT_TYPE_SWAPCHAIN_KHR: objectTypeName = L"SWAPCHAIN_KHR"; break;
                    case VK_OBJECT_TYPE_DISPLAY_KHR: objectTypeName = L"DISPLAY_KHR"; break;
                    case VK_OBJECT_TYPE_DISPLAY_MODE_KHR: objectTypeName = L"DISPLAY_MODE_KHR"; break;
                    case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT: objectTypeName = L"DEBUG_REPORT_CALLBACK_EXT"; break;
                    case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT: objectTypeName = L"DEBUG_UTILS_MESSENGER_EXT"; break;
                    case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR: objectTypeName = L"ACCELERATION_STRUCTURE_KHR"; break;
                    case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT: objectTypeName = L"VALIDATION_CACHE_EXT"; break;
                    case VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL: objectTypeName = L"PERFORMANCE_CONFIGURATION_INTEL"; break;
                    case VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR: objectTypeName = L"DEFERRED_OPERATION_KHR"; break;
                    case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV: objectTypeName = L"INDIRECT_COMMANDS_LAYOUT_NV"; break;
                    default:
                        AssertNotImplemented();
                    }

                    Format(oss, L" --OBJECT-- {0} - <{1}> - \"{2}\"",
                        Fmt::Pointer((void*)obj.objectHandle),
                        objectTypeName,
                        MakeCStringView(obj.pObjectName));
                }
            };

            FLogger::Log(
                LOG_CATEGORY_GET(Vulkan),
                level,
                FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
                L"{3}<{1}> {0} ({2}){4}",
                MakeCStringView(pCallbackData->pMessage),
                MakeCStringView(pCallbackData->pMessageIdName),
                pCallbackData->messageIdNumber,
                Fmt::Formator<wchar_t>(fmtMessageType),
                Fmt::Formator<wchar_t>(fmtCallbackData));

            return VK_FALSE;
    };

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;

    VkDebugUtilsMessengerEXT vkDebuMessenger = VK_NULL_HANDLE;
    VkResult vkResult = vkCreateDebugUtilsMessengerEXT(vkInstance, &createInfo, vkAllocator, &vkDebuMessenger);
    if (VK_SUCCESS != vkResult)
        PPE_THROW_IT(FVulkanInstanceException("vkCreateDebugUtilsMessengerEXT", vkResult));

    return vkDebuMessenger;

#else
    UNUSED(vkInstance);
    UNUSED(vkAllocator);
    return VK_NULL_HANDLE;

#endif
}
#endif //!USE_PPE_VULKAN_DEBUGLAYER
//----------------------------------------------------------------------------
#if USE_PPE_VULKAN_DEBUGLAYER
static void DestroyDebugLogger_(
    VkDebugUtilsMessengerEXT vkDebugMessenger,
    VkInstance vkInstance,
    const VkAllocationCallbacks* vkAllocator,
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT) {
    Assert(VK_NULL_HANDLE != vkDebugMessenger);

    vkDestroyDebugUtilsMessengerEXT(vkInstance, vkDebugMessenger, vkAllocator);
}
#endif //!USE_PPE_VULKAN_DEBUGLAYER
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ETargetRHI FVulkanInstance::TargetRHI() const NOEXCEPT { return ETargetRHI::Vulkan; }
//----------------------------------------------------------------------------
bool FVulkanInstance::Create(FVulkanInstance* pInstance, ERHIFeature features) {
    Assert(pInstance);
    Assert_NoAssume(pInstance->_vkInstance == VK_NULL_HANDLE);

    const FCriticalScope scopeLock(&pInstance->_barrier);

    pInstance->_features = features;

    if (not LoadSharedLib(&pInstance->_sharedLib))
        return false;

    if (not AttachGlobal(pInstance, pInstance->_sharedLib))
        return false;

    TArray<const char*> extensionNames;
    if (not pInstance->SetupExtensions(&extensionNames, features & ERHIFeature::Headless, features & ERHIFeature::Debugging))
        return false;

    TArray<const char*> layerNames;
    if (not pInstance->SetupLayers(&layerNames, features & ERHIFeature::Debugging))
        return false;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = STRINGIZE(BUILD_TARGET_NAME) "-" STRINGIZE(BUILD_FAMILY);
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "PPE";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2; // VulkanSDK v1.2

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.pNext = nullptr;
    createInfo.enabledExtensionCount = checked_cast<u32>(extensionNames.size());
    createInfo.ppEnabledExtensionNames = extensionNames.data();
    createInfo.enabledLayerCount = checked_cast<u32>(layerNames.size());
    createInfo.ppEnabledLayerNames = layerNames.data();

    pInstance->_vkAllocator = MakeAllocatorCallbacks_();

    const VkResult result = pInstance->vkCreateInstance(&createInfo, &pInstance->_vkAllocator, &pInstance->_vkInstance);
    if (VK_SUCCESS != result)
        PPE_THROW_IT(FVulkanInstanceException("vkCreateInstance", result));

    if (not AttachInstance(pInstance, pInstance->_sharedLib, pInstance->_vkInstance))
        return false;

#if USE_PPE_VULKAN_DEBUGLAYER
    pInstance->_vkDebugMessenger = CreateDebugLogger_(pInstance->_vkInstance, &pInstance->_vkAllocator, pInstance->vkCreateDebugUtilsMessengerEXT);
#endif

    Assert_NoAssume(VK_NULL_HANDLE != pInstance->_vkInstance);
    return true;
}
//----------------------------------------------------------------------------
void FVulkanInstance::Destroy(FVulkanInstance* pInstance) {
    Assert(pInstance);

    const FCriticalScope scopeLock(&pInstance->_barrier);
    Assert_NoAssume(VK_NULL_HANDLE != pInstance->_vkInstance);

#if USE_PPE_VULKAN_DEBUGLAYER
    DestroyDebugLogger_(pInstance->_vkDebugMessenger, pInstance->_vkInstance, &pInstance->_vkAllocator, pInstance->vkDestroyDebugUtilsMessengerEXT);
#endif

    pInstance->vkDestroyInstance(pInstance->_vkInstance, &pInstance->_vkAllocator);
    pInstance->_vkInstance = VK_NULL_HANDLE;

    Detach(pInstance);
}
//----------------------------------------------------------------------------
auto FVulkanInstance::CreateWindowSurface(FWindowHandle hwnd) -> FWindowSurface {
    AssertRelease(not (_features & ERHIFeature::Headless)); // can't create window surface when headless
    Assert(nullptr != hwnd);

    const FCriticalScope scopeLock(&_barrier);
    Assert_NoAssume(VK_NULL_HANDLE != _vkInstance);

    VkSurfaceKHR surface = VK_NULL_HANDLE;

#ifdef PLATFORM_WINDOWS
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = static_cast<::HWND>(hwnd);
    createInfo.hinstance = static_cast<::HINSTANCE>(FCurrentProcess::Get().AppHandle());

    const VkResult result = vkCreateWin32SurfaceKHR(_vkInstance, &createInfo, &_vkAllocator, &surface);
    if (VK_SUCCESS != result)
        PPE_THROW_IT(FVulkanDeviceException("vkCreateWin32SurfaceKHR", result));

#else
#     error "unsupported platform"
#endif

    Assert_NoAssume(VK_NULL_HANDLE != surface);
    return FVulkanWindowSurface{ surface };
}
//----------------------------------------------------------------------------
void FVulkanInstance::DestroyWindowSurface(FWindowSurface surface) {
    AssertRelease(not (_features & ERHIFeature::Headless)); // can't create window surface when headless
    Assert(VK_NULL_HANDLE != surface);

    const FCriticalScope scopeLock(&_barrier);
    Assert_NoAssume(VK_NULL_HANDLE != _vkInstance);

    vkDestroySurfaceKHR(_vkInstance, surface, &_vkAllocator);
}
//----------------------------------------------------------------------------
FVulkanDevice* FVulkanInstance::CreateLogicalDevice(
    ERHIFeature features,
    FWindowSurface surfaceIFN ) {
    const FCriticalScope scopeLock(&_barrier);
    Assert_NoAssume(VK_NULL_HANDLE != _vkInstance);

    // setup needed device features
    VkPhysicalDeviceFeatures deviceFeatures{};
    if (features & ERHIFeature::Graphics) {
        deviceFeatures.multiDrawIndirect = true;
        deviceFeatures.samplerAnisotropy = true;
        deviceFeatures.textureCompressionBC = true;
    }
    if (features & ERHIFeature::Compute) {
        deviceFeatures.robustBufferAccess = true;
    }

    // iterate to find a matching device
    TArray<VkPhysicalDevice> vkPhysicalDevices;
    if (not PickPhysicalDevices(&vkPhysicalDevices, _vkInstance, deviceFeatures, surfaceIFN))
        PPE_THROW_IT(FVulkanInstanceException("PickPhysicalDevices", VK_ERROR_INITIALIZATION_FAILED));

    for (VkPhysicalDevice vkPhysicalDevice : vkPhysicalDevices) {
        VkPhysicalDeviceProperties vkProperties;
        vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vkProperties);

        // check device features
        if (features & ERHIFeature::Discrete) {
            if (vkProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                continue;
        }

        // check device extensions and features
        TArray<const char*> deviceExtensions;
        if (not FVulkanDeviceFunctions::SetupExtensions(&deviceExtensions, *this, vkPhysicalDevice, surfaceIFN != VK_NULL_HANDLE))
            continue;

        TArray<const char*> deviceLayers;
        if (not FVulkanDeviceFunctions::SetupLayers(&deviceLayers, *this, vkPhysicalDevice, features & ERHIFeature::Debugging))
            continue;

        // swap-chain if needed
        TArray<VkPresentModeKHR> presentModes;
        TArray<VkSurfaceFormatKHR> surfaceFormats;
        if (surfaceIFN != VK_NULL_HANDLE &&
            not SetupSwapChain(&presentModes, &surfaceFormats, vkPhysicalDevice, surfaceIFN))
            continue;

        // device queues
        TArray<VkDeviceQueueCreateInfo> deviceQueues;
        TArray<VkQueueFamilyProperties> queueFamilies;
        SetupQueueFamilies(&deviceQueues, &queueFamilies, vkPhysicalDevice);

        FVulkanDeviceQueueInfo // create queues by order of importance (*ORDER MATTERS*)
            queueGraphics,
            queuePresent,
            queueAsyncCompute,
            queueTransfer;

        if (features & ERHIFeature::Graphics) {
            if (not SetupDeviceQueue(
                &queueGraphics.FamilyIndex, &queueGraphics.FamilyQueueIndex,
                vkPhysicalDevice, deviceQueues, queueFamilies,
                VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT) )
                continue;
            if (not SetupDeviceQueue(
                &queuePresent.FamilyIndex, &queuePresent.FamilyQueueIndex,
                vkPhysicalDevice, deviceQueues, queueFamilies,
                VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, surfaceIFN) )
                continue;
        }

        if (features & ERHIFeature::Compute) {
            if (not SetupDeviceQueue(
                &queuePresent.FamilyIndex, &queuePresent.FamilyQueueIndex,
                vkPhysicalDevice, deviceQueues, queueFamilies,
                VK_QUEUE_COMPUTE_BIT) )
                continue;
        }

        if (not SetupDeviceQueue(
            &queueTransfer.FamilyIndex, &queueTransfer.FamilyQueueIndex,
            vkPhysicalDevice, deviceQueues, queueFamilies,
            VK_QUEUE_TRANSFER_BIT))
            continue;

        Remove_If(deviceQueues, [](const VkDeviceQueueCreateInfo& vkDeviceQueue) {
            return (vkDeviceQueue.queueCount == 0); // remove empty device queues
        });

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        createInfo.enabledExtensionCount = checked_cast<u32>(deviceExtensions.size());
        createInfo.ppEnabledLayerNames = deviceLayers.data();
        createInfo.enabledLayerCount = checked_cast<u32>(deviceLayers.size());
        createInfo.pQueueCreateInfos = deviceQueues.data();
        createInfo.queueCreateInfoCount = checked_cast<u32>(deviceQueues.size());

        VkDevice vkDevice = VK_NULL_HANDLE;
        const VkResult result = vkCreateDevice(vkPhysicalDevice, &createInfo, &_vkAllocator, &vkDevice);
        if (VK_SUCCESS != result)
            PPE_THROW_IT(FVulkanInstanceException("vkCreateDevice", result));

        FVulkanDevice* const pdevice = TRACKING_NEW(RHIInstance, FVulkanDevice) {
            *this,
            vkPhysicalDevice,
            IdentifyVendorId(vkProperties.vendorID),
            std::move(presentModes),
            std::move(surfaceFormats)
        };

        LOG(Vulkan, Info,
            L"successfuly created Vulkan device {0} (vk:{1}):"
            L"\t- Name: {2}\n",
            L"\t- Api: {3}\n"
            L"\t- DeviceID: {4}\n",
            L"\t- VendorID: {5} ({6})\n"
            L"\t- Driver version: {7}"
            L"\t- Window surface: {8}",
            Fmt::Pointer(pdevice),
            Fmt::Pointer(vkDevice),
            MakeCStringView(vkProperties.deviceName),
            vkProperties.apiVersion,
            vkProperties.deviceID,
            pdevice->VendorId(), vkProperties.vendorID,
            vkProperties.driverVersion,
            Fmt::Pointer(surfaceIFN) );

        if (not pdevice->SetupDevice(vkDevice, queueGraphics, queuePresent, queueAsyncCompute, queueTransfer)) {
            DestroyLogicalDevice(pdevice);
            PPE_THROW_IT(FVulkanDeviceException("SetupDevice", VK_ERROR_INITIALIZATION_FAILED));
        }

        return pdevice;
    }

    LOG(Vulkan, Error, L"failed to create a vulkan device");
    return nullptr;
}
//----------------------------------------------------------------------------
void FVulkanInstance::DestroyLogicalDevice(FVulkanDevice* pLogicalDevice) {
    Assert(pLogicalDevice);
    Assert(VK_NULL_HANDLE != pLogicalDevice);

    const FCriticalScope instanceLock(&_barrier);
    Assert_NoAssume(VK_NULL_HANDLE != _vkInstance);

    const VkDevice vkDevice = pLogicalDevice->_vkDevice;

    pLogicalDevice->vkDeviceWaitIdle(vkDevice);

    pLogicalDevice->TearDownDevice();

    pLogicalDevice->vkDestroyDevice(vkDevice, &_vkAllocator);

    FVulkanDeviceFunctions::Detach(pLogicalDevice);

    TRACKING_DELETE(RHIInstance, pLogicalDevice);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
