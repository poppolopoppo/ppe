// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Vulkan/Instance/VulkanInstance.h"

#include "Vulkan/VulkanIncludes.h"
#include "Vulkan/Instance/VulkanDevice.h"

#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "Meta/Utility.h"
#include "Modular/ModularDomain.h"

#if USE_PPE_LOGGER
#   include "Vulkan/Common/VulkanEnumToString.h"
#   include "IO/String.h"
#endif

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// Allocator
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
static FMemoryTracking& VulkanTrackingData_(VkSystemAllocationScope vkScope) {
    switch (vkScope) {
    case VK_SYSTEM_ALLOCATION_SCOPE_COMMAND: return MEMORYDOMAIN_TRACKING_DATA(VkCommand);
    case VK_SYSTEM_ALLOCATION_SCOPE_OBJECT: return MEMORYDOMAIN_TRACKING_DATA(VkObject);
    case VK_SYSTEM_ALLOCATION_SCOPE_CACHE: return MEMORYDOMAIN_TRACKING_DATA(VkCache);
    case VK_SYSTEM_ALLOCATION_SCOPE_DEVICE: return MEMORYDOMAIN_TRACKING_DATA(VkDevice);
    case VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE: return MEMORYDOMAIN_TRACKING_DATA(VkInstance);
    default:
        AssertNotImplemented();
    }
}
#endif
//----------------------------------------------------------------------------
static void* VulkanMalloc_(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
    Unused(pUserData);
#if USE_PPE_MEMORYDOMAINS
    return PPE::tracking_aligned_malloc(VulkanTrackingData_(allocationScope), size, alignment);
#else
    Unused(alignment);
    Unused(allocationScope);
    Assert_NoAssume(alignment < ALLOCATION_BOUNDARY);
    void* const p = PPE::malloc(size);
    Assert_NoAssume(Meta::IsAlignedPow2(alignment, p));
    return p;
#endif
}
//----------------------------------------------------------------------------
static void* VulkanRealloc_(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
    Unused(pUserData);
#if USE_PPE_MEMORYDOMAINS
    return PPE::tracking_aligned_realloc(VulkanTrackingData_(allocationScope), pOriginal, size, alignment);
#else
    Unused(alignment);
    Unused(allocationScope);
    Assert_NoAssume(alignment < ALLOCATION_BOUNDARY);
    void* const p = PPE::realloc(pOriginal, size);
    Assert_NoAssume(Meta::IsAlignedPow2(alignment, p));
    return p;
#endif
}
//----------------------------------------------------------------------------
static void VulkanFree_(void* pUserData, void* pMemory) {
    Unused(pUserData);
#if USE_PPE_MEMORYDOMAINS
    PPE::tracking_free(pMemory);
#else
    PPE::free(pMemory);
#endif
}
//----------------------------------------------------------------------------
static VkAllocationCallbacks MakeVulkanAllocator_(FVulkanInstance* pInstance) {
    Assert(pInstance);

    PFN_vkInternalAllocationNotification pfnInternalAllocation = nullptr;
    PFN_vkInternalFreeNotification pfnInternalFree = nullptr;
#if USE_PPE_MEMORYDOMAINS
    // Notified when Vulkan make an internal allocation without our own allocator,
    // this is intended only for tracking purpose (and shouldn't be a common need).
    pfnInternalAllocation = [](
        void*, size_t size,
        VkInternalAllocationType allocationType,
        VkSystemAllocationScope allocationScope) {
        Unused(allocationType);
        Unused(allocationScope);
        MEMORYDOMAIN_TRACKING_DATA(RHIInternal).Allocate(size, size);
    };
    pfnInternalFree = [](void*, size_t size,
        VkInternalAllocationType allocationType,
        VkSystemAllocationScope allocationScope) {
        Unused(allocationType);
        Unused(allocationScope);
        MEMORYDOMAIN_TRACKING_DATA(RHIInternal).Deallocate(size, size);
    };
#endif

    return VkAllocationCallbacks{
        pInstance,
        &VulkanMalloc_, &VulkanRealloc_, &VulkanFree_,
        pfnInternalAllocation, pfnInternalFree
    };
}
//----------------------------------------------------------------------------
// Global API
//----------------------------------------------------------------------------
bool LoadVulkanGlobalAPI_(FDynamicLibrary* pLib, FVulkanExportedAPI* pExported, FVulkanGlobalAPI* pGlobal) {
    const wchar_t* const vulkanLibPaths[] = {
#if defined(VK_USE_PLATFORM_WIN32_KHR) && VK_USE_PLATFORM_WIN32_KHR
        L"vulkan-1.dll"
#elif defined(VK_USE_PLATFORM_XCB_KHR) or defined(VK_USE_PLATFORM_XLIB_KHR)
        L"libvulkan.so", L"libvulkan.so.1"
#else
#       error "unknown platform"
#endif
    };

    for (const wchar_t* wpath : vulkanLibPaths) {
        if (pLib->AttachOrLoad(wpath)) {
            LOG(RHI, Verbose, L"successfuly loaded vulkan library '{0}'", pLib->ModuleName());

            pExported->vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(pLib->FunctionAddr("vkGetInstanceProcAddr"));
            if (pExported->vkGetInstanceProcAddr) {
                const char* err = pGlobal->attach_return_error(pExported);
                if (nullptr == err)
                    return true;

                LOG(RHI, Error, L"failed to load vulkan global API: {0}", MakeCStringView(err));
            }
            else {
                LOG(RHI, Error, L"failed to bind exported vulkan function <vkGetInstanceProcAddr>, abort library");
            }

            pLib->Unload();
        }
        else {
            LOG(RHI, Warning, L"could not load vulkan library from '{0}'", MakeCStringView(wpath));
        }
    }

    LOG(RHI, Error, L"failed to load vulkan library");
    return false;
}
//----------------------------------------------------------------------------
// Instance API
//----------------------------------------------------------------------------
bool ValidateInstanceLayers_(VECTOR(RHIInstance, FConstChar)* pLayerNames, const FVulkanGlobalAPI& api) {
    u32 numLayers = 0;
    VK_CHECK( api.vkEnumerateInstanceLayerProperties(&numLayers, nullptr) );

    if (0 == numLayers) {
        CLOG(numLayers != pLayerNames->size(), RHI, Warning, L"vulkan layer are not supported and will be ignored");
        pLayerNames->clear();
        return true;
    }

    STACKLOCAL_POD_ARRAY(VkLayerProperties, layerProperties, numLayers);
    VK_CHECK( api.vkEnumerateInstanceLayerProperties(&numLayers, layerProperties.data()) );

    LOG(RHI, Info, L"available vulkan layers: [\n    {0} ]",
        Fmt::Join(MakeIterable(layerProperties).Map([](const VkLayerProperties& x) {
                return FConstChar(x.layerName);
            }), MakeStringView(L",\n    ")));

    for (auto it = pLayerNames->begin(); it != pLayerNames->end(); ) {
        const auto found = layerProperties.FindIf([name{*it}](const VkLayerProperties& props) {
            return FConstChar(props.layerName).Equals(name);
        });

        if (layerProperties.end() == found) {
            LOG(RHI, Warning, L"vulkan layer '{0}' is not supported and will be removed", it->MakeView());
            it = pLayerNames->erase(it);
        }
        else {
            LOG(RHI, Verbose, L"vulkan layer '{0}' is supported and will be initialized", it->MakeView());
            ++it;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
bool ValidateInstanceExtensions_(
    VECTOR(RHIInstance, FConstChar)* pExtensionNames,
    const FVulkanGlobalAPI& api,
    FVulkanInstanceExtensionSet& requiredInstanceExts,
    FVulkanInstanceExtensionSet& optionalInstanceExts,
    const FVulkanDeviceExtensionSet& requiredDeviceExts,
    const FVulkanDeviceExtensionSet& optionalDeviceExts) {
    const FVulkanInstanceExtensionSet userInstanceExts{ requiredInstanceExts };

    // expand device requirements:
    requiredInstanceExts |= vk::instance_extensions_require(vk::device_extensions_require(requiredDeviceExts));
    optionalInstanceExts |= vk::instance_extensions_require(vk::device_extensions_require(optionalDeviceExts));

    // expand instance requirements:
    requiredInstanceExts = vk::instance_extensions_require(requiredInstanceExts);
    optionalInstanceExts = vk::instance_extensions_require(optionalInstanceExts);

    pExtensionNames->reserve(
        requiredInstanceExts.Count() +
        optionalInstanceExts.Count() );

    FVulkanInstanceExtensionSet it{ requiredInstanceExts | optionalInstanceExts };
    for (;;) {
        const auto bit = it.PopFront();
        if (not bit) break;
        const auto ext = static_cast<EVulkanInstanceExtension>(bit - 1);

        FConstChar name{ vk::instance_extension_name(ext) };
        AssertRelease(name);

        CLOG(not (userInstanceExts & ext), RHI, Warning, L"add vulkan instance extension '{0}' required by user input", name.MakeView());

        pExtensionNames->push_back_AssumeNoGrow(std::move(name));
    }

    u32 numExtensions = 0;
    VK_CHECK( api.vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, nullptr) );

    if (0 == numExtensions) {
        CLOG(numExtensions != pExtensionNames->size(), RHI, Warning, L"vulkan instance extensions are not supported and will be ignored");
        pExtensionNames->clear();
        return true;
    }

    STACKLOCAL_POD_ARRAY(VkExtensionProperties, extProperties, numExtensions);
    VK_CHECK( api.vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, extProperties.data()) );

    for (auto jt = pExtensionNames->begin(); jt != pExtensionNames->end(); ) {
        const auto found = extProperties.FindIf([name{*jt}](const VkExtensionProperties& props) {
            return FConstChar(props.extensionName).Equals(name);
        });

        if (extProperties.end() == found) {
            const EVulkanInstanceExtension ext = vk::instance_extension_from(*jt);
            if (userInstanceExts & ext) {
                LOG(RHI, Error, L"required vulkan instance extension '{0}' is not supported and will be removed!", jt->MakeView());
                requiredInstanceExts -= ext;
            }
            else {
                LOG(RHI, Warning, L"optional vulkan instance extension '{0}' is not supported and will be removed", jt->MakeView());
                optionalInstanceExts -= ext;
            }
            jt = pExtensionNames->erase(jt);
        }
        else {
            LOG(RHI, Verbose, L"vulkan instance extension '{0}' is supported and will be initialized", jt->MakeView());
            ++jt;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
bool CreateVulkanInstance_(
    VkInstance* pVkInstance,
    const FVulkanGlobalAPI& api,
    const VkAllocationCallbacks* pVkAllocator,
    FConstChar applicationName,
    FConstChar engineName,
    EVulkanVersion& version,
    const TMemoryView<const FConstChar> layers,
    FVulkanInstanceExtensionSet& requiredInstanceExtensions,
    FVulkanInstanceExtensionSet& optionalInstanceExtensions,
    const FVulkanDeviceExtensionSet& requiredDeviceExtensions,
    const FVulkanDeviceExtensionSet& optionalDeviceExtensions ) {
    STATIC_ASSERT(sizeof(FConstChar) == sizeof(const char*));
    Assert_NoAssume(VK_NULL_HANDLE == *pVkInstance);

    u32 currentVersion = 0;
    VK_CHECK( api.vkEnumerateInstanceVersion(&currentVersion) );
    const u32 apiVersion = Min( static_cast<u32>(version), Max(VK_API_VERSION_1_0, currentVersion) );
    version = static_cast<EVulkanVersion>(VK_MAKE_VERSION(VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), 0));

    VECTOR(RHIInstance, FConstChar) layerNames{ layers.begin(), layers.end() };
    if (not ValidateInstanceLayers_(&layerNames, api))
        return false;

    VECTOR(RHIInstance, FConstChar) extensionNames;
    if (not ValidateInstanceExtensions_(
        &extensionNames, api,
        requiredInstanceExtensions,
        optionalInstanceExtensions,
        requiredDeviceExtensions,
        optionalDeviceExtensions ))
        return false;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = apiVersion;
    appInfo.pApplicationName = applicationName;
    appInfo.pEngineName = engineName;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = checked_cast<u32>(layerNames.size());
    createInfo.ppEnabledLayerNames = reinterpret_cast<const char* const*>(layerNames.data());
    createInfo.enabledExtensionCount = checked_cast<u32>(extensionNames.size());
    createInfo.ppEnabledExtensionNames = reinterpret_cast<const char* const*>(extensionNames.data());

    LOG(RHI, Debug, L"create vulkan instance version {0}\n\t- Layers: {1}\n\t- Extensions: {2}",
        version, Fmt::CommaSeparatedW(layerNames.MakeView()), Fmt::CommaSeparatedW(extensionNames.MakeView()) );

    VK_CHECK( api.vkCreateInstance(&createInfo, pVkAllocator, pVkInstance) );

    return true;
}
//----------------------------------------------------------------------------
// Debug utils
//----------------------------------------------------------------------------
#if USE_PPE_LOGGER
#if USE_PPE_RHIDEBUG
LOG_CATEGORY_VERBOSITY(, VkGeneral, All)
#else
LOG_CATEGORY_VERBOSITY(, VkGeneral, NoDebug)
#endif
LOG_CATEGORY_VERBOSITY(, VkPerformance, All)
LOG_CATEGORY_VERBOSITY(, VkValidation, All)
static void CreateDebugUtilsMessengerIFP_(
    VkDebugUtilsMessengerEXT* pDebugUtilsMessenger,
    VkInstance vkInstance,
    const VkAllocationCallbacks* vkAllocator,
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT ) {
    PFN_vkDebugUtilsMessengerCallbackEXT debugCallback = [](
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData ) -> VkBool32 {
        Unused(pUserData);

        ELoggerVerbosity level;
        switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            level = ELoggerVerbosity::Verbose; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            level = ELoggerVerbosity::Info; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            level = ELoggerVerbosity::Warning; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            level = ELoggerVerbosity::Error; break;
        default:
            AssertNotImplemented();
        }

        const FLoggerCategory* pLogCategory = &LOG_CATEGORY_GET(RHI);
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
            pLogCategory = &LOG_CATEGORY_GET(VkGeneral);
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            pLogCategory = &LOG_CATEGORY_GET(VkValidation);
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            pLogCategory = &LOG_CATEGORY_GET(VkPerformance);

        if (not (pLogCategory->Verbosity & level))
            return false;

        auto fmtCallbackData = [pCallbackData, level](FWTextWriter& oss) {
            if (level <= ELoggerVerbosity::Verbose)
                return;

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
                case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV: objectTypeName = L"ACCELERATION_STRUCTURE_NV"; break;
                default:
                    AssertNotImplemented();
                }

                Format(oss, L" --OBJECT-- {0} - <{1}> - \"{2}\"",
                    Fmt::Pointer(bit_cast<void*>(obj.objectHandle)),
                    objectTypeName,
                    MakeCStringView(obj.pObjectName));
            }
        };

        FLogger::Log(
            *pLogCategory,
            level,
            FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
            L"{0}{1}",
            MakeCStringView(pCallbackData->pMessage),
            Fmt::Formator<wchar_t>(fmtCallbackData));

        return VK_FALSE;
    };

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

#if USE_PPE_DEBUG
    createInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
#endif

    *pDebugUtilsMessenger = VK_NULL_HANDLE;
    VK_CALL( vkCreateDebugUtilsMessengerEXT(vkInstance, &createInfo, vkAllocator, pDebugUtilsMessenger) );

    LOG(RHI, Verbose, L"created vulkan debug utils messenger for performance and validation");
}
#endif //!USE_PPE_LOGGER
//----------------------------------------------------------------------------
// Device features
//----------------------------------------------------------------------------
struct FVulkanDeviceFeatures_ {

