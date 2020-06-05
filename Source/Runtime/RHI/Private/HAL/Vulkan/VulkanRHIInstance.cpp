#include "stdafx.h"
#include "Diagnostic/CurrentProcess.h"

#ifdef RHI_VULKAN

#include "HAL/Vulkan/VulkanRHIInstance.h"

#include "HAL/Vulkan/VulkanError.h"
#include "HAL/Vulkan/VulkanRHIDevice.h"

#include "Allocator/Allocation.h"
#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"
#include "Container/BitMask.h"
#include "IO/ConstChar.h"
#include "Memory/MemoryDomain.h"
#include "Meta/Optional.h"

#define USE_PPE_VULKAN_DEBUGLAYER (USE_PPE_RHIDEBUG || USE_PPE_LOGGER)

#if USE_PPE_LOGGER
#    include "IO/FormatHelpers.h"
#endif

#include <mutex>

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI)
LOG_CATEGORY(, Vulkan)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FVulkanAllocator_ {
    VkAllocationCallbacks Callbacks{
        this,
        &FVulkanAllocator_::Allocate,
        &FVulkanAllocator_::Reallocate,
        &FVulkanAllocator_::Free,
#if USE_PPE_MEMORYDOMAINS
        &FVulkanAllocator_::InternalAllocateNotification,
        &FVulkanAllocator_::InternalFreeNotification
#else
        nullptr, nullptr
#endif
    };

#if USE_PPE_MEMORYDOMAINS
    static FMemoryTracking& ScopeTrackingData(VkSystemAllocationScope allocationScope) NOEXCEPT {
        switch (allocationScope) {
        case VK_SYSTEM_ALLOCATION_SCOPE_COMMAND: return MEMORYDOMAIN_TRACKING_DATA(RHICommand);
            break;
        case VK_SYSTEM_ALLOCATION_SCOPE_OBJECT: return MEMORYDOMAIN_TRACKING_DATA(RHIObject);
            break;
        case VK_SYSTEM_ALLOCATION_SCOPE_CACHE: return MEMORYDOMAIN_TRACKING_DATA(RHICache);
            break;
        case VK_SYSTEM_ALLOCATION_SCOPE_DEVICE: return MEMORYDOMAIN_TRACKING_DATA(RHIDevice);
            break;
        case VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE: return MEMORYDOMAIN_TRACKING_DATA(RHIInstance);
            break;
        default:
            AssertNotImplemented();
        }
    }

#endif
    static void* VKAPI_PTR Allocate(void* pUserData, size_t size, size_t alignment,
                                    VkSystemAllocationScope allocationScope) {
        UNUSED(pUserData);
#if USE_PPE_MEMORYDOMAINS
        return PPE::tracking_aligned_malloc(ScopeTrackingData(allocationScope), size, alignment);
#else
        UNUSED(alignment);
        UNUSED(allocationScope);
        void* const p = PPE::malloc(size);
        Assert_NoAssume(Meta::IsAligned(alignment, p));
        return p;
#endif
    }

    static void* VKAPI_PTR Reallocate(void* pUserData, void* pOriginal, size_t size, size_t alignment,
                                      VkSystemAllocationScope allocationScope) {
        UNUSED(pUserData);
#if USE_PPE_MEMORYDOMAINS
        return PPE::tracking_aligned_realloc(ScopeTrackingData(allocationScope), pOriginal, size, alignment);
#else
        UNUSED(alignment);
        UNUSED(allocationScope);
        void* const p = PPE::realloc(pOriginal, size);
        Assert_NoAssume(Meta::IsAligned(alignment, p));
        return p;
#endif
    }

    static void VKAPI_PTR Free(void* pUserData, void* pMemory) {
        UNUSED(pUserData);
#if USE_PPE_MEMORYDOMAINS
        PPE::tracking_free(pMemory);
#else
        PPE::free(pMemory);
#endif
    }

#if USE_PPE_MEMORYDOMAINS
    // Notified when Vulkan make an internal allocation without our own allocator,
    // this is intended only for tracking purpose (and shouldn't be a common need).
    static void VKAPI_PTR InternalAllocateNotification(void*, size_t size,
                                                       VkInternalAllocationType allocationType,
                                                       VkSystemAllocationScope allocationScope) {
        UNUSED(allocationType);
        UNUSED(allocationScope);
        MEMORYDOMAIN_TRACKING_DATA(RHIInternal).Allocate(size, size);
    }

    static void VKAPI_PTR InternalFreeNotification(void*, size_t size,
                                                   VkInternalAllocationType allocationType,
                                                   VkSystemAllocationScope allocationScope) {
        UNUSED(allocationType);
        UNUSED(allocationScope);
        MEMORYDOMAIN_TRACKING_DATA(RHIInternal).Deallocate(size, size);
    }
#endif
};
//----------------------------------------------------------------------------
struct FVulkanProperties_ {
    using FNameList = VECTORINSITU(RHIInternal, const char*, 8);

