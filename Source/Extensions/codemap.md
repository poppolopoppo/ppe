# Source/Extensions/

This folder aggregates codemap documentation for all extension modules under Source/Extensions/. It serves as the top-level entry point for navigating the extension architecture, providing an overview of each submodule's responsibility, design patterns, data flow, and integration points.

## Responsibility
This folder provides a top-level navigational index for all extension codemap files. It documents the structure and relationships between extension modules, enabling developers to understand the extension ecosystem without reading every individual file. It covers the RHIVulkan Vulkan backend extension and any other future extensions added to the repository.

## Design
The aggregator follows a hierarchical structure: Source/Extensions/codemap.md links to Source/Extensions/RHIVulkan/codemap.md (module root), which in turn links to its subfolders—Private/HAL, Public/Vulkan/Buffer, Public/Vulkan/Common, and Public/Vulkan/RenderPass. Each subfolder's codemap.md file provides consistent four-section documentation (Responsibility, Design, Flow, Integration). This modular approach allows teams to maintain independent documentation while providing unified entry points.

## Flow
Data flow begins at the top-level aggregator, which lists all extension modules. A developer investigating the rendering pipeline navigates from Source/Extensions/codemap.md to Source/Extensions/RHIVulkan/codemap.md for the module overview, then drills down into specific subfolders (e.g., Public/Vulkan/RenderPass for render pass logic, Public/Vulkan/Buffer for buffer management, Private/HAL for hardware abstraction). Each level provides Integration sections citing real Source/ paths for cross-referencing.

## Integration
This aggregator connects to Source/Extensions/RHIVulkan/codemap.md for the RHIVulkan module root. The RHIVulkan module integrates with Source/Runtime/RHI/ through FVulkanDeviceAPI, with memory allocation drawing from Source/Runtime/Core/Public/Memory/ and Source/Runtime/Core/Public/Allocator/. Presentation flows through FVulkanSwapchain. The task graph (TVulkanTaskGraph<_Visitor>) is consumed by FVulkanCommandBuffer, and descriptor set layouts (FVulkanDescriptorSetLayout) are consumed by FVulkanPipelineResources. Ray tracing shader tables (FVulkanRayTracingShaderTable) integrate with the pipeline system.