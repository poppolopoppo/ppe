#include "stdafx.h"

#ifdef RHI_VULKAN

#include "HAL/Vulkan/VulkanRHIDevice.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanDevice::FVulkanDevice(
     FVulkanDeviceHandle deviceHandle,
     FVulkanQueue graphicsQueue,
     FVulkanQueue presentQueue,
     FVulkanQueue asyncComputeQueue,
     FVulkanQueue transferQueue,
     FVulkanQueue readbackQueue ) NOEXCEPT
:    _deviceHandle(deviceHandle)
,    _graphicsQueue(graphicsQueue)
,    _presentQueue(presentQueue)
,    _asyncComputeQueue(asyncComputeQueue)
,    _transferQueue(transferQueue)
,    _readBackQueue(readbackQueue) {
     Assert_NoAssume(VK_NULL_HANDLE != _deviceHandle);
     Assert_NoAssume( // at least one queue should be created !
          VK_NULL_HANDLE != _graphicsQueue ||
          VK_NULL_HANDLE != _presentQueue ||
          VK_NULL_HANDLE != _asyncComputeQueue ||
          VK_NULL_HANDLE != _transferQueue ||
          VK_NULL_HANDLE != _readBackQueue );
     Assert_NoAssume(VK_NULL_HANDLE != _graphicsQueue || VK_NULL_HANDLE != _presentQueue);
}
//----------------------------------------------------------------------------
FVulkanDevice::~FVulkanDevice() {
     // should have been destroyed by FVulkanInstance::DestroyLogicalDevice() !
     Assert_NoAssume(nullptr == _deviceHandle);
     Assert_NoAssume(nullptr == _graphicsQueue);
     Assert_NoAssume(nullptr == _presentQueue);
     Assert_NoAssume(nullptr == _asyncComputeQueue);
     Assert_NoAssume(nullptr == _transferQueue);
     Assert_NoAssume(nullptr == _readBackQueue);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