    FNameList Extensions;
    FNameList Layers;

    FVulkanProperties_() = default;

    void Reset() {
        Extensions.clear();
        Layers.clear();
    }

    NODISCARD bool SetupInstance(bool headless, bool debug) NOEXCEPT {
        Reset();

        if (not headless) {
            Append(Extensions, {
                VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(_WIN32)
                VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
                VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#elif defined(_DIRECT2DISPLAY)
                VK_KHR_DISPLAY_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
                VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XCB_KHR)
                VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_IOS_MVK)
                VK_MVK_IOS_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
                VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
#endif
            });
        }

#if USE_PPE_RHIDEBUG
        if (debug) {
            Append(Extensions, {
                "VK_EXT_debug_report",
#if USE_PPE_LOGGER
                "VK_EXT_debug_utils"
#endif
            });
        }
#endif

        bool validated = true;
        validated &= CheckNames_(
            "vkEnumerateInstanceExtensionProperties",
            Extensions.MakeConstView(),
            &VkExtensionProperties::extensionName,
            [](u32* pNumExtensions, VkExtensionProperties* pExtensions) {
                return vkEnumerateInstanceExtensionProperties(nullptr, pNumExtensions, pExtensions);
            });

        SetupValidationLayers_(debug);

        validated &= CheckNames_(
            "vkEnumerateInstanceLayerProperties",
            Layers.MakeConstView(),
            &VkLayerProperties::layerName,
            [](u32* pNumLayers, VkLayerProperties* pLayers) {
                return vkEnumerateInstanceLayerProperties(pNumLayers, pLayers);
            });

        return validated;
    }

    NODISCARD bool SetupDevice(VkPhysicalDevice device, EVulkanPhysicalDeviceFlags flags, bool debug) {
        Reset();

        if (EVulkanPhysicalDeviceFlags::Swapchain ^ flags)
            Extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        bool validated = true;

        validated &= CheckNames_(
            "vkEnumerateDeviceExtensionProperties",
            Extensions.MakeConstView(),
            &VkExtensionProperties::extensionName,
            [device](u32* pNumExtensions, VkExtensionProperties* pExtensions) {
                return vkEnumerateDeviceExtensionProperties(device, nullptr, pNumExtensions, pExtensions);
            });

        SetupValidationLayers_(debug);

        validated &= CheckNames_(
            "vkEnumerateDeviceLayerProperties",
            Layers.MakeConstView(),
            &VkLayerProperties::layerName,
            [device](u32* pNumLayers, VkLayerProperties* pLayers) {
                return vkEnumerateDeviceLayerProperties(device, pNumLayers, pLayers);
            });

        return validated;
    }

