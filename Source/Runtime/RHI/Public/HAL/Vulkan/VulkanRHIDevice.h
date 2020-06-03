#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#include "HAL/Generic/GenericRHIDevice.h"

#include "Container/Vector.h"
#include "Meta/StronglyTyped.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FVulkanRHIQueue);
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FVulkanRHIDeviceHandle);
//----------------------------------------------------------------------------
class PPE_RHI_API FVulkanRHIDevice : public FGenericRHIDevice {
protected:
    friend struct FVulkanRHIInstance;

    FVulkanRHIDevice(
        FVulkanRHIDeviceHandle deviceHandle,
        FVulkanRHIQueue graphicsQueue,
        FVulkanRHIQueue asyncComputeQueue,
        FVulkanRHIQueue transferQueue,
        FVulkanRHIQueue readbackQueue ) NOEXCEPT;

public: // must be implemented:
    virtual ~FVulkanRHIDevice();

private:
    FVulkanRHIDeviceHandle _deviceHandle;

    FVulkanRHIQueue _graphicsQueue;
    FVulkanRHIQueue _asyncComputeQueue;
    FVulkanRHIQueue _transferQueue;
    FVulkanRHIQueue _readbackQueue;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
