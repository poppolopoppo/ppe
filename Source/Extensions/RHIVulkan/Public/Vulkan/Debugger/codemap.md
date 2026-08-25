# Source/Extensions/RHIVulkan/Public/Vulkan/Debugger/

## Responsibility
The Public Vulkan Debugger folder exposes the API-visible Vulkan debug types and interfaces used by engine code. It declares the types that are safe to include in public headers for debug messenger setup, validation callback registration, and diagnostic data collection. This folder ensures that engine code can register Vulkan debug utilities without depending on private implementation details.

## Design
The public API provides a stable interface for Vulkan debug messenger setup, declaring the VkDebugUtilsMessengerCreateInfoEXT structure and the debug callback function signature. Engine code can register the debug messenger during device creation without needing to include private Vulkan headers. The debug callback signature and message categories/types are documented for engine code that needs to process validation messages. All public debug types are designed to be stable across Vulkan API versions, allowing the engine to remain compatible across API updates.

## Flow
Engine code includes the public debug types and calls vkCreateDebugUtilsMessengerEXT during device initialization, registering the debug callback. The Vulkan API subsequently calls the registered callback whenever a validation message is generated. The engine processes the message by category (General, Validation, Performance) and severity (Verbose, Info, Warning, Error), logging to the application's logging system. During command buffer recording, validation messages may be emitted if API usage violations are detected. At the end of the debug session, the engine calls vkDestroyDebugUtilsMessengerEXT to unregister the callback.

## Integration
This folder is consumed by the private debugger subsystem (Private/Vulkan/Debugger/) which implements the full debug functionality. The public types are included by engine code that needs to register debug utilities without depending on private details. The frame graph subsystem (FVulkanFrameGraph) may reference public debug types for pipeline state tracking. The swapchain subsystem (FVulkanSwapchain) may use public debug types to validate presentation operations. The ray tracing subsystem (FVulkanRayTracingShaderTable) may use public debug types to validate ray tracing pipeline creation.