private:
    void SetupValidationLayers_(bool debug) NOEXCEPT {
#if USE_PPE_RHIDEBUG
        if (debug)
            Layers.assign({
                "VK_LAYER_KHRONOS_validation",
                "VK_LAYER_LUNARG_standard_validation",
            });
        else
            Layers.clear();
#endif
    }

    using FVulkanStaticName = char[VK_MAX_EXTENSION_NAME_SIZE];
    template <typename T, typename _Enumerate>
    static NODISCARD bool CheckNames_(
        const char* name,
        const TMemoryView<const char* const>& neededNames,
        FVulkanStaticName T::* member,
        _Enumerate&& each ) {
        u32 numElements = 0;

        VkResult result;
        result = each(&numElements, nullptr);
        if (VK_SUCCESS != result)
            PPE_THROW_IT(FVulkanDeviceException(name, result));

        STACKLOCAL_POD_ARRAY(T, elements, numElements);
        result = each(&numElements, elements.data());
        if (VK_SUCCESS != result)
            PPE_THROW_IT(FVulkanDeviceException(name, result));

        LOG(Vulkan, Verbose, L"{0}: found {2} results = [{1}]",
            MakeCStringView(name),
            Fmt::Join(
                MakeOutputIterable(elements.begin(), elements.end(),
                    [member](const T& elt) NOEXCEPT {
                        return MakeCStringView(elt.*member);
                    }),
                MakeCStringView(L", ")),
            elements.size() );

        for (const char* needed : neededNames) {
            bool found = false;
            for (const T& elt : elements) {
                if (FConstChar{ needed }.EqualsI(elt.*member)) {
                    found = true;
                    break;
                }
            }

            if (not found) {
                LOG(Vulkan, Error, L"{0}: failed to find <{1}> !",
                    MakeCStringView(name), MakeCStringView(needed) );
                return false;
            }
        }

        LOG(Vulkan, Info, L"{0}: selected {2} results = [{1}]",
            MakeCStringView(name),
            Fmt::Join(
                MakeOutputIterable(neededNames.begin(), neededNames.end(),
                    [member](const char* cstr) NOEXCEPT {
                        return MakeCStringView(cstr);
                    }),
                MakeCStringView(L", ")),
            neededNames.size() );

        return true;
    }
};
//----------------------------------------------------------------------------
#if USE_PPE_VULKAN_DEBUGLAYER
struct FVulkanDebugLayer_ {
    VkDebugUtilsMessengerEXT DebugMessenger;

    void Start(VkInstance instance, VkAllocationCallbacks& allocationCallbacks) {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = &DebugCallback;
        createInfo.pUserData = this;

        VkResult result = CreateDebugUtilsMessengerEXT(instance, &createInfo, &allocationCallbacks, &DebugMessenger);
        if (VK_SUCCESS != result)
            PPE_THROW_IT(FVulkanDeviceException("CreateDebugUtilsMessengerEXT", result));
    }

    void Shutdown(VkInstance instance, VkAllocationCallbacks& allocationCallbacks) {
        DestroyDebugUtilsMessengerEXT(instance, DebugMessenger, &allocationCallbacks);
    }

    static VkBool32 VKAPI_PTR DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
#if USE_PPE_LOGGER
        UNUSED(pUserData); // FVulkanDebugLayer_*

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

        auto fmtCallbackData = [pCallbackData](FWTextWriter& oss){
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
                    MakeCStringView(obj.pObjectName) );
            }
        };

        FLogger::Log(
            LOG_CATEGORY_GET(Vulkan),
            level,
            FLogger::FSiteInfo::Make(WIDESTRING(__FILE__),__LINE__),
            L"{3}<{1}> {0} ({2}){4}",
            MakeCStringView(pCallbackData->pMessage),
            MakeCStringView(pCallbackData->pMessageIdName),
            pCallbackData->messageIdNumber,
            Fmt::Formator<wchar_t>(fmtMessageType),
            Fmt::Formator<wchar_t>(fmtCallbackData) );

#else
        UNUSED(messageSeverity);
        UNUSED(messageType);
        UNUSED(pCallbackData);
        UNUSED(pUserData);
#endif
        return VK_FALSE;
    }

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                 const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                              const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }
};
#endif //!USE_PPE_VULKAN_DEBUGLAYER
//----------------------------------------------------------------------------
struct FVulkanInstanceData_ {

    static FVulkanInstanceData_& Get() {
        ONE_TIME_DEFAULT_INITIALIZE(FVulkanInstanceData_, GInstance);
        return GInstance;
    }

    std::mutex Barrier;
    VkInstance Instance{VK_NULL_HANDLE};
    FVulkanAllocator_ Allocator;
    FVulkanProperties_ Properties;
#if USE_PPE_VULKAN_DEBUGLAYER
    FVulkanDebugLayer_ DebugLayer;
#endif

    void Start(VkInstance instance) {
        Assert(VK_NULL_HANDLE != instance);
        Assert_NoAssume(VK_NULL_HANDLE == Instance);
        Instance = instance;
#if USE_PPE_VULKAN_DEBUGLAYER
        DebugLayer.Start(instance, Allocator.Callbacks);
#endif
    }

