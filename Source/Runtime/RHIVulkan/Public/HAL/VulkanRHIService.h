﻿#pragma once

#include "RHIVulkan_fwd.h"

#include "VulkanTargetRHI.h"
#include "Vulkan/Instance/VulkanInstance.h"

#include "HAL/RHIService.h"
#include "Vulkan/Instance/VulkanDevice.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanRHIService final : public IRHIService {
public:
    FVulkanRHIService(
        const FVulkanTargetRHI& vulkanRHI,
        ERHIFeature features );
    virtual ~FVulkanRHIService();

    const RHI::FSwapchainID& Swapchain() const { return _swapchain; }

    NODISCARD bool Construct(
        const FStringView& applicationName,
        const FRHISurfaceCreateInfo* pOptionalWindow = nullptr,
        FStringView deviceName = Default);

public: // IRHIService
    virtual void TearDown() override;

    virtual const ITargetRHI& Target() const NOEXCEPT override { return _vulkanRHI; }
    virtual ERHIFeature Features() const NOEXCEPT override { return _features; }

    virtual RHI::SFrameGraph FrameGraph() const NOEXCEPT override;
    virtual RHI::FWindowSurface BackBuffer() const NOEXCEPT override { return RHI::FWindowSurface{ _backBuffer }; }

    virtual void ResizeWindow(const FRHISurfaceCreateInfo& window) override;
    virtual void ReleaseMemory() NOEXCEPT override;

private:
    const FVulkanTargetRHI& _vulkanRHI;
    const ERHIFeature _features;

    RHI::FVulkanInstance _instance;
    RHI::FVulkanDeviceInfo _deviceInfo;
    RHI::PVulkanFrameGraph _frameGraph;

    RHI::FSwapchainID _swapchain;
    VkSurfaceKHR _backBuffer{ VK_NULL_HANDLE };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE