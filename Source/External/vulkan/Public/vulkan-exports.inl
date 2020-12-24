//----------------------------------------------------------------------------
// Functions that must be manually linked with system API
//----------------------------------------------------------------------------
#ifndef VK_EXPORTED_FUNCTION
#  define VK_EXPORTED_FUNCTION( function )
#endif
//----------------------------------------------------------------------------
VK_EXPORTED_FUNCTION( vkGetInstanceProcAddr )
//----------------------------------------------------------------------------
#undef VK_EXPORTED_FUNCTION
//----------------------------------------------------------------------------
// Global Vulkan functions
//----------------------------------------------------------------------------
#ifndef VK_GLOBAL_LEVEL_FUNCTION
#   define VK_GLOBAL_LEVEL_FUNCTION( function )
#endif
//----------------------------------------------------------------------------
VK_GLOBAL_LEVEL_FUNCTION( vkCreateInstance )
VK_GLOBAL_LEVEL_FUNCTION( vkEnumerateInstanceExtensionProperties )
VK_GLOBAL_LEVEL_FUNCTION( vkEnumerateInstanceLayerProperties )
//----------------------------------------------------------------------------
#undef VK_GLOBAL_LEVEL_FUNCTION
//----------------------------------------------------------------------------
// Instance functions
//----------------------------------------------------------------------------
#ifndef VK_INSTANCE_LEVEL_FUNCTION
#   define VK_INSTANCE_LEVEL_FUNCTION( function )
#endif
//----------------------------------------------------------------------------
VK_INSTANCE_LEVEL_FUNCTION( vkCreateDevice )
VK_INSTANCE_LEVEL_FUNCTION( vkDestroyInstance )
VK_INSTANCE_LEVEL_FUNCTION( vkEnumerateDeviceExtensionProperties )
VK_INSTANCE_LEVEL_FUNCTION( vkEnumerateDeviceLayerProperties )
VK_INSTANCE_LEVEL_FUNCTION( vkEnumeratePhysicalDevices )
VK_INSTANCE_LEVEL_FUNCTION( vkGetDeviceProcAddr )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceImageFormatProperties )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceFeatures )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceFormatProperties )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceMemoryProperties )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceProperties )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceQueueFamilyProperties )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceSparseImageFormatProperties )
//----------------------------------------------------------------------------
#undef VK_INSTANCE_LEVEL_FUNCTION
//----------------------------------------------------------------------------
// Instance extension functions
//----------------------------------------------------------------------------
#ifndef VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION
#   define VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( function, extension )
#endif
//----------------------------------------------------------------------------
VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( vkCreateDebugUtilsMessengerEXT, VK_EXT_DEBUG_UTILS_EXTENSION_NAME )
VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( vkDestroyDebugUtilsMessengerEXT, VK_EXT_DEBUG_UTILS_EXTENSION_NAME )
VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( vkDestroySurfaceKHR, VK_KHR_SURFACE_EXTENSION_NAME )
VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( vkGetPhysicalDeviceSurfaceCapabilitiesKHR, VK_KHR_SURFACE_EXTENSION_NAME )
VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( vkGetPhysicalDeviceSurfaceFormatsKHR, VK_KHR_SURFACE_EXTENSION_NAME )
VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( vkGetPhysicalDeviceSurfacePresentModesKHR, VK_KHR_SURFACE_EXTENSION_NAME )
VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( vkGetPhysicalDeviceSurfaceSupportKHR, VK_KHR_SURFACE_EXTENSION_NAME)
#if defined(VK_USE_PLATFORM_WIN32_KHR)
VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( vkCreateWin32SurfaceKHR, VK_KHR_WIN32_SURFACE_EXTENSION_NAME )
VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( vkGetPhysicalDeviceWin32PresentationSupportKHR, VK_KHR_WIN32_SURFACE_EXTENSION_NAME )
#elif defined(VK_USE_PLATFORM_XCB_KHR)
VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( vkCreateXcbSurfaceKHR, VK_KHR_XCB_SURFACE_EXTENSION_NAME )
VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( vkGetPhysicalDeviceXcbPresentationSupportKHR, VK_KHR_XCB_SURFACE_EXTENSION_NAME )
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( vkCreateXlibSurfaceKHR, VK_KHR_XLIB_SURFACE_EXTENSION_NAME )
VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION( vkGetPhysicalDeviceXlibPresentationSupportKHR, VK_KHR_XLIB_SURFACE_EXTENSION_NAME )
#endif
//----------------------------------------------------------------------------
#undef VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION
//----------------------------------------------------------------------------
// Device functions
//----------------------------------------------------------------------------
#ifndef VK_DEVICE_LEVEL_FUNCTION
#   define VK_DEVICE_LEVEL_FUNCTION( function )
#endif
//----------------------------------------------------------------------------
VK_DEVICE_LEVEL_FUNCTION( vkAllocateMemory )
VK_DEVICE_LEVEL_FUNCTION( vkCreateDescriptorSetLayout )
VK_DEVICE_LEVEL_FUNCTION( vkCreateImageView )
VK_DEVICE_LEVEL_FUNCTION( vkCreatePipelineLayout )
VK_DEVICE_LEVEL_FUNCTION( vkCreateShaderModule )
VK_DEVICE_LEVEL_FUNCTION( vkCreateSwapchainKHR )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyDescriptorSetLayout )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyDevice )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyImageView )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyPipelineLayout )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyShaderModule )
VK_DEVICE_LEVEL_FUNCTION( vkDestroySwapchainKHR )
VK_DEVICE_LEVEL_FUNCTION( vkDeviceWaitIdle )
VK_DEVICE_LEVEL_FUNCTION( vkFreeMemory )
VK_DEVICE_LEVEL_FUNCTION( vkGetDeviceQueue )
VK_DEVICE_LEVEL_FUNCTION( vkGetSwapchainImagesKHR )
VK_DEVICE_LEVEL_FUNCTION( vkMapMemory )
VK_DEVICE_LEVEL_FUNCTION( vkUnmapMemory )
//----------------------------------------------------------------------------
#undef VK_DEVICE_LEVEL_FUNCTION
//----------------------------------------------------------------------------
// Device extension functions
//----------------------------------------------------------------------------
#ifndef VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION
#   define VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION( function, extension )
#endif
//----------------------------------------------------------------------------
VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION( vkDebugMarkerSetObjectTagEXT, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)
VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION( vkDebugMarkerSetObjectNameEXT, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)
VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION( vkCmdDebugMarkerBeginEXT, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)
VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION( vkCmdDebugMarkerEndEXT, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)
VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION( vkCmdDebugMarkerInsertEXT, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)
//----------------------------------------------------------------------------
#undef VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