    void Shutdown() {
        Assert_NoAssume(VK_NULL_HANDLE != Instance);
#if USE_PPE_VULKAN_DEBUGLAYER
        DebugLayer.Shutdown(Instance, Allocator.Callbacks);
#endif
        Instance = VK_NULL_HANDLE;
    }
};
//----------------------------------------------------------------------------
struct FVulkanQueueFamilies_ {
    struct FQueueFamily {
        u32 FamilyIndex;
        u32 NumQueues;
        VkFlags QueueFlags;
        u32 FirstQueueAvailable;
    };

    struct FDeviceQueue {
        u32 FamilyIndex;
        u32 QueueIndex;
    };

    STATIC_CONST_INTEGRAL(size_t, InSitu, 4);

    VkPhysicalDevice Device;
    VECTORINSITU(RHIInternal, FQueueFamily, InSitu) Families;
    VECTORINSITU(RHIInternal, FDeviceQueue, InSitu) Queues;
    VECTORINSITU(RHIInternal, VkDeviceQueueCreateInfo, InSitu) CreateInfos;

    NODISCARD bool AddIFP(u32* pQueueIndex, VkFlags queueFlags, VkSurfaceKHR surfaceIFN = VK_NULL_HANDLE) NOEXCEPT {
        Assert(pQueueIndex);

        FQueueFamily* candidate = nullptr;
        for (FQueueFamily& family : Families) {
            if (queueFlags != (queueFlags & family.QueueFlags))
                continue;

            if (VK_NULL_HANDLE != surfaceIFN) {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(Device, family.FamilyIndex, surfaceIFN, &presentSupport);

                if (not presentSupport)
                    continue;
            }

            candidate = &family;
            if (0 == family.FirstQueueAvailable)
                break; // try to spread queues across all families
        }

        if (not candidate)
            return false;

        Assert(candidate->FirstQueueAvailable < candidate->NumQueues);
        *pQueueIndex = checked_cast<u32>(Queues.size());
        Queues.emplace_back(candidate->FamilyIndex, candidate->FirstQueueAvailable++);

        return true;
    }

    VkQueue Get(VkDevice device, u32 queueIndex) const NOEXCEPT {
        const FDeviceQueue& queue = Queues[queueIndex];

        VkQueue deviceQueue = VK_NULL_HANDLE;
        vkGetDeviceQueue(device, queue.FamilyIndex, queue.QueueIndex, &deviceQueue);
        Assert(VK_NULL_HANDLE != deviceQueue);

        return deviceQueue;
    }

    static CONSTEXPR const float GDefaultQueuePriorities[] = {
        1.00f,0.95f,0.90f,0.85f,0.80f,0.75f,0.70f,
        0.65f,0.60f,0.55f,0.50f,0.45f,0.40f,0.35f,
        0.30f,0.25f,0.20f,0.15f,0.10f,0.05f,0.00f
    };

    void SetupDevice(VkPhysicalDevice device) NOEXCEPT {
        Device = device;

        u32 numQueueFamilies = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(Device, &numQueueFamilies, nullptr);

        STACKLOCAL_POD_ARRAY(VkQueueFamilyProperties, queueFamilies, numQueueFamilies);
        vkGetPhysicalDeviceQueueFamilyProperties(Device, &numQueueFamilies, queueFamilies.data());

        // sort queue families from least to most number of flags enabled
        std::sort(queueFamilies.begin(), queueFamilies.end(),
            [](const VkQueueFamilyProperties& a, const VkQueueFamilyProperties& b) NOEXCEPT -> bool {
                using bmask_t = TBitMask<VkFlags>;
                return (bmask_t{ a.queueFlags }.Count() < bmask_t{ b.queueFlags }.Count());
            });

        Queues.clear();
        Families.resize_Uninitialized(numQueueFamilies);

        forrange(i, 0, numQueueFamilies) {
            FQueueFamily& dst = Families[i];
            const VkQueueFamilyProperties& src = queueFamilies[i];

            dst.FamilyIndex = i;
            dst.NumQueues = src.queueCount;
            dst.QueueFlags = src.queueFlags;
            dst.FirstQueueAvailable = 0;
        }
    }

