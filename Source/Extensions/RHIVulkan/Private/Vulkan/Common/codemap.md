# Source/Extensions/RHIVulkan/Private/Vulkan/Common/

## Responsibility
The Private Vulkan Common folder contains utility types, macros, and shared implementations used across the Vulkan subsystem. This includes Vulkan version checking, feature enumeration, debug utilities, and common helper functions that are referenced by multiple subsystems (command, descriptors, memory, pipeline, ray tracing, image).

## Design
The design provides a set of utility types and macros that abstract away Vulkan version and feature checks. VkVersion comparison macros ensure consistent version checking across the codebase. Feature enumeration types abstract the VK_PHYSICAL_DEVICE_FEATURES_2 structure, providing a portable way to query physical device capabilities. Debug utilities include the VkDebugUtilsMessengerCreateInfoEXT structure and helper functions for setting up Vulkan debug messengers. Common helper functions include VkResult checking, memory size alignment, and type conversions between engine types and Vulkan types. All utilities are designed to be header-only, with no external dependencies beyond the Vulkan API.

## Flow
Utility types and macros are included by subsystems throughout the Vulkan codebase. Version checking macros are used during physical device initialization to ensure the required Vulkan version is available. Feature enumeration is queried during device creation and is consumed by the memory allocator and the render pass creation code. Debug messenger setup is called during device initialization, and message callbacks are invoked during Vulkan object creation/destruction. Helper functions are called throughout the rendering pipeline for common operations such as alignment, result checking, and type conversion.

## Integration
This folder is included by all other Vulkan subsystems (Command/, Descriptors/, Memory/, Pipeline/, RayTracing/, Image/). The version checking macros are used during physical device initialization, which is implemented in Source/Extensions/RHIVulkan/Private/Vulkan/Instance/. Feature enumeration is consumed by the memory allocator (TMemoryPool, TSlabHeap) and the render pass creation code (Private/Vulkan/RenderPass/). Debug messenger setup is called during device initialization across the Vulkan codebase. Helper functions are called throughout the rendering pipeline for common operations.