# Source/Runtime/RHI/Public/HAL/

## Responsibility
The HAL (Hardware Abstraction Layer) public folder declares the low-level hardware interface types used by the RHI and API encapsulators. This folder exposes hardware-dependent types such as platform memory addresses, queue family properties, and device feature flags that are consumed by both the Vulkan and DX11 backends. The HAL ensures that higher-level RHI code remains portable by encapsulating platform-specific details behind abstract types.

## Design
The HAL design separates platform-specific hardware information from API-agnostic rendering logic. Key types include platform memory addresses and handles, GPU device feature enums, and queue family property structures. These types are deliberately kept as data-only structures without behavior, allowing the RHI and API encapsulators to impose interpretation and validation logic. The HAL types are designed for passing by value or reference across module boundaries without ownership semantics, reflecting their role as descriptive metadata rather than managed objects.

## Flow
Engine code and API encapsulators query the HAL for device capabilities and hardware properties. The Vulkan encapsulator reads HAL types to initialize VkPhysicalDeviceFeatures and VkQueueFamilyProperties. The DX11 encapsulator maps equivalent concepts for the DirectX 11 backend. HAL types are passed by value across module boundaries in function parameters, ensuring zero-overhead propagation of hardware information. The fiber scheduler in Source/Runtime/Core/Private/Thread/Task/ queries HAL for thread affinity and CPU topology when scheduling rendering tasks.

## Integration
This folder integrates with Source/Extensions/RHIVulkan/ through the Vulkan encapsulator, which reads HAL types to configure VkPhysicalDeviceFeatures, VkQueueFamilyProperties, and memory properties. The RHI layer in Source/Runtime/RHI/ also consumes HAL types when creating device-dependent resources. HAL memory and topology information is used by the TMemoryPool allocator in Source/Runtime/Core/Public/Memory/ to make per-thread bucket caching decisions. The fiber scheduler in Source/Runtime/Core/Private/Thread/Task/ queries HAL for CPU core information when assigning rendering work to worker fibers.