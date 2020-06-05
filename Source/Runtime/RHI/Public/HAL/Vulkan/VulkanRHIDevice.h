#pragma once

#include "VulkanRHIDevice.h"
#include "HAL/Vulkan/VulkanRHI_fwd.h"

#include "HAL/Generic/GenericRHIDevice.h"

#include "Container/Vector.h"
#include "Meta/StronglyTyped.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FVulkanQueue);
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FVulkanDeviceHandle);
//----------------------------------------------------------------------------
class PPE_RHI_API FVulkanDevice : public FGenericDevice {
protected:
    friend struct FVulkanInstance;

    FVulkanDevice(
        FVulkanDeviceHandle deviceHandle,
        FVulkanQueue graphicsQueue,
        FVulkanQueue presentQueue,
        FVulkanQueue asyncComputeQueue,
        FVulkanQueue transferQueue,
        FVulkanQueue readBackQueue ) NOEXCEPT;

    FVulkanQueue GraphicsQueue() const NOEXCEPT { return _graphicsQueue; }
    FVulkanQueue PresentQueue() const NOEXCEPT { return _presentQueue; }
    FVulkanQueue AsyncComputeQueue() const NOEXCEPT { return _asyncComputeQueue; }
    FVulkanQueue TransferQueue() const NOEXCEPT { return _transferQueue; }
    FVulkanQueue ReadBackQueue() const NOEXCEPT { return _readBackQueue; }

public: // must be implemented:
    virtual ~FVulkanDevice();

private:
    FVulkanDeviceHandle _deviceHandle;

    FVulkanQueue _graphicsQueue;
    FVulkanQueue _presentQueue;
    FVulkanQueue _asyncComputeQueue;
    FVulkanQueue _transferQueue;
    FVulkanQueue _readBackQueue;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