    void PrepareCreateInfos() NOEXCEPT {
        const auto defaultPriorities = MakeView(GDefaultQueuePriorities);

        CreateInfos.clear();

        for (const FQueueFamily& family : Families) {
            if (family.FirstQueueAvailable > 0) {
                Assert_NoAssume(family.FirstQueueAvailable <= defaultPriorities.size());

                VkDeviceQueueCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                createInfo.queueFamilyIndex = family.FamilyIndex;
                createInfo.queueCount = family.FirstQueueAvailable;
                createInfo.pQueuePriorities = defaultPriorities.data();
                createInfo.flags = 0; // only used for VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT
                createInfo.pNext = VK_NULL_HANDLE; // only used for VkDeviceQueueGlobalPriorityCreateInfoEXT

                CreateInfos.push_back(createInfo);
            }
        }
    }
};
//----------------------------------------------------------------------------
struct FVulkanQueueIndices_ {
    Meta::TOptional<u32> Graphics;
    Meta::TOptional<u32> Present;
    Meta::TOptional<u32> AsyncCompute;
    Meta::TOptional<u32> Transfer;
    Meta::TOptional<u32> ReadBack;

    NODISCARD bool SetupDevice(FVulkanQueueFamilies_& queueFamilies, VkSurfaceKHR surfaceIFN = VK_NULL_HANDLE) NOEXCEPT {
        bool result = true;
        result &= SetupQueue(&Graphics, queueFamilies, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
        result &= SetupQueue(&Present, queueFamilies, VK_QUEUE_TRANSFER_BIT, surfaceIFN);
        result &= SetupQueue(&AsyncCompute, queueFamilies, VK_QUEUE_COMPUTE_BIT);
        result &= SetupQueue(&Transfer, queueFamilies, VK_QUEUE_TRANSFER_BIT);
        result &= SetupQueue(&ReadBack, queueFamilies, VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
        return result;
    }

    static bool SetupQueue(Meta::TOptional<u32>* pQueueIndex, FVulkanQueueFamilies_& queueFamilies, VkFlags queueFlags, VkSurfaceKHR surfaceIFN = VK_NULL_HANDLE) NOEXCEPT {
        Assert(pQueueIndex);
        u32 result;
        if (queueFamilies.AddIFP(&result, queueFlags, surfaceIFN)) {
            *pQueueIndex = result;
            return true;
        }
        else {
            pQueueIndex->reset();
            return false;
        }
    }
};
//----------------------------------------------------------------------------
struct FVulkanDeviceFeatures_ {
    using feature_t = VkBool32(VkPhysicalDeviceFeatures::*);
    static CONSTEXPR const feature_t GNeededForGraphics[] = {
        &VkPhysicalDeviceFeatures::multiDrawIndirect,
        &VkPhysicalDeviceFeatures::geometryShader,
        &VkPhysicalDeviceFeatures::tessellationShader,
        &VkPhysicalDeviceFeatures::samplerAnisotropy,
        &VkPhysicalDeviceFeatures::sampleRateShading,
        &VkPhysicalDeviceFeatures::textureCompressionBC,
    };

    static bool CheckGraphics(const VkPhysicalDeviceFeatures& features) NOEXCEPT {
        for (feature_t needed : GNeededForGraphics) {
            if (features.*needed != VK_TRUE)
                return false;
        }
        return true;
    }
    static void SetupGraphics(VkPhysicalDeviceFeatures* pFeatures) NOEXCEPT {
        for (feature_t needed : GNeededForGraphics) {
            pFeatures->*needed = VK_TRUE;
        }
    }
};
//----------------------------------------------------------------------------
static VkPhysicalDevice PickPhysicalDevice_(
    FVulkanProperties_* pDeviceProperties,
    FVulkanQueueFamilies_* pQueueFamilies,
    FVulkanQueueIndices_* pQueueIndices,
    VkInstance instance,
    EVulkanPhysicalDeviceFlags deviceFlags,
    VkSurfaceKHR surfaceIFN ) {
    Assert(pDeviceProperties);
    Assert(pQueueFamilies);
    Assert(pQueueIndices);

    AssertRelease(not (deviceFlags ^ EVulkanPhysicalDeviceFlags::Swapchain) || not FVulkanInstance::GHeadless);
    AssertRelease(not FVulkanInstance::GHeadless || VK_NULL_HANDLE == surfaceIFN); // can't create window surface when headless

    u32 numDevices = 0;

    VkResult result;
    result = vkEnumeratePhysicalDevices(instance, &numDevices, nullptr);
    if  (VK_SUCCESS != result)
        PPE_THROW_IT(FVulkanDeviceException("vkEnumeratePhysicalDevices", result));

    STACKLOCAL_POD_ARRAY(VkPhysicalDevice, devices, numDevices);
    result = vkEnumeratePhysicalDevices(instance, &numDevices, devices.data());
    if  (VK_SUCCESS != result)
        PPE_THROW_IT(FVulkanDeviceException("vkEnumeratePhysicalDevices", result));

    int bestScore = 0;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDevice lastDeviceValidated = VK_NULL_HANDLE;
    auto setupDeviceIFP = [=, &lastDeviceValidated](VkPhysicalDevice device) NOEXCEPT -> bool {
        if (device == lastDeviceValidated)
            return true;

        if (not pDeviceProperties->SetupDevice(device, deviceFlags, FVulkanInstance::GDebugEnabled))
            return false;

        pQueueFamilies->SetupDevice(device);

        if (not pQueueIndices->SetupDevice(*pQueueFamilies, surfaceIFN))
            return false;

        lastDeviceValidated = device;
        return true;
    };

    for (const VkPhysicalDevice device : devices) {
        Assert(VK_NULL_HANDLE != device);

        if (not setupDeviceIFP(device))
            continue;

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);

        int score = 0;

        if (deviceFlags ^ EVulkanPhysicalDeviceFlags::Graphics) {
            if (not FVulkanDeviceFeatures_::CheckGraphics(features))
                continue;
        }

        if (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == properties.deviceType)
            score += 1000;
        else if (deviceFlags ^ EVulkanPhysicalDeviceFlags::Discrete)
            continue;

        score += properties.limits.maxImageDimension2D;
        score += properties.limits.maxImageDimension3D;
        score += properties.limits.maxImageDimensionCube;

        if (score > bestScore) {
            bestScore = score;
            physicalDevice = device;
        }
    }

    if (VK_NULL_HANDLE != physicalDevice && setupDeviceIFP(physicalDevice))
        return physicalDevice;
    else
        return VK_NULL_HANDLE;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FVulkanInstance::Start() {
    FVulkanInstanceData_& vk = FVulkanInstanceData_::Get();
    const Meta::FUniqueLock scopeLock(vk.Barrier);

    VerifyRelease(vk.Properties.SetupInstance(GHeadless, GDebugEnabled));

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

    createInfo.enabledExtensionCount = checked_cast<u32>(vk.Properties.Extensions.size());
    createInfo.ppEnabledExtensionNames = vk.Properties.Extensions.data();

    createInfo.enabledLayerCount = checked_cast<u32>(vk.Properties.Layers.size());
    createInfo.ppEnabledLayerNames = vk.Properties.Layers.data();

    VkInstance instance;
    VkResult result = vkCreateInstance(&createInfo, &vk.Allocator.Callbacks, &instance);
    if (VK_SUCCESS != result)
        PPE_THROW_IT(FVulkanDeviceException("vkCreateInstance", result));

    vk.Start(instance);
}
//----------------------------------------------------------------------------
void FVulkanInstance::Shutdown() {
    FVulkanInstanceData_& vk = FVulkanInstanceData_::Get();
    const Meta::FUniqueLock scopeLock(vk.Barrier);

    VkInstance instance = vk.Instance;

    vk.Shutdown();

    vkDestroyInstance(instance, &vk.Allocator.Callbacks);
}
//----------------------------------------------------------------------------
auto FVulkanInstance::CreateWindowSurface(FWindowHandle hwnd) -> FWindowSurface {
    AssertRelease(not GHeadless); // can't create window surface when headless
    Assert(nullptr != *hwnd);

    FVulkanInstanceData_& vk = FVulkanInstanceData_::Get();
    const Meta::FUniqueLock scopeLock(vk.Barrier);

    VkSurfaceKHR surface;

#ifdef PLATFORM_WINDOWS
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = static_cast<::HWND>(*hwnd);
    createInfo.hinstance = static_cast<::HINSTANCE>(FCurrentProcess::Get().AppHandle());

    const VkResult result = vkCreateWin32SurfaceKHR(vk.Instance, &createInfo, &vk.Allocator.Callbacks, &surface);
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
    AssertRelease(not GHeadless); // can't create window surface when headless
    Assert(nullptr != *surface);

    FVulkanInstanceData_& vk = FVulkanInstanceData_::Get();
    const Meta::FUniqueLock scopeLock(vk.Barrier);

    vkDestroySurfaceKHR(vk.Instance, static_cast<VkSurfaceKHR>(*surface), &vk.Allocator.Callbacks);
}
//----------------------------------------------------------------------------
FVulkanDevice* FVulkanInstance::CreateLogicalDevice(
    EPhysicalDeviceFlags deviceFlags,
    FWindowSurface surfaceIFN ) {

    FVulkanInstanceData_& vk = FVulkanInstanceData_::Get();
    const Meta::FUniqueLock scopeLock(vk.Barrier);

    // iterate to find a matching device
    FVulkanProperties_ deviceProperties;
    FVulkanQueueIndices_ queueIndices;
    FVulkanQueueFamilies_ queueFamilies;

    const VkPhysicalDevice physicalDevice = PickPhysicalDevice_(
        &deviceProperties, &queueFamilies, &queueIndices,
        vk.Instance, deviceFlags,
        static_cast<VkSurfaceKHR>(*surfaceIFN) );
    CLOG(VK_NULL_HANDLE == physicalDevice, Vulkan, Fatal, L"failed to pick a compliant physical device");

    // setup queue and features
    queueFamilies.PrepareCreateInfos();

    VkPhysicalDeviceFeatures features{};
    if (EPhysicalDeviceFlags::Graphics ^ deviceFlags)
        FVulkanDeviceFeatures_::SetupGraphics(&features);

    // main device info
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pEnabledFeatures = &features;

    createInfo.pQueueCreateInfos = queueFamilies.CreateInfos.data();
    createInfo.queueCreateInfoCount = checked_cast<u32>(queueFamilies.CreateInfos.size());

    createInfo.enabledExtensionCount = checked_cast<u32>(deviceProperties.Extensions.size());
    createInfo.ppEnabledExtensionNames = deviceProperties.Extensions.data();

    createInfo.enabledLayerCount = checked_cast<u32>(deviceProperties.Layers.size());
    createInfo.ppEnabledLayerNames = deviceProperties.Layers.data();;

    VkDevice logicalDevice = VK_NULL_HANDLE;
    const VkResult result = vkCreateDevice(
        physicalDevice,
        &createInfo,
        &vk.Allocator.Callbacks,
        &logicalDevice );
    if (VK_SUCCESS != result)
        PPE_THROW_IT(FVulkanInstanceException("vkCreateDevice", result));

    return TRACKING_NEW(RHIInstance, FVulkanDevice) {
        FVulkanDeviceHandle{ logicalDevice },
        FVulkanQueue{ queueFamilies.Get(logicalDevice, *queueIndices.Graphics) },
        FVulkanQueue{ queueFamilies.Get(logicalDevice, *queueIndices.Present) },
        FVulkanQueue{ queueFamilies.Get(logicalDevice, *queueIndices.AsyncCompute) },
        FVulkanQueue{ queueFamilies.Get(logicalDevice, *queueIndices.Transfer) },
        FVulkanQueue{ queueFamilies.Get(logicalDevice, *queueIndices.ReadBack) }
    };
}
//----------------------------------------------------------------------------
void FVulkanInstance::DestroyLogicalDevice(FVulkanDevice* pLogicalDevice) {
    Assert(pLogicalDevice);
    Assert(VK_NULL_HANDLE != pLogicalDevice);

    FVulkanInstanceData_& vk = FVulkanInstanceData_::Get();
    const Meta::FUniqueLock scopeLock(vk.Barrier);

    vkDestroyDevice(
        static_cast<VkDevice>(*pLogicalDevice->_deviceHandle),
        &vk.Allocator.Callbacks );

#if USE_PPE_ASSERT // see ~FVulkanDevice
    *pLogicalDevice->_deviceHandle = nullptr;
    *pLogicalDevice->_graphicsQueue = nullptr;
    *pLogicalDevice->_presentQueue = nullptr;
    *pLogicalDevice->_asyncComputeQueue = nullptr;
    *pLogicalDevice->_transferQueue = nullptr;
    *pLogicalDevice->_readBackQueue = nullptr;
#endif

    TRACKING_DELETE(RHIInstance, pLogicalDevice);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
