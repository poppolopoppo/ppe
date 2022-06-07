#pragma once

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
    explicit FVulkanRHIService(const FVulkanTargetRHI& vulkanRHI) NOEXCEPT;
    virtual ~FVulkanRHIService();

    NODISCARD bool Construct(
        const FStringView& applicationName,
        const FRHIDeviceCreateInfo& deviceInfo,
        const FRHISurfaceCreateInfo* pOptionalWindow = nullptr);

public: // IRHIService
    virtual void TearDown() override;

    virtual const ITargetRHI& Target() const NOEXCEPT override { return _vulkanRHI; }
    virtual ERHIFeature Features() const NOEXCEPT override { return _features; }

    virtual RHI::SFrameGraph FrameGraph() const NOEXCEPT override;
    virtual RHI::FWindowSurface BackBuffer() const NOEXCEPT override;
    virtual const RHI::FSwapchainID& Swapchain() const NOEXCEPT override { return _swapchain; }

    virtual RHI::FFrameIndex CurrentFrame() const NOEXCEPT override { return _currentFrame; }
    virtual FTimespan ElapsedTime() const NOEXCEPT override { return _elspasedTime; }

    virtual RHI::SPipelineCompiler Compiler(RHI::EShaderLangFormat lang) const NOEXCEPT override;

    virtual void RenderFrame(FTimespan dt) override;
    virtual void ResizeWindow(const FRHISurfaceCreateInfo& window) override;

    virtual void DeviceLost() override;
    virtual void ReleaseMemory() NOEXCEPT override;

#if USE_PPE_RHIDEBUG
    virtual void UnitTest() NOEXCEPT override;
#endif

private:
    NODISCARD bool CreateBackBufferSwapchain_(
        ERHIFeature features,
        RHI::FRawSwapchainID oldSwapchain,
        const FRHISurfaceCreateInfo& surfaceInfo );

    const FVulkanTargetRHI& _vulkanRHI;

    ERHIFeature _features{ Zero };
    RHI::FVulkanInstance _instance;
    RHI::FVulkanDeviceInfo _deviceInfo;
    RHI::PVulkanFrameGraph _frameGraph;

    RHI::FFrameIndex _currentFrame;
    FTimespan _elspasedTime;

    RHI::FSwapchainID _swapchain;
    VkSurfaceKHR _backBuffer{ VK_NULL_HANDLE };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
