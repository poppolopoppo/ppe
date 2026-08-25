# Source/Extensions/RHIVulkan/Public/Vulkan/Image/

## Responsibility
The Public Vulkan Image folder exposes the API-visible Vulkan image types and interfaces used by engine code. It declares the types that are safe to include in public headers for image creation, memory allocation, and resource management. This folder ensures that engine code depends only on publicly documented Vulkan image types, allowing the private implementation to evolve independently.

## Design
The public API provides Vulkan image types such as VkImageCreateInfo parameters (format, extent, usage, tiling), VkImageViewCreateInfo for image view creation, and related memory allocation parameters. These types are deliberately kept as data-only structures without behavior, allowing the private Vulkan implementation to impose interpretation and validation logic. The design follows the principle that the public image types should be a stable interface, allowing the private implementation to evolve its internal representations without breaking engine code that depends on these types.

## Flow
Engine code uses the public image types to describe VkImageCreateInfo when creating images, and VkImageViewCreateInfo when creating image views. The private implementation reads these parameters and creates the corresponding Vulkan objects. Image views are used for sampling (shader-readable) or as render target attachments. The public types are passed by value or const reference across module boundaries, ensuring zero-overhead propagation of image configuration.

## Integration
This folder is consumed by the private image subsystem (Private/Vulkan/Image/) which implements the full image creation and management functionality. The frame graph subsystem (FVulkanFrameGraph) references public image types when passing images as inputs and outputs to TVulkanFrameTask<_Task> nodes. The descriptor set subsystem (FVulkanDescriptorSetLayout) binds image samplers and storage buffers using the public types. The pipeline resources subsystem (FVulkanPipelineResources) binds images as textures or storage buffers to pipelines. The swapchain subsystem (FVulkanSwapchain) creates the swapchain images using the public type parameters.