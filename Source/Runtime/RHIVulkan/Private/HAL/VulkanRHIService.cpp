#include "stdafx.h"

#include "HAL/VulkanRHIService.h"
#include "Vulkan/Instance/VulkanFrameGraph.h"

#include "Diagnostic/Logger.h"
#include "Memory/MemoryStream.h"
#include "Meta/Utility.h"
#include "RHI/SwapchainDesc.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void MakeSwapchainDesc_(
    RHI::FSwapchainDesc* pSwapchainDesc,
    VkSurfaceKHR backBuffer,
    const ERHIFeature features,
    const FRHISurfaceCreateInfo& surfaceInfo ) NOEXCEPT {
    Assert(pSwapchainDesc);
    Assert(VK_NULL_HANDLE != backBuffer);

    using namespace RHI;

    pSwapchainDesc->Surface = FWindowSurface{ backBuffer };
    pSwapchainDesc->Dimensions = surfaceInfo.Dimensions;

    if (surfaceInfo.EnableVSync) {
        pSwapchainDesc->PresentModes.Push(EPresentMode::RelaxedFifo);
    }

    if (features & ERHIFeature::HighDynamicRange) {
        pSwapchainDesc->SurfaceFormats.Push(
            EPixelFormat::RGB10_A2_UNorm,
            EColorSpace::HDR10_ST2084 );
        pSwapchainDesc->SurfaceFormats.Push(
            EPixelFormat::RGB10_A2_UNorm,
            EColorSpace::HDR10_HLG );
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanRHIService::FVulkanRHIService(const FVulkanTargetRHI& vulkanRHI, ERHIFeature features)
:   _vulkanRHI(vulkanRHI)
,   _features(features)
{

}
//----------------------------------------------------------------------------
FVulkanRHIService::~FVulkanRHIService() {
    Assert_NoAssume(not _instance.Valid()); // must call TearDown() !
    Assert_NoAssume(VK_NULL_HANDLE  == _backBuffer);
}
//----------------------------------------------------------------------------
bool FVulkanRHIService::Construct(const FStringView& applicationName, const FRHISurfaceCreateInfo* pOptionalWindow, FStringView deviceName) {
    Assert_NoAssume(not _instance.Valid());
    Assert_NoAssume(VK_NULL_HANDLE == _backBuffer);

    using namespace RHI;

    RHI_LOG(Info, L"creating vulkan RHI service");
    RHI_LOG(Verbose, L"creating vulkan instance for rhi service");

    const EVulkanVersion version = EVulkanVersion::API_version_latest;

    FVulkanDeviceExtensionSet requiredDeviceExtensions = FVulkanInstance::RequiredDeviceExtensions(version);
    FVulkanDeviceExtensionSet optionalDeviceExtensions = FVulkanInstance::RecommendedDeviceExtensions(version);

    FVulkanInstanceExtensionSet requiredInstanceExtensions = FVulkanInstance::RequiredInstanceExtensions(version);
    FVulkanInstanceExtensionSet optionalInstanceExtensions = FVulkanInstance::RecommendedInstanceExtensions(version);
    requiredInstanceExtensions |= FVulkanInstance::RequiredInstanceExtensions(version,
        pOptionalWindow ? pOptionalWindow->Hwnd : FWindowHandle{} );

    if (not _instance.Construct(
        *FString(applicationName),
        "PPE", version,
        FVulkanInstance::RecommendedInstanceLayers(version),
        requiredInstanceExtensions, optionalInstanceExtensions,
        requiredDeviceExtensions, optionalDeviceExtensions )) {
        RHI_LOG(Error, L"failed to create vulkan instance, abort!");
        return false;
    }

    bool success = false;
    ON_SCOPE_EXIT([&success, this]() {
        if (not success) // we cleanup the service if something failed
            TearDown();
    });

    if (pOptionalWindow) {
        RHI_LOG(Verbose, L"creating vulkan back-buffer surface for given window");

        AssertRelease(pOptionalWindow->Hwnd);
        if (not _instance.CreateSurface(&_backBuffer, pOptionalWindow->Hwnd)) {
            RHI_LOG(Error, L"failed to create vulkan window surface, abort!");
            return false;
        }

        Assert(_backBuffer);
    }

    RHI_LOG(Verbose, L"creating vulkan device for service");

    const FVulkanInstance::FPhysicalDeviceInfo* const pPhysicalDevice =
        (deviceName.empty()
            ? _instance.PickHighPerformanceDevice()
            : _instance.PickPhysicalDeviceByName(deviceName) );

    const TMemoryView<const FVulkanInstance::FQueueCreateInfo> deviceQueues =
        _instance.RecommendedDeviceQueues(version);

    if (not _instance.CreateDevice(
        &_deviceInfo, _backBuffer,
        requiredDeviceExtensions,
        optionalDeviceExtensions,
        pPhysicalDevice, deviceQueues )) {
        RHI_LOG(Error, L"failed to create vulkan device, abort!");
        return false;
    }

    RHI_LOG(Verbose, L"creating vulkan frame graph for service");

    _frameGraph = NEW_REF(RHIVulkan, FVulkanFrameGraph, _deviceInfo);
    if (not _frameGraph->Construct()) {
        RHI_LOG(Error, L"failed to create vulkan framegraph, abort!");
        RemoveRef_AssertReachZero(_frameGraph);
        return false;
    }

    if (_backBuffer) {
        Assert(pOptionalWindow);
        RHI_LOG(Verbose, L"creating vulkan swapchain for service");

        FSwapchainDesc swapchainDesc{};
        MakeSwapchainDesc_(&swapchainDesc, _backBuffer, _features, *pOptionalWindow);

        _swapchain = _frameGraph->CreateSwapchain(swapchainDesc, Default ARGS_IF_RHIDEBUG("BackBuffer"));

        if (not _swapchain) {
            RHI_LOG(Error, L"failed to create vulkan swapchain, abort!");
            return false;
        }
    }

    Assert_NoAssume(_instance.Valid());
    success = true; // assign true to success to avoid TearDown()
    return true;
}
//----------------------------------------------------------------------------
void FVulkanRHIService::TearDown() {
    Assert_NoAssume(_instance.Valid());

    using namespace RHI;

    RHI_LOG(Info, L"destroying vulkan RHI service");

    if (_swapchain.Valid()) {
        AssertRelease(_frameGraph);
        RHI_LOG(Verbose, L"destroying vulkan swapchain for rhi service");

        if (not _frameGraph->ReleaseResource(_swapchain))
            RHI_LOG(Error, L"failed to destroy vulkan swapchain for rhi service");
    }

    if (_frameGraph) {
        RHI_LOG(Verbose, L"destroying vulkan frame graph for rhi service");

        _frameGraph->TearDown();
        RemoveRef_AssertReachZero(_frameGraph);
    }

    if (_deviceInfo.vkDevice) {
        RHI_LOG(Verbose, L"destroying vulkan device for rhi service");

        _instance.DestroyDevice(&_deviceInfo);
    }

    if (_backBuffer) {
        RHI_LOG(Verbose, L"destroying vulkan back-buffer for rhi service");

        _instance.DestroySurface(_backBuffer);
        _backBuffer = VK_NULL_HANDLE;
    }

    RHI_LOG(Verbose, L"destroying vulkan instance for rhi service");

    _instance.TearDown();

    Assert_NoAssume(not _instance.Valid());
}
//----------------------------------------------------------------------------
RHI::SFrameGraph FVulkanRHIService::FrameGraph() const NOEXCEPT {
    return RHI::SFrameGraph{ _frameGraph };
}
//----------------------------------------------------------------------------
void FVulkanRHIService::ResizeWindow(const FRHISurfaceCreateInfo& surfaceInfo) {
    Assert(surfaceInfo.Hwnd);
    Assert(_backBuffer);

    if (surfaceInfo.Dimensions == uint2::Zero)
        return;

    using namespace RHI;
    RHI_LOG(Info, L"resizing window to ({0}, {1}) in vulkan service", surfaceInfo.Dimensions.x, surfaceInfo.Dimensions.y);

    Verify( _frameGraph->WaitIdle() );

    FSwapchainDesc swapchainDesc;
    MakeSwapchainDesc_(&swapchainDesc, _backBuffer, _features, surfaceInfo);

    _swapchain = _frameGraph->CreateSwapchain(swapchainDesc, _swapchain.Release() ARGS_IF_RHIDEBUG("BackBuffer"));

    AssertReleaseMessage(L"failed to resize swapchain", !!_swapchain);
}
//----------------------------------------------------------------------------
void FVulkanRHIService::ReleaseMemory() NOEXCEPT {
    Assert_NoAssume(_instance.Valid());

    using namespace RHI;
    RHI_LOG(Info, L"releasing memory in vulkan service");

    if (_frameGraph)
        _frameGraph->ReleaseMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
