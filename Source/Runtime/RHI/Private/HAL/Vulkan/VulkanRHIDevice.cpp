#include "stdafx.h"

#ifdef RHI_VULKAN

#include "HAL/Vulkan/VulkanRHIDevice.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanRHIDevice::FVulkanRHIDevice(
     FVulkanRHIDeviceHandle deviceHandle,
     FVulkanRHIQueue graphicsQueue,
     FVulkanRHIQueue asyncComputeQueue,
     FVulkanRHIQueue transferQueue,
     FVulkanRHIQueue readbackQueue ) NOEXCEPT
:    _deviceHandle(deviceHandle)
,    _graphicsQueue(graphicsQueue)
,    _asyncComputeQueue(asyncComputeQueue)
,    _transferQueue(transferQueue)
,    _readbackQueue(readbackQueue) {
     Assert_NoAssume(VK_NULL_HANDLE != _deviceHandle);
     Assert_NoAssume( // at least one queue should be created !
          VK_NULL_HANDLE != _graphicsQueue ||
          VK_NULL_HANDLE != _asyncComputeQueue ||
          VK_NULL_HANDLE != _transferQueue ||
          VK_NULL_HANDLE != _readbackQueue );
     Assert_NoAssume(VK_NULL_HANDLE != _graphicsQueue ||
          (_graphicsQueue != _asyncComputeQueue && _graphicsQueue != _transferQueue && _graphicsQueue != _readbackQueue));
     Assert_NoAssume(VK_NULL_HANDLE != _asyncComputeQueue ||
          (_asyncComputeQueue != _graphicsQueue && _asyncComputeQueue != _transferQueue && _asyncComputeQueue != _readbackQueue));
     Assert_NoAssume(VK_NULL_HANDLE != _transferQueue ||
          (_transferQueue != _graphicsQueue && _transferQueue != _asyncComputeQueue && _transferQueue != _readbackQueue));
     Assert_NoAssume(VK_NULL_HANDLE != _readbackQueue ||
          (_readbackQueue != _asyncComputeQueue && _readbackQueue != _transferQueue && _readbackQueue != _graphicsQueue));
}
//----------------------------------------------------------------------------
FVulkanRHIDevice::~FVulkanRHIDevice() {
     // should have been destroyed by FVulkanRHIInstance::DestroyLogicalDevice() !
     Assert_NoAssume(nullptr == _deviceHandle);
     Assert_NoAssume(nullptr == _graphicsQueue);
     Assert_NoAssume(nullptr == _asyncComputeQueue);
     Assert_NoAssume(nullptr == _transferQueue);
     Assert_NoAssume(nullptr == _readbackQueue);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!RHI_VULKAN