    VkPhysicalDeviceFeatures Main{};
    VkPhysicalDeviceFeatures2 Main2{};
    //VkDeviceGeneratedCommandsFeaturesNVX GeneratedCommands;
    VkPhysicalDeviceMultiviewFeatures Multiview{};
    //VkPhysicalDeviceShaderAtomicInt64FeaturesKHR ShaderAtomicI64;
    VkPhysicalDevice8BitStorageFeaturesKHR Storage8Bit{};
    VkPhysicalDevice16BitStorageFeatures Storage16Bit{};
    VkPhysicalDeviceSamplerYcbcrConversionFeatures SamplerYcbcrConversion{};
    VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT BlendOpAdvanced{};
    VkPhysicalDeviceConditionalRenderingFeaturesEXT ConditionalRendering{};
    VkPhysicalDeviceShaderDrawParameterFeatures ShaderDrawParameters{};
    #ifdef VK_NV_mesh_shader
    VkPhysicalDeviceMeshShaderFeaturesNV MeshShader{};
    #endif
    #ifdef VK_EXT_descriptor_indexing
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT DescriptorIndexing{};
    #endif
    //VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT VertexAttribDivisor;
    //VkPhysicalDeviceASTCDecodeFeaturesEXT AstcDecode;
    #ifdef VK_KHR_vulkan_memory_model
    VkPhysicalDeviceVulkanMemoryModelFeaturesKHR MemoryModel{};
    #endif
    #ifdef VK_EXT_inline_uniform_block
    VkPhysicalDeviceInlineUniformBlockFeaturesEXT InlineUniformBlock{};
    #endif
    #ifdef VK_NV_representative_fragment_test
    VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV RepresentativeFragmentTest{};
    #endif
    //VkPhysicalDeviceExclusiveScissorFeaturesNV ExclusiveScissorTest;
    //VkPhysicalDeviceCornerSampledImageFeaturesNV CornerSampledImage;
    //VkPhysicalDeviceComputeShaderDerivativesFeaturesNV ComputeShaderDerivatives;
    #ifdef VK_NV_fragment_shader_barycentric
    VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV FragmentShaderBarycentric{};
    #endif
    #ifdef VK_NV_shader_image_footprint
    VkPhysicalDeviceShaderImageFootprintFeaturesNV ShaderImageFootprint{};
    #endif
    #ifdef VK_NV_shading_rate_image
    VkPhysicalDeviceShadingRateImageFeaturesNV ShadingRateImage{};
    #endif
    #ifdef VK_KHR_shader_float16_int8
    VkPhysicalDeviceShaderFloat16Int8FeaturesKHR ShaderFloat16Int8{};
    #endif
    #ifdef VK_KHR_timeline_semaphore
    VkPhysicalDeviceTimelineSemaphoreFeaturesKHR TimelineSemaphore{};
    #endif
    #ifdef VK_KHR_buffer_device_address
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR BufferDeviceAddress{};
    #endif
    #ifdef VK_KHR_shader_atomic_int64
    VkPhysicalDeviceShaderAtomicInt64FeaturesKHR ShaderAtomicInt64{};
    #endif
    #ifdef VK_KHR_shader_clock
    VkPhysicalDeviceShaderClockFeaturesKHR ShaderClock{};
    #endif
    #ifdef VK_EXT_extended_dynamic_state
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT ExtendedDynamicState{};
    #endif
    #ifdef VK_KHR_ray_tracing_pipeline
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR RayTracing{};
    #endif
    #ifdef VK_AMD_device_coherent_memory
    VkPhysicalDeviceCoherentMemoryFeaturesAMD DeviceCoherentMemory{};
    #endif
    #ifdef VK_EXT_memory_priority
    VkPhysicalDeviceMemoryPriorityFeaturesEXT MemoryPriority{};
    #endif
};
//----------------------------------------------------------------------------
void SetupDeviceFeatures_(FVulkanDeviceFeatures_* pFeatures, void** nextExt, VkPhysicalDevice vkPhysicalDevice, const FVulkanInstanceFunctions& api, const FVulkanDeviceExtensionSet& extensions) {
    void** nextFeatures = &pFeatures->Main2.pNext;
    const auto enableFeature = [&](auto* pFeature, VkStructureType sType) NOEXCEPT {
        pFeature->sType = sType;
        *nextFeatures = pFeature;
        nextFeatures = &pFeature->pNext;
    };

    *nextExt = &pFeatures->Main2;

#ifdef VK_KHR_multiview
    if (extensions & EVulkanDeviceExtension::KHR_multiview)
        enableFeature(&pFeatures->Multiview, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES);
#endif
#ifdef VK_KHR_8bit_storage
    if (extensions & EVulkanDeviceExtension::KHR_8bit_storage)
        enableFeature(&pFeatures->Storage8Bit, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR);
#endif
#ifdef VK_KHR_16bit_storage
    if (extensions & EVulkanDeviceExtension::KHR_16bit_storage)
        enableFeature(&pFeatures->Storage16Bit, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES);
#endif
#ifdef VK_KHR_sampler_ycbcr_conversion
    if (extensions & EVulkanDeviceExtension::KHR_sampler_ycbcr_conversion)
        enableFeature(&pFeatures->SamplerYcbcrConversion, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES);
#endif
#ifdef VK_EXT_blend_operation_advanced
    if (extensions & EVulkanDeviceExtension::EXT_blend_operation_advanced)
        enableFeature(&pFeatures->BlendOpAdvanced, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT);
#endif
#ifdef VK_EXT_descriptor_indexing
    if (api.version() >= EVulkanVersion::API_version_1_2 or
        extensions & EVulkanDeviceExtension::EXT_descriptor_indexing )
        enableFeature(&pFeatures->DescriptorIndexing, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT);
#endif
#ifdef VK_NV_mesh_shader
    if (extensions & EVulkanDeviceExtension::NV_mesh_shader)
        enableFeature(&pFeatures->MeshShader, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV);
#endif
#ifdef VK_KHR_vulkan_memory_model
    if (extensions & EVulkanDeviceExtension::KHR_vulkan_memory_model)
        enableFeature(&pFeatures->MemoryModel, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES_KHR);
#endif
#ifdef VK_EXT_inline_uniform_block
    if (extensions & EVulkanDeviceExtension::EXT_inline_uniform_block)
        enableFeature(&pFeatures->InlineUniformBlock, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT);
#endif
#ifdef VK_NV_representative_fragment_test
    if (extensions & EVulkanDeviceExtension::NV_representative_fragment_test)
        enableFeature(&pFeatures->RepresentativeFragmentTest, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV);
#endif
#ifdef VK_NV_fragment_shader_barycentric
    if (extensions & EVulkanDeviceExtension::NV_fragment_shader_barycentric)
        enableFeature(&pFeatures->FragmentShaderBarycentric, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_NV);
#endif
#ifdef VK_NV_shader_image_footprint
    if (extensions & EVulkanDeviceExtension::NV_shader_image_footprint)
        enableFeature(&pFeatures->ShaderImageFootprint, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV);
#endif
#ifdef VK_NV_shading_rate_image
    if (extensions & EVulkanDeviceExtension::NV_shading_rate_image)
        enableFeature(&pFeatures->ShadingRateImage, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV);
#endif
#ifdef VK_KHR_shader_float16_int8
    if (api.version() >= EVulkanVersion::API_version_1_2 or
        extensions & EVulkanDeviceExtension::KHR_shader_float16_int8)
        enableFeature(&pFeatures->ShaderFloat16Int8, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR);
#endif
#ifdef VK_KHR_timeline_semaphore
    if (api.version() >= EVulkanVersion::API_version_1_2 or
        extensions & EVulkanDeviceExtension::KHR_timeline_semaphore)
        enableFeature(&pFeatures->TimelineSemaphore, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR);
#endif
#ifdef VK_KHR_buffer_device_address
    if (api.version() >= EVulkanVersion::API_version_1_2 or
        extensions & EVulkanDeviceExtension::KHR_buffer_device_address)
        enableFeature(&pFeatures->BufferDeviceAddress, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR);
#endif
#ifdef VK_KHR_shader_atomic_int64
    if (api.version() >= EVulkanVersion::API_version_1_2 or
        extensions & EVulkanDeviceExtension::KHR_shader_atomic_int64)
        enableFeature(&pFeatures->ShaderAtomicInt64, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES_KHR);
#endif
#ifdef VK_KHR_shader_clock
    if (extensions & EVulkanDeviceExtension::KHR_shader_clock)
        enableFeature(&pFeatures->ShaderClock, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR);
#endif
#ifdef VK_EXT_extended_dynamic_state
    if (extensions & EVulkanDeviceExtension::EXT_extended_dynamic_state)
        enableFeature(&pFeatures->ExtendedDynamicState, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT);
#endif
#ifdef VK_KHR_ray_tracing_pipeline
    if (extensions & EVulkanDeviceExtension::KHR_ray_tracing_pipeline)
        enableFeature(&pFeatures->RayTracing, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR);
#endif
#ifdef VK_AMD_device_coherent_memory
    if (extensions & EVulkanDeviceExtension::AMD_device_coherent_memory) {
        pFeatures->DeviceCoherentMemory.deviceCoherentMemory = VK_TRUE;
        enableFeature(&pFeatures->DeviceCoherentMemory, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD);
    }
#endif
#ifdef VK_EXT_memory_priority
    if (extensions & EVulkanDeviceExtension::EXT_memory_priority)
        enableFeature(&pFeatures->MemoryPriority, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT);
#endif

    enableFeature(&pFeatures->ConditionalRendering, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT);
    enableFeature(&pFeatures->ShaderDrawParameters, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETER_FEATURES);

    api.vkGetPhysicalDeviceFeatures2( vkPhysicalDevice, &pFeatures->Main2 );
}
//----------------------------------------------------------------------------
// Device queues
//----------------------------------------------------------------------------
bool ChooseVulkanQueueIndex_(
    u32* pFamilyIndex,
    const TMemoryView<const VkQueueFamilyProperties>& families,
    VkQueueFlags& requiredFlags ) {
    // validate required flags:
    //   if the capabilities of a queue family include VK_QUEUE_GRAPHICS_BIT or VK_QUEUE_COMPUTE_BIT,
    //   then reporting the VK_QUEUE_TRANSFER_BIT capability separately for that queue family is optional.
    if (requiredFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
        requiredFlags &= ~VK_QUEUE_TRANSFER_BIT;

    TPair<VkQueueFlags, u32> compatible{ 0, UMax };
    forrange(i, 0, u32(families.size())) {
        const VkQueueFamilyProperties& properties = families[i];
        VkQueueFlags queueFlags = properties.queueFlags;

        if (queueFlags == requiredFlags) {
            requiredFlags = queueFlags;
            *pFamilyIndex = i;
            return true;
        }

        if ((Meta::EnumHas(queueFlags, static_cast<VkFlags>(requiredFlags))) &&
            (compatible.first == 0 || TBitMask<u32>{ compatible.first }.Count() > TBitMask<u32>{ queueFlags }.Count())) {
            compatible = MakePair(queueFlags, i);
        }
    }

    if (compatible.first) {
        requiredFlags = compatible.first;
        *pFamilyIndex = compatible.second;
        return true;
    }

    LOG(RHI, Error, L"no suitable vulkan queue family found");
    return false;
}
//----------------------------------------------------------------------------
bool SetupVulkanQueues_(
    TFixedSizeStack<FVulkanDeviceQueueInfo, MaxQueueFamilies>* pQueues,
    VkPhysicalDevice vkPhysicalDevice,
    VkSurfaceKHR vkSurface,
    const FVulkanInstanceFunctions& api,
    const TMemoryView<const FVulkanInstance::FQueueCreateInfo>& createInfos ) {
    u32 numFamilies = 0;
    api.vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &numFamilies, nullptr);

    STACKLOCAL_POD_ARRAY(VkQueueFamilyProperties, familyProperties, numFamilies);
    api.vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &numFamilies, familyProperties.data());

    // setup default queue
    if (createInfos.empty()) {
        u32 familyIndex = 0;
        VkQueueFlags queueFlags = static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
        if (not ChooseVulkanQueueIndex_(&familyIndex, familyProperties, queueFlags))
            return false;

        VkBool32 supportsPresent = false;
        if (VK_NULL_HANDLE != vkSurface)
            VK_CALL( api.vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, familyIndex, vkSurface, &supportsPresent) );

        pQueues->Push(FVulkanDeviceQueueInfo{
            VK_NULL_HANDLE,
            static_cast<EVulkanQueueFamily>(familyIndex),
            UMax, queueFlags, 0.0f, {
                familyProperties[familyIndex].minImageTransferGranularity.width,
                familyProperties[familyIndex].minImageTransferGranularity.height,
                familyProperties[familyIndex].minImageTransferGranularity.depth
            },
            !!supportsPresent
            ARGS_IF_RHIDEBUG(MakeStringView("DefaultQueue"))
        });
        return true;
    }

    STACKLOCAL_POD_ARRAY(u32, queueCount, numFamilies);
    Broadcast(queueCount, 0);

    for (const FVulkanInstance::FQueueCreateInfo& queue : createInfos) {
        u32 familyIndex = 0;
        VkQueueFlags queueFlags{ queue.Flags };
        if (not ChooseVulkanQueueIndex_(&familyIndex, familyProperties, queueFlags))
            return false;

        const u32 queueIndex = queueCount[familyIndex]++;
        AssertRelease(queueIndex < familyProperties[familyIndex].queueCount);

        if (queueCount[familyIndex] == familyProperties[familyIndex].queueCount)
            familyProperties[familyIndex].queueFlags = 0; // can't allocate from this queue anymore

        VkBool32 supportsPresent = false;
        if (VK_NULL_HANDLE != vkSurface)
            VK_CALL( api.vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, familyIndex, vkSurface, &supportsPresent) );

        pQueues->Push(FVulkanDeviceQueueInfo{
            VK_NULL_HANDLE,
            static_cast<EVulkanQueueFamily>(familyIndex),
            UMax, queueFlags, queue.Priority,
            {
                familyProperties[familyIndex].minImageTransferGranularity.width,
                familyProperties[familyIndex].minImageTransferGranularity.height,
                familyProperties[familyIndex].minImageTransferGranularity.depth
            },
            !!supportsPresent
            ARGS_IF_RHIDEBUG(queue.DebugName)
        });
    }

    return true;
}
//----------------------------------------------------------------------------
// Device creation
//----------------------------------------------------------------------------
bool ValidateDeviceExtensions_(
    VECTOR(RHIInstance, FConstChar)* pExtensionNames,
    const FVulkanInstanceFunctions& api,
    FVulkanDeviceInfo& deviceInfo ) {
    FVulkanDeviceExtensionSet& requiredDeviceExts = deviceInfo.RequiredDeviceExtensions;
    FVulkanDeviceExtensionSet& optionalDeviceExts = deviceInfo.OptionalDeviceExtensions;

#if defined(VK_EXT_load_store_op_none) && VK_EXT_load_store_op_none
    // See FVulkanInstance::RecommendedDeviceExtensions()
    if (deviceInfo.Features & ERHIFeature::Debugging) {
        // Validation layer is not compatible with this extension:
        //   Device Extension VK_EXT_load_store_op_none is not supported by this layer.
        //   Using this extension may adversely affect validation results and/or produce undefined behavior.
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_EXT_load_store_op_none.html

        // Need to handle this extension dynamically, see EAttachmentLoadOp/StoreOp
        if (requiredDeviceExts & EVulkanDeviceExtension::EXT_load_store_op_none ||
            optionalDeviceExts & EVulkanDeviceExtension::EXT_load_store_op_none ) {
            LOG(RHI, Warning, L"disabled EXT_load_store_op_none extension since it's not compatible with KHRONOS validation layer");

            requiredDeviceExts -= EVulkanDeviceExtension::EXT_load_store_op_none;
            optionalDeviceExts -= EVulkanDeviceExtension::EXT_load_store_op_none;
        }
    }
#endif

    const FVulkanDeviceExtensionSet userDeviceExts{ requiredDeviceExts };

    // expand requirements:
    requiredDeviceExts = vk::device_extensions_require(requiredDeviceExts);
    optionalDeviceExts = vk::device_extensions_require(optionalDeviceExts);

    pExtensionNames->reserve(
        requiredDeviceExts.Count() +
        optionalDeviceExts.Count() );

    FVulkanDeviceExtensionSet it{ requiredDeviceExts | optionalDeviceExts };
    for (;;) {
        const auto bit = it.PopFront();
        if (not bit) break;
        const auto ext = static_cast<EVulkanDeviceExtension>(bit - 1);

        FConstChar name{ vk::device_extension_name(ext) };
        AssertRelease(name);

        CLOG(not (userDeviceExts & ext), RHI, Warning, L"add vulkan device extension '{0}' required by user input", name.MakeView());

        // check for missing instance extensions required by this device extension
        FVulkanInstanceExtensionSet missingInstanceExts{
            vk::instance_extensions_require(vk::device_extension_set{ static_cast<u32>(ext) }) -
            api.instance_extensions()
        };
        for (;;) {
            const auto missingBit = missingInstanceExts.PopFront();
            if (not missingBit) break;
            LOG(RHI, Error, L"missing vulkan instance extension {0} required by device extension {1}",
                MakeCStringView(vk::instance_extension_name(static_cast<vk::instance_extension>(missingBit - 1))),
                MakeCStringView(vk::device_extension_name(ext)) );
        }

        pExtensionNames->push_back_AssumeNoGrow(std::move(name));
    }

    u32 numExtensions = 0;
    VK_CHECK( api.vkEnumerateDeviceExtensionProperties(deviceInfo.vkPhysicalDevice, nullptr, &numExtensions, nullptr) );

    if (0 == numExtensions) {
        CLOG(numExtensions != pExtensionNames->size(), RHI, Warning, L"vulkan device extensions are not supported and will be ignored");
        pExtensionNames->clear();
        return true;
    }

    STACKLOCAL_POD_ARRAY(VkExtensionProperties, extProperties, numExtensions);
    VK_CHECK( api.vkEnumerateDeviceExtensionProperties(deviceInfo.vkPhysicalDevice, nullptr, &numExtensions, extProperties.data()) );

    for (auto jt = pExtensionNames->begin(); jt != pExtensionNames->end(); ) {
        const auto found = extProperties.FindIf([name{*jt}](const VkExtensionProperties& props) {
            return FConstChar(props.extensionName).Equals(name);
        });

        if (extProperties.end() == found) {
            const EVulkanDeviceExtension ext = vk::device_extension_from(*jt);
            if (userDeviceExts & ext) {
                LOG(RHI, Error, L"required vulkan device extension '{0}' is not supported and will be removed!", jt->MakeView());
                requiredDeviceExts -= ext;
            }
            else {
                LOG(RHI, Warning, L"optional vulkan device extension '{0}' is not supported and will be removed", jt->MakeView());
                optionalDeviceExts -= ext;
            }
            jt = pExtensionNames->erase(jt);
        }
        else {
            LOG(RHI, Verbose, L"vulkan device extension '{0}' is supported and will be initialized", jt->MakeView());
            ++jt;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
bool CreateVulkanDevice_(
    FVulkanDeviceInfo* pDevice,
    const FVulkanInstanceFunctions& api,
    VkSurfaceKHR vkSurface,
    FVulkanInstance::FPhysicalDeviceInfoRef physicalDevice,
    const TMemoryView<const FVulkanInstance::FQueueCreateInfo>& queues ) {
    pDevice->vkPhysicalDevice = physicalDevice->vkPhysicalDevice;
    Assert(pDevice->vkPhysicalDevice);

    LOG(RHI, Info, L"pick vulkan gpu: {0}", *physicalDevice);

    if (not SetupVulkanQueues_(&pDevice->Queues, pDevice->vkPhysicalDevice, vkSurface, api, queues)) {
        LOG(RHI, Error, L"failed to setup vulkan device queues");
        return false;
    }
    Assert_NoAssume(not pDevice->Queues.empty());
    Assert_NoAssume(pDevice->Queues.size() == queues.size() || queues.empty());

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    void** nextExt = const_cast<void**>(&deviceInfo.pNext);

    // extensions
    VECTOR(RHIInstance, FConstChar) extensionNames;
    if (not ValidateDeviceExtensions_(&extensionNames, api, *pDevice)) {
        LOG(RHI, Error, L"failed to validate vulkan device extensions");
        return false;
    }

    deviceInfo.enabledExtensionCount = checked_cast<u32>(extensionNames.size());
    deviceInfo.ppEnabledExtensionNames = reinterpret_cast<const char* const*>(extensionNames.data());

    // queues
    u32 maxQueueFamilies = 0;
    api.vkGetPhysicalDeviceQueueFamilyProperties(pDevice->vkPhysicalDevice, &maxQueueFamilies, nullptr);
    AssertRelease(maxQueueFamilies <= RHI::MaxQueueFamilies);

    using queue_priorities_t = TFixedSizeStack<float, 16>;
    STACKLOCAL_POD_STACK(queue_priorities_t, priorities, maxQueueFamilies);
    priorities.Resize(maxQueueFamilies);
    STACKLOCAL_POD_STACK(VkDeviceQueueCreateInfo, queueInfos, maxQueueFamilies);

    forrange(i, 0, maxQueueFamilies) {
        VkDeviceQueueCreateInfo& createInfo = *queueInfos.Push_Uninitialized();
        createInfo = VkDeviceQueueCreateInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.queueFamilyIndex = i;
        createInfo.queueCount = 0;
        createInfo.pQueuePriorities = priorities[i].data();
    }

    for (FVulkanDeviceQueueInfo& queue : pDevice->Queues) {
        const u32 q = static_cast<u32>(queue.FamilyIndex);
        queue.QueueIndex = (queueInfos[q].queueCount++);
        priorities[q].Push(queue.Priority);
    }

    for (size_t q = 0; q != queueInfos.size(); ) {
        if (0 == queueInfos[q].queueCount)
            queueInfos.EraseAt(q);
        else
            ++q;
    }
    AssertReleaseMessage(L"no valid device queue", not queueInfos.empty());

    deviceInfo.queueCreateInfoCount = checked_cast<u32>(queueInfos.size());
    deviceInfo.pQueueCreateInfos = queueInfos.data();

    const FVulkanDeviceExtensionSet allDeviceExtensions =
        (pDevice->RequiredDeviceExtensions | pDevice->OptionalDeviceExtensions);

    // features
    FVulkanDeviceFeatures_ features;
    features.Main = {};
    api.vkGetPhysicalDeviceFeatures(pDevice->vkPhysicalDevice, &features.Main);

    features.Main2 = {};
    if (api.version() >= vk::API_version_1_1) {
        features.Main2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        api.vkGetPhysicalDeviceFeatures2(pDevice->vkPhysicalDevice, &features.Main2);
    }

    SetupDeviceFeatures_(&features, nextExt, pDevice->vkPhysicalDevice, api, allDeviceExtensions);

    if (features.Main2.sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2) {
        deviceInfo.pEnabledFeatures = &features.Main; // enable all features supported
    }
    else {
        // VkDeviceCreateInfo->pNext includes a VkPhysicalDeviceFeatures2KHR struct when pCreateInfo->pEnabledFeatures is non-NULL.
        // The Vulkan spec states: If the pNext chain includes a VkPhysicalDeviceFeatures2 structure, then pEnabledFeatures must be NULL.
        // (https://vulkan.lunarg.com/doc/view/1.2.148.0/windows/1.2-extensions/vkspec.html#VUID-VkDeviceCreateInfo-pNext-00373)
        deviceInfo.pEnabledFeatures = nullptr;
    }

    // finally try to create the device
    VK_CHECK( api.vkCreateDevice(pDevice->vkPhysicalDevice, &deviceInfo, pDevice->pAllocator, &pDevice->vkDevice) );

    LOG(RHI, Info, L"created vulkan device {3} with {0} queues and {1} extensions: {2}",
        pDevice->Queues.size(),
        allDeviceExtensions.Count(),
        allDeviceExtensions,
        Fmt::Pointer(pDevice->vkDevice) );

    const char* apiError = pDevice->API.attach_return_error(api, pDevice->vkDevice,
        pDevice->RequiredDeviceExtensions, pDevice->OptionalDeviceExtensions );
    if (Unlikely(apiError)) {
        LOG(RHI, Error, L"failed to attach to vulkan device API: {0}", MakeCStringView(apiError));
        pDevice->API.vkDestroyDevice(pDevice->vkDevice, pDevice->pAllocator);
        pDevice->vkDevice = VK_NULL_HANDLE;
        return false;
    }

    pDevice->API.setup_backward_compatibility();

    const FVulkanDeviceFunctions deviceFn{ &pDevice->API };
    for (FVulkanDeviceQueueInfo& queue : pDevice->Queues)
        deviceFn.vkGetDeviceQueue(pDevice->vkDevice, static_cast<u32>(queue.FamilyIndex), queue.QueueIndex, &queue.Handle);

    return true;
}
//----------------------------------------------------------------------------
// Physical Device Info logging
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& WritePhysicalDeviceInfo_(TBasicTextWriter<_Char>& oss, const FVulkanInstance::FPhysicalDeviceInfo& info) {
    oss << STRING_LITERAL(_Char, '[') << info.Properties.deviceType
        << STRING_LITERAL(_Char, ':') << info.Properties.deviceID
        << STRING_LITERAL(_Char, "]  ") << info.DeviceName()
        << STRING_LITERAL(_Char, " - ") << info.VendorID()
        << STRING_LITERAL(_Char, " - Vulkan ") << info.ApiVersion()
        << STRING_LITERAL(_Char, " - Driver v") << info.Properties.driverVersion;

    u32 numMemoryTypes = 0;
    for (const VkMemoryType& type : MakeView(info.Memory.memoryTypes, info.Memory.memoryTypes + info.Memory.memoryTypeCount)) {
        oss << Eol << Tab
            << STRING_LITERAL(_Char, "- MemoryType#") << numMemoryTypes++
            << STRING_LITERAL(_Char, " Heap: ") << type.heapIndex
            << STRING_LITERAL(_Char, " Flags: ") << static_cast<VkMemoryPropertyFlagBits>(type.propertyFlags);
    }

    u32 numMemoryHeaps = 0;
    for (const VkMemoryHeap& heap : MakeView(info.Memory.memoryHeaps, info.Memory.memoryHeaps + info.Memory.memoryHeapCount)) {
        oss << Eol << Tab
            << STRING_LITERAL(_Char, "- MemoryHeap#") << numMemoryHeaps++
            << STRING_LITERAL(_Char, " Size: ") << Fmt::SizeInBytes(heap.size)
            << STRING_LITERAL(_Char, " Flags: ") << static_cast<VkMemoryHeapFlagBits>(heap.flags);
    }

    oss << Eol << Tab
        << STRING_LITERAL(_Char, "- Perf Rating = ") << info.PerfRating();

    return oss;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanInstance::FVulkanInstance()
:   _vkAllocationCallbacks(MakeVulkanAllocator_(this)) {
}
//----------------------------------------------------------------------------
FVulkanInstance::~FVulkanInstance() {
    Assert_NoAssume(not _vulkanLib.IsValid());
    Assert_NoAssume(VK_NULL_HANDLE == _vkInstance); // must call TearDown() !
#if USE_PPE_RHIDEBUG
    Assert_NoAssume(VK_NULL_HANDLE == _vkDebugUtilsMessenger);
#endif
}
//----------------------------------------------------------------------------
bool FVulkanInstance::Construct(
    FConstChar applicationName,
    FConstChar engineName,
    EVulkanVersion version,
    const TMemoryView<const FConstChar>& instanceLayers,
    FVulkanInstanceExtensionSet requiredInstanceExtensions,
    FVulkanInstanceExtensionSet optionalInstanceExtensions,
    FVulkanDeviceExtensionSet requiredDeviceExtensions,
    FVulkanDeviceExtensionSet optionalDeviceExtensions ) {
    Assert(applicationName);
    Assert(engineName);
    Assert_NoAssume(VK_NULL_HANDLE == _vkInstance);
    Assert_NoAssume(not _vulkanLib.IsValid());

    if (LoadVulkanGlobalAPI_(&_vulkanLib, &_exportedAPI, &_globalAPI) &&
        CreateVulkanInstance_(&_vkInstance, _globalAPI, vkAllocationCallbacks(), applicationName, engineName, version, instanceLayers,
            requiredInstanceExtensions, optionalInstanceExtensions,
            requiredDeviceExtensions, optionalDeviceExtensions )) {
        Assert(VK_NULL_HANDLE != _vkInstance);

        instance_api_ = &_instanceAPI;

        if (const char* err = _instanceAPI.attach_return_error(&_globalAPI, _vkInstance, version, requiredInstanceExtensions, optionalInstanceExtensions)) {
            LOG(RHI, Error, L"failed to attach vulkan instance API: {0}, abort!", MakeCStringView(err));

            vkDestroyInstance(_vkInstance, vkAllocationCallbacks());
            _vkInstance = VK_NULL_HANDLE;
        }
        else {
            LOG(RHI, Info, L"vulkan instance {0} available with {1} extensions: {2}", version, _instanceAPI.instance_extensions_.Count(), _instanceAPI.instance_extensions_);

            _instanceAPI.setup_backward_compatibility();

#if USE_PPE_LOGGER
            if (HasExtension(EVulkanInstanceExtension::EXT_debug_utils))
                CreateDebugUtilsMessengerIFP_(&_vkDebugUtilsMessenger, _vkInstance, vkAllocationCallbacks(), _instanceAPI.vkCreateDebugUtilsMessengerEXT);
#endif

            // retrieve physical device infos once and for all

            u32 numPhysicalDevices{ 0 };
            VK_CALL( vkEnumeratePhysicalDevices(_vkInstance, &numPhysicalDevices, nullptr) );

            STACKLOCAL_POD_ARRAY(VkPhysicalDevice, vkPhysicalDevices, numPhysicalDevices);
            VK_CALL( vkEnumeratePhysicalDevices(_vkInstance, &numPhysicalDevices, vkPhysicalDevices.data()) );

            _physicalDevices.resize_AssumeEmpty(numPhysicalDevices);
            forrange(i, 0, numPhysicalDevices) {
                FPhysicalDeviceInfo& physicalDevice = _physicalDevices[i];
                physicalDevice.vkPhysicalDevice = vkPhysicalDevices[i];
                Assert(VK_NULL_HANDLE != physicalDevice.vkPhysicalDevice);

                vkGetPhysicalDeviceProperties(physicalDevice.vkPhysicalDevice, &physicalDevice.Properties);
                vkGetPhysicalDeviceFeatures(physicalDevice.vkPhysicalDevice, &physicalDevice.Features);
                vkGetPhysicalDeviceMemoryProperties(physicalDevice.vkPhysicalDevice, &physicalDevice.Memory);
            }

            LOG(RHI, Debug, L"listed {0} available physical devices for vulkan:\n {1}",
                numPhysicalDevices,
                Fmt::Join(_physicalDevices.MakeView(), L"\n"));

            return true;
        }
    }

    Assert_NoAssume(VK_NULL_HANDLE == _vkInstance);
#if USE_PPE_RHIDEBUG
    Assert_NoAssume(VK_NULL_HANDLE == _vkDebugUtilsMessenger);
#endif

    _vulkanLib.UnloadIFP(); // cleanup state before leaving

    _exportedAPI = FVulkanExportedAPI{};
    _globalAPI = FVulkanGlobalAPI{};
    _instanceAPI = FVulkanInstanceAPI{};

    FVulkanInstanceFunctions::instance_api_ = nullptr;

    return false;
}
//----------------------------------------------------------------------------
void FVulkanInstance::TearDown() {
    Assert_NoAssume(VK_NULL_HANDLE != _vkInstance);
    Assert_NoAssume(_vulkanLib.IsValid());

    LOG(RHI, Debug, L"destroying vulkan instance");

#if USE_PPE_LOGGER
    if (VK_NULL_HANDLE != _vkDebugUtilsMessenger) {
        vkDestroyDebugUtilsMessengerEXT(_vkInstance, _vkDebugUtilsMessenger, vkAllocationCallbacks());
        _vkDebugUtilsMessenger = VK_NULL_HANDLE;
    }
#endif

    vkDestroyInstance(_vkInstance, vkAllocationCallbacks());
    _vkInstance = VK_NULL_HANDLE;

    _vulkanLib.Unload();

    _exportedAPI = FVulkanExportedAPI{};
    _globalAPI = FVulkanGlobalAPI{};
    _instanceAPI = FVulkanInstanceAPI{};

    FVulkanInstanceFunctions::instance_api_ = nullptr;
}
//----------------------------------------------------------------------------
bool FVulkanInstance::CreateSurface(VkSurfaceKHR* pSurface, FVulkanWindowHandle window) const {
    Assert(pSurface);
    Assert(window);
    Assert_NoAssume(VK_NULL_HANDLE != _vkInstance);

#if defined(VK_USE_PLATFORM_WIN32_KHR) && VK_USE_PLATFORM_WIN32_KHR
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.flags = 0;
    createInfo.hwnd = static_cast<HWND>(window);
    createInfo.hinstance = static_cast<::HINSTANCE>(FCurrentProcess::Get().AppHandle());

    VK_CHECK( vkCreateWin32SurfaceKHR(_vkInstance, &createInfo, vkAllocationCallbacks(), pSurface) );

#elif defined(VK_USE_PLATFORM_ANDROID_KHR) && VK_USE_PLATFORM_ANDROID_KHR
    VkAndroidSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.flags = 0;
    createInfo.window = static_cast<::ANativeWindow>(window);

    VK_CHECK( vkCreateAndroidSurfaceKHR(_vkInstance, &createInfo, vkAllocationCallbacks(), pSurface) );

#elif defined(VK_USE_PLATFORM_XLIB_KHR) && VK_USE_PLATFORM_XLIB_KHR
    VkXlibSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.flags = 0;
    createInfo.window = reinterpret_cast<::Window>(window);

    VK_CHECK( vkCreateXlibSurfaceKHR(_vkInstance, &createInfo, vkAllocationCallbacks(), pSurface) );

#else
#   error "platform not supported"
#endif

    LOG(RHI, Debug, L"created vulkan surface {0} -> {1}", Fmt::Pointer((void*)window), Fmt::Pointer((void*)*pSurface));

    Assert_NoAssume(VK_NULL_HANDLE != *pSurface);
    return true;
}
//----------------------------------------------------------------------------
void FVulkanInstance::DestroySurface(VkSurfaceKHR vkSurface) const {
    Assert(VK_NULL_HANDLE != vkSurface);
    Assert_NoAssume(VK_NULL_HANDLE != _vkInstance);

    LOG(RHI, Debug, L"destroying vulkan surface {0}", Fmt::Pointer((void*)vkSurface));

    vkDestroySurfaceKHR(_vkInstance, vkSurface, vkAllocationCallbacks());
}
//----------------------------------------------------------------------------
bool FVulkanInstance::CreateDevice(
    FVulkanDeviceInfo* pDevice,
    VkSurfaceKHR vkSurface,
    const FVulkanDeviceExtensionSet& requiredDeviceExtensions,
    const FVulkanDeviceExtensionSet& optionalDeviceExtensions,
    FPhysicalDeviceInfoRef physicalDevice,
    const TMemoryView<const FQueueCreateInfo>& queues ) const {
    Assert(pDevice);
    Assert_NoAssume(VK_NULL_HANDLE != _vkInstance);

    if (not physicalDevice)
        physicalDevice = PickHighPerformanceDevice();

    pDevice->vkInstance = _vkInstance;
    pDevice->RequiredInstanceExtensions = _instanceAPI.instance_extensions_;
    pDevice->RequiredDeviceExtensions = requiredDeviceExtensions;
    pDevice->OptionalDeviceExtensions = optionalDeviceExtensions;
    pDevice->pAllocator = vkAllocationCallbacks();

    return CreateVulkanDevice_(pDevice, Fn(), vkSurface, physicalDevice, queues);
}
//----------------------------------------------------------------------------
bool FVulkanInstance::CreateDevice(
    FVulkanDeviceInfo* pDevice,
    const FVulkanDeviceExtensionSet& requiredDeviceExtensions,
    const FVulkanDeviceExtensionSet& optionalDeviceExtensions,
    FPhysicalDeviceInfoRef physicalDevice,
    const TMemoryView<const FQueueCreateInfo>& queues ) const {
    Assert(pDevice);
    Assert_NoAssume(VK_NULL_HANDLE != _vkInstance);

    if (not physicalDevice)
        physicalDevice = PickHighPerformanceDevice();

    pDevice->vkInstance = _vkInstance;
    pDevice->RequiredInstanceExtensions = _instanceAPI.instance_extensions_;
    pDevice->RequiredDeviceExtensions = requiredDeviceExtensions;
    pDevice->OptionalDeviceExtensions = optionalDeviceExtensions;
    pDevice->pAllocator = vkAllocationCallbacks();

    return CreateVulkanDevice_(pDevice, Fn(), VK_NULL_HANDLE, physicalDevice, queues);
}
//----------------------------------------------------------------------------
void FVulkanInstance::DestroyDevice(FVulkanDeviceInfo* pDevice) const {
    Assert(pDevice);
    Assert(VK_NULL_HANDLE != pDevice->vkDevice);
    Assert_NoAssume(VK_NULL_HANDLE != _vkInstance);
    Assert_NoAssume(vkAllocationCallbacks() == pDevice->pAllocator);

    LOG(RHI, Debug, L"destroying vulkan device {0}", Fmt::Pointer(pDevice->vkDevice));

    pDevice->API.vkDestroyDevice(pDevice->vkDevice, pDevice->pAllocator);

    *pDevice = FVulkanDeviceInfo{}; // reset structure before yielding
}
//----------------------------------------------------------------------------
bool FVulkanInstance::FPhysicalDeviceInfo::IsDiscreteGPU() const NOEXCEPT {
    return (Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
}
//----------------------------------------------------------------------------
bool FVulkanInstance::FPhysicalDeviceInfo::IsIntegratedGPU() const NOEXCEPT {
    return (Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
}
//----------------------------------------------------------------------------
float FVulkanInstance::FPhysicalDeviceInfo::PerfRating() const noexcept {
    return (
        static_cast<float>(IsDiscreteGPU() and not IsIntegratedGPU() ? 4 : 1) +
        static_cast<float>(Properties.limits.maxComputeSharedMemorySize >> 10) / 64.0f + // big local cache is good
        static_cast<float>(Properties.limits.maxComputeWorkGroupInvocations) / 1024.0f +
        static_cast<float>(Features.tessellationShader) +
        static_cast<float>(Features.geometryShader) );
}
//----------------------------------------------------------------------------
auto FVulkanInstance::PickHighPerformanceDevice() const NOEXCEPT -> FPhysicalDeviceInfoRef {
    FPhysicalDeviceInfoRef anyDevice{};
    FPhysicalDeviceInfoRef highPerfDevice{};

    float maxPerformance = 0.f;
    for (const FPhysicalDeviceInfo& pdi : _physicalDevices) {
        const float perfRating = pdi.PerfRating();
        if (perfRating > maxPerformance) {
            maxPerformance = perfRating;
            highPerfDevice = MakePtrRef(pdi);
        }

        if (not anyDevice)
            anyDevice = MakePtrRef(pdi);
    }

    return (highPerfDevice ? highPerfDevice : anyDevice);
}
//----------------------------------------------------------------------------
auto FVulkanInstance::PickPhysicalDeviceByName(FStringView deviceName) const NOEXCEPT -> FPhysicalDeviceInfoRef {
    Assert(VK_NULL_HANDLE != _vkInstance);

    return _physicalDevices.MakeConstView().Any([deviceName](const FPhysicalDeviceInfo& pdi) NOEXCEPT -> bool{
        return (HasSubStringI(MakeCStringView(pdi.Properties.deviceName), deviceName) or
                HasSubStringI(EVulkanVendor_Name(pdi.VendorID()), deviceName));
    });
}
//----------------------------------------------------------------------------
TMemoryView<const FConstChar> FVulkanInstance::DebuggingInstanceLayers(EVulkanVersion) NOEXCEPT {
    static const FConstChar GLayerNames[] = {
        //"VK_LAYER_LUNARG_api_dump", // for logging all VK calls %_NOCOMMIT%
        "VK_LAYER_KHRONOS_validation",

#if defined(VK_USE_PLATFORM_ANDROID_KHR) && VK_USE_PLATFORM_ANDROID_KHR
        // may be unsupported:
        "VK_LAYER_ARM_MGD",
        "VK_LAYER_ARM_mali_perf_doc",

#else
        "VK_LAYER_LUNARG_standard_validation", // for old VulkanSDK
        "VK_LAYER_LUNARG_parameter_validation",

#endif
    };
    return MakeView(GLayerNames);
}
//----------------------------------------------------------------------------
TMemoryView<const FConstChar> FVulkanInstance::ProfilingInstanceLayers(EVulkanVersion) NOEXCEPT {
#if defined(VK_USE_PLATFORM_ANDROID_KHR) && VK_USE_PLATFORM_ANDROID_KHR
    static const FConstChar GLayerNames[] = {
        "VK_LAYER_ARM_mali_perf_doc"
    };
    return MakeView(GLayerNames);
#else
    return {}; // no profiling instance layers for now
#endif
}
//----------------------------------------------------------------------------
TMemoryView<const FConstChar> FVulkanInstance::RecommendedInstanceLayers(EVulkanVersion ) NOEXCEPT {
    return {}; // no recommended instance layers for now
}
//----------------------------------------------------------------------------
TMemoryView<const FVulkanInstance::FQueueCreateInfo> FVulkanInstance::RecommendedDeviceQueues(EVulkanVersion ) NOEXCEPT {
    static const FQueueCreateInfo GCreateInfos[] = {
        { VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_SPARSE_BINDING_BIT, 0.f ARGS_IF_RHIDEBUG("Graphics") },
        { VK_QUEUE_COMPUTE_BIT, 0.f ARGS_IF_RHIDEBUG("Compute") },
        { VK_QUEUE_TRANSFER_BIT, 0.f ARGS_IF_RHIDEBUG("Transfer") }
    };
    return GCreateInfos;
}
//----------------------------------------------------------------------------
FVulkanInstanceExtensionSet FVulkanInstance::RequiredInstanceExtensions(EVulkanVersion version) NOEXCEPT {
    if (EVulkanVersion::API_version_1_0 == version)
        return vk::instance_extensions_require(FVulkanInstanceExtensionSet{
    #ifdef VK_KHR_surface
            EVulkanInstanceExtension::KHR_surface,
    #endif
    #ifdef VK_KHR_get_physical_device_properties2
            EVulkanInstanceExtension::KHR_get_physical_device_properties_2,
    #endif
    #ifdef VK_KHR_get_surface_capabilities2
            EVulkanInstanceExtension::KHR_get_surface_capabilities_2,
    #endif
    #ifdef VK_KHR_device_group_creation
            EVulkanInstanceExtension::KHR_device_group_creation,
    #endif
            });
    if (EVulkanVersion::API_version_1_1 == version)
        return vk::instance_extensions_require(FVulkanInstanceExtensionSet{
    #ifdef VK_KHR_surface
            EVulkanInstanceExtension::KHR_surface,
    #endif
    #ifdef VK_KHR_get_surface_capabilities2
            EVulkanInstanceExtension::KHR_get_surface_capabilities_2,
    #endif
            });
    if (EVulkanVersion::API_version_1_2 == version)
        return vk::instance_extensions_require(FVulkanInstanceExtensionSet{
    #ifdef VK_KHR_surface
            EVulkanInstanceExtension::KHR_surface,
    #endif
    #ifdef VK_KHR_get_surface_capabilities2
            EVulkanInstanceExtension::KHR_get_surface_capabilities_2,
    #endif
            });
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
FVulkanInstanceExtensionSet FVulkanInstance::DebuggingInstanceExtensions(EVulkanVersion version) NOEXCEPT {
    if (EVulkanVersion::API_version_1_0 == version)
        return vk::instance_extensions_require(FVulkanInstanceExtensionSet{
    #ifdef VK_EXT_debug_utils
            EVulkanInstanceExtension::EXT_debug_utils,
    #endif
            });
    if (EVulkanVersion::API_version_1_1 == version)
        return vk::instance_extensions_require(FVulkanInstanceExtensionSet{
    #ifdef VK_EXT_debug_utils
            EVulkanInstanceExtension::EXT_debug_utils,
    #endif
            });
    if (EVulkanVersion::API_version_1_2 == version)
        return vk::instance_extensions_require(FVulkanInstanceExtensionSet{
    #ifdef VK_EXT_debug_utils
            EVulkanInstanceExtension::EXT_debug_utils,
    #endif
            });
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
FVulkanInstanceExtensionSet FVulkanInstance::ProfilingInstanceExtensions(EVulkanVersion version) NOEXCEPT {
    Unused(version);
    return Default;
}
//----------------------------------------------------------------------------
FVulkanInstanceExtensionSet FVulkanInstance::RecommendedInstanceExtensions(EVulkanVersion version) NOEXCEPT {
    Unused(version);
    return Default;
}
//----------------------------------------------------------------------------
FVulkanInstanceExtensionSet FVulkanInstance::RequiredInstanceExtensions(EVulkanVersion, FWindowHandle window) NOEXCEPT {
    if (nullptr == window) return {}; // test if a window handle was really given
    return vk::instance_extensions_require(FVulkanInstanceExtensionSet{
#if defined(VK_USE_PLATFORM_WIN32_KHR) && VK_USE_PLATFORM_WIN32_KHR
        EVulkanInstanceExtension::KHR_win32_surface,
#elif defined(VK_USE_PLATFORM_ANDROID_KHR) && VK_USE_PLATFORM_ANDROID_KHR
        EVulkanInstanceExtension::KHR_android_surface,
#elif defined(VK_USE_PLATFORM_XLIB_KHR) && VK_USE_PLATFORM_XLIB_KHR
        EVulkanInstanceExtension::KHR_xlib_surface,
#else
#   error "platform not supported (#TODO)"
#endif
    });
}
//----------------------------------------------------------------------------
FVulkanDeviceExtensionSet FVulkanInstance::RequiredDeviceExtensions(EVulkanVersion version) NOEXCEPT {
    Unused(version);
    FVulkanDeviceExtensionSet result;

    // platform specific extensions
#if defined(VK_USE_PLATFORM_WIN32_KHR) && VK_USE_PLATFORM_WIN32_KHR
    result.Append({
        EVulkanDeviceExtension::KHR_external_fence_win32,
        EVulkanDeviceExtension::KHR_external_memory_win32,
        EVulkanDeviceExtension::KHR_external_semaphore_win32 });
#elif defined(VK_USE_PLATFORM_ANDROID_KHR) && VK_USE_PLATFORM_ANDROID_KHR
#elif defined(VK_USE_PLATFORM_XLIB_KHR) && VK_USE_PLATFORM_XLIB_KHR
#else
#   error "platform not supported (#TODO)"
#endif

#ifdef VK_KHR_swapchain
    result += EVulkanDeviceExtension::KHR_swapchain;
#endif

    return vk::device_extensions_require(result);
}
//----------------------------------------------------------------------------
FVulkanDeviceExtensionSet FVulkanInstance::DebuggingDeviceExtensions(EVulkanVersion version) NOEXCEPT {
    Unused(version);
    return Default;
}
//----------------------------------------------------------------------------
FVulkanDeviceExtensionSet FVulkanInstance::ProfilingDeviceExtensions(EVulkanVersion version) NOEXCEPT {
    Unused(version);
    FVulkanDeviceExtensionSet result;
#ifdef VK_KHR_shader_clock
    result += EVulkanDeviceExtension::KHR_shader_clock;
#endif
    return vk::device_extensions_require(result);
}
//----------------------------------------------------------------------------
FVulkanDeviceExtensionSet FVulkanInstance::RecommendedDeviceExtensions(EVulkanVersion version) NOEXCEPT {
    FVulkanDeviceExtensionSet result;

    if (EVulkanVersion::API_version_1_0 == version)
        result.Append({
    #ifdef VK_KHR_get_memory_requirements2
        EVulkanDeviceExtension::KHR_get_memory_requirements_2,
    #endif
    #ifdef VK_KHR_bind_memory2
        EVulkanDeviceExtension::KHR_bind_memory_2,
    #endif
    #ifdef VK_KHR_dedicated_allocation
        EVulkanDeviceExtension::KHR_dedicated_allocation,
    #endif
    #ifdef VK_KHR_multiview
        EVulkanDeviceExtension::KHR_multiview,
    #endif
    #ifdef VK_KHR_maintenance1
        EVulkanDeviceExtension::KHR_maintenance1,
    #endif
    #ifdef VK_KHR_maintenance2
        EVulkanDeviceExtension::KHR_maintenance2,
    #endif
    #ifdef VK_KHR_maintenance3
        EVulkanDeviceExtension::KHR_maintenance3,
    #endif
    #ifdef VK_KHR_create_renderpass2
        EVulkanDeviceExtension::KHR_create_renderpass_2,
    #endif
    #ifdef VK_KHR_sampler_ycbcr_conversion
        EVulkanDeviceExtension::KHR_sampler_ycbcr_conversion,
    #endif
    #ifdef VK_KHR_descriptor_update_template
        EVulkanDeviceExtension::KHR_descriptor_update_template,
    #endif
    #ifdef VK_KHR_device_group
        EVulkanDeviceExtension::KHR_device_group,
    #endif
    #ifdef VK_EXT_depth_range_unrestricted
        EVulkanDeviceExtension::EXT_depth_range_unrestricted,
    #endif
    #ifdef VK_EXT_sampler_filter_minmax
        EVulkanDeviceExtension::EXT_sampler_filter_minmax,
    #endif
    #ifdef VK_EXT_shader_stencil_export
        EVulkanDeviceExtension::EXT_shader_stencil_export,
    #endif
        });
    else if (EVulkanVersion::API_version_1_1 == version)
        result.Append({
    #ifdef VK_KHR_create_renderpass2
        EVulkanDeviceExtension::KHR_create_renderpass_2,
    #endif
    #ifdef VK_KHR_draw_indirect_count
        EVulkanDeviceExtension::KHR_draw_indirect_count,
    #endif
    #ifdef VK_KHR_8bit_storage
        EVulkanDeviceExtension::KHR_8bit_storage,
    #endif
    #ifdef VK_EXT_sample_locations
        EVulkanDeviceExtension::EXT_sample_locations,
    #endif
    #ifdef VK_KHR_push_descriptor
        EVulkanDeviceExtension::KHR_push_descriptor,
    #endif
    #ifdef VK_KHR_shader_atomic_int64
        EVulkanDeviceExtension::KHR_shader_atomic_int64,
    #endif
    #ifdef VK_KHR_shader_float16_int8
        EVulkanDeviceExtension::KHR_shader_float16_int8,
    #endif
    #ifdef VK_KHR_shader_float_controls
        EVulkanDeviceExtension::KHR_shader_float_controls,
    #endif
    #ifdef VK_EXT_blend_operation_advanced
        EVulkanDeviceExtension::EXT_blend_operation_advanced,
    #endif
    #ifdef VK_KHR_maintenance1
        EVulkanDeviceExtension::KHR_maintenance1, // required for VK_EXT_inline_uniform_block
    #endif
    #ifdef VK_EXT_inline_uniform_block
        EVulkanDeviceExtension::EXT_inline_uniform_block,
    #endif
    #ifdef VK_EXT_descriptor_indexing
        EVulkanDeviceExtension::EXT_descriptor_indexing,
    #endif
    #ifdef VK_EXT_memory_budget
        EVulkanDeviceExtension::EXT_memory_budget,
    #endif
    #ifdef VK_KHR_timeline_semaphore
        EVulkanDeviceExtension::KHR_timeline_semaphore,
    #endif
    #ifdef VK_EXT_subgroup_size_control
        EVulkanDeviceExtension::EXT_subgroup_size_control,
    #endif
    #ifdef VK_KHR_performance_query
        EVulkanDeviceExtension::KHR_performance_query,
    #endif
    #ifdef VK_KHR_spirv_1_4
        EVulkanDeviceExtension::KHR_spirv_1_4,
    #endif
    #ifdef VK_KHR_depth_stencil_resolve
        EVulkanDeviceExtension::KHR_depth_stencil_resolve,
    #endif
    #ifdef VK_KHR_buffer_device_address
        EVulkanDeviceExtension::KHR_buffer_device_address,
    #endif
    #ifdef VK_EXT_depth_range_unrestricted
        EVulkanDeviceExtension::EXT_depth_range_unrestricted,
    #endif
    #ifdef VK_EXT_sampler_filter_minmax
        EVulkanDeviceExtension::EXT_sampler_filter_minmax,
    #endif
    #ifdef VK_EXT_shader_stencil_export
        EVulkanDeviceExtension::EXT_shader_stencil_export,
    #endif
    // Vendor specific extensions
    #ifdef VK_NV_mesh_shader
        EVulkanDeviceExtension::NV_mesh_shader,
    #endif
    #ifdef VK_NV_shader_image_footprint
        EVulkanDeviceExtension::NV_shader_image_footprint,
    #endif
    #ifdef VK_NV_shading_rate_image
        EVulkanDeviceExtension::NV_shading_rate_image,
    #endif
    #ifdef VK_NV_fragment_shader_barycentric
        EVulkanDeviceExtension::NV_fragment_shader_barycentric,
    #endif
    #ifdef VK_NV_ray_tracing
        EVulkanDeviceExtension::NV_ray_tracing,
    #endif
    #ifdef VK_AMD_shader_info
        EVulkanDeviceExtension::AMD_shader_info,
    #endif
    #ifdef VK_AMD_shader_core_properties
        EVulkanDeviceExtension::AMD_shader_core_properties,
    #endif
    #ifdef VK_AMD_shader_core_properties2
        EVulkanDeviceExtension::AMD_shader_core_properties_2,
    #endif
    #ifdef VK_AMD_rasterization_order
        EVulkanDeviceExtension::AMD_rasterization_order,
    #endif
        });
    else if (EVulkanVersion::API_version_1_2 == version)
        result.Append({
    #ifdef VK_EXT_sample_locations
        EVulkanDeviceExtension::EXT_sample_locations,
    #endif
    #ifdef VK_KHR_push_descriptor
        EVulkanDeviceExtension::KHR_push_descriptor,
    #endif
    #ifdef VK_EXT_blend_operation_advanced
        EVulkanDeviceExtension::EXT_blend_operation_advanced,
    #endif
    #ifdef VK_KHR_maintenance1
        EVulkanDeviceExtension::KHR_maintenance1, // required for VK_EXT_inline_uniform_block
    #endif
    #ifdef VK_EXT_inline_uniform_block
        EVulkanDeviceExtension::EXT_inline_uniform_block,
    #endif
    #ifdef VK_EXT_memory_budget
        EVulkanDeviceExtension::EXT_memory_budget,
    #endif
    #ifdef VK_KHR_buffer_device_address
        EVulkanDeviceExtension::KHR_buffer_device_address,
    #endif
    #ifdef VK_EXT_subgroup_size_control
        EVulkanDeviceExtension::EXT_subgroup_size_control,
    #endif
    #ifdef VK_KHR_performance_query
        EVulkanDeviceExtension::KHR_performance_query,
    #endif
    #ifdef VK_EXT_depth_range_unrestricted
        EVulkanDeviceExtension::EXT_depth_range_unrestricted,
    #endif
    #ifdef VK_EXT_extended_dynamic_state
        EVulkanDeviceExtension::EXT_extended_dynamic_state,
    #endif
    #ifdef VK_EXT_shader_stencil_export
        EVulkanDeviceExtension::EXT_shader_stencil_export,
    #endif
    #ifdef VK_KHR_ray_tracing_pipeline
        //EVulkanDeviceExtension::KHR_ray_tracing_pipeline, #TODO: KHR to NV
    #endif
    // Vendor specific extensions
    #ifdef VK_NV_mesh_shader
        EVulkanDeviceExtension::NV_mesh_shader,
    #endif
    #ifdef VK_NV_shader_image_footprint
        EVulkanDeviceExtension::NV_shader_image_footprint,
    #endif
    #ifdef VK_NV_shading_rate_image
        EVulkanDeviceExtension::NV_shading_rate_image,
    #endif
    #ifdef VK_NV_fragment_shader_barycentric
        EVulkanDeviceExtension::NV_fragment_shader_barycentric,
    #endif
    #ifdef VK_NV_ray_tracing
        EVulkanDeviceExtension::NV_ray_tracing,
    #endif
    #ifdef VK_AMD_shader_info
        EVulkanDeviceExtension::AMD_shader_info,
    #endif
    #ifdef VK_AMD_shader_core_properties
        EVulkanDeviceExtension::AMD_shader_core_properties,
    #endif
    #ifdef VK_AMD_shader_core_properties2
        EVulkanDeviceExtension::AMD_shader_core_properties_2,
    #endif
    #ifdef VK_AMD_rasterization_order
        EVulkanDeviceExtension::AMD_rasterization_order,
    #endif
        });
    else
        AssertNotImplemented();

#if defined(VK_EXT_load_store_op_none) && VK_EXT_load_store_op_none
    // See EAttachmentLoadOp::Keep and EAttachementStoreOp::Keep
    result += EVulkanDeviceExtension::EXT_load_store_op_none;
#endif

    return vk::device_extensions_require(result);
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FVulkanInstance::FPhysicalDeviceInfo& info) {
    return WritePhysicalDeviceInfo_(*oss.FormatScope(), info);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FVulkanInstance::FPhysicalDeviceInfo& info) {
    return WritePhysicalDeviceInfo_(*oss.FormatScope(), info);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
