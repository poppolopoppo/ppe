# Source/Extensions/RHIVulkan/Private/Vulkan/Debugger/

## Responsibility
The Private Vulkan Debugger folder contains the implementation of Vulkan debug utilities, validation layers, and diagnostic tools for the rendering pipeline. This folder implements the debug messenger setup, validation callback registration, and diagnostic data collection that are used to debug rendering issues and validate Vulkan API usage.

## Design
The design implements Vulkan debug messenger setup with VkDebugUtilsMessengerCreateInfoEXT, registering a debug callback function that receives validation messages from the Vulkan API. The debug callback logs messages to the application's logging system, categorizing them by severity (Verbose, Info, Warning, Error) and type (General, Validation, Performance). Additional diagnostic data includes pipeline state tracking, resource lifetime monitoring, and command buffer validation. All debug utilities are conditionalized behind preprocessor macros (PPE_VULKAN_ENABLE_DEBUG), ensuring that debug code is excluded from production builds.

## Flow
Debug messenger setup is called during device initialization, registering the debug callback with the Vulkan API. During command buffer recording, validation messages may be emitted if API usage violations are detected. The debug callback is invoked synchronously or asynchronously, depending on the Vulkan implementation. Diagnostic data is collected at the end of each frame, including resource lifetime information, pipeline state snapshots, and command buffer statistics. This data is logged to the application's logging system for debugging purposes. At the end of the frame, all debug data is flushed and the callback is unregistered.

## Integration
This folder integrates with the command processing subsystem (Private/Vulkan/Command/) which may emit validation messages during command buffer recording. It integrates with the memory subsystem (Private/Vulkan/Memory/) for resource lifetime monitoring. The frame graph subsystem (FVulkanFrameGraph) uses debug utilities to track pipeline state transitions and resource layout changes. The swapchain subsystem (FVulkanSwapchain) uses debug utilities to validate presentation operations. The ray tracing subsystem (FVulkanRayTracingShaderTable) uses debug utilities to validate ray tracing pipeline creation and command execution.