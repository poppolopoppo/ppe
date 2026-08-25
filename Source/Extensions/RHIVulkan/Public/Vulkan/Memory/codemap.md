# Source/Extensions/RHIVulkan/Public/Vulkan/Memory/

## Responsibility
The Public Vulkan Memory folder exposes the API-visible Vulkan memory types and interfaces used by engine code. It declares the types that are safe to include in public headers for memory allocation and resource tracking. This folder ensures that engine code depends only on publicly documented Vulkan memory types, allowing the private implementation to evolve independently.

## Design
The public API provides Vulkan memory types such as VkMemoryAllocateInfo, VkMemoryRequirements, and VkSparseMemoryBind parameters. These types are deliberately kept as data-only structures without behavior, allowing the private Vulkan implementation to impose interpretation and validation logic. The design follows the principle that the public memory types should be a stable interface, allowing the private implementation to evolve its internal representations (such as TMemoryPool and TSlabHeap) without breaking engine code that depends on these types.

## Flow
Engine code uses the public memory types to describe VkMemoryAllocateInfo when allocating memory for images and buffers, and VkMemoryRequirements to query the memory requirements of created objects. The private implementation reads these parameters and performs the actual allocation using TMemoryPool or TSlabHeap. Memory type index selection is performed by the private implementation based on the VkMemoryProperties queried from the physical device. The public types are passed by value or const reference across module boundaries.

## Integration
This folder is consumed by the private memory subsystem (Private/Vulkan/Memory/) which implements the full memory allocation functionality using TMemoryPool and TSlabHeap. The frame graph subsystem (FVulkanFrameGraph) references public memory types when allocating memory for images and buffers used in TVulkanFrameTask<_Task> nodes. The image subsystem (FVulkanImage) uses public memory types when creating images and allocating memory. The pipeline resources subsystem (FVulkanPipelineResources) uses public memory types when binding resources to pipelines. The swapchain subsystem (FVulkanSwapchain) uses public memory types for swapchain image allocation.