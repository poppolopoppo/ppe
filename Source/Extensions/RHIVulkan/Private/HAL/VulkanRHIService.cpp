#include "stdafx.h"

#include "HAL/VulkanRHIService.h"

#include "Vulkan/Instance/VulkanFrameGraph.h"

#include "RHIModule.h"
#include "RHI/SwapchainDesc.h"

#include "Meta/Utility.h"
#include "Diagnostic/Logger.h"
#include "Memory/MemoryStream.h"
#include "Meta/Utility.h"
#include "Modular/ModularDomain.h"

#if USE_PPE_RHIDEBUG
namespace PPE::RHI {
extern void UnitTest_LocalBuffer();
extern void UnitTest_LocalImage();
}
#endif

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
CONSTEXPR const RHI::EPresentMode GVSyncPresentMode_{ RHI::EPresentMode::RelaxedFifo };
//----------------------------------------------------------------------------
CONSTEXPR const RHI::FSurfaceFormat GHDRSurfaceFormats_[] = {
    { RHI::EPixelFormat::RGB10_A2_UNorm, RHI::EColorSpace::HDR10_ST2084 },
    { RHI::EPixelFormat::RGB10_A2_UNorm, RHI::EColorSpace::HDR10_HLG },
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanRHIService::FVulkanRHIService(const FVulkanTargetRHI& vulkanRHI) NOEXCEPT
:   _vulkanRHI(vulkanRHI)
,   _currentFrame(UMax)
,   _elapsedTime(0)
{

}
//----------------------------------------------------------------------------
FVulkanRHIService::~FVulkanRHIService() {
    Assert_NoAssume(not _instance.Valid()); // must call TearDown() !
    Assert_NoAssume(VK_NULL_HANDLE  == _backBuffer);
}
//----------------------------------------------------------------------------
bool FVulkanRHIService::Construct(
    const FStringView& applicationName,
    const FRHIDeviceCreateInfo& deviceInfo,
    const FRHISurfaceCreateInfo* pOptionalWindow ) {
    Assert_NoAssume(Zero == _features);
    Assert_NoAssume(not _instance.Valid());
    Assert_NoAssume(VK_NULL_HANDLE == _backBuffer);

    using namespace RHI;

    RHI_LOG(Info, L"creating vulkan RHI service");
    RHI_LOG(Verbose, L"creating vulkan instance for rhi service");

    _elapsedTime = 0;

    const EVulkanVersion version = EVulkanVersion::API_version_latest;

    FVulkanDeviceExtensionSet requiredDeviceExtensions = FVulkanInstance::RequiredDeviceExtensions(version);
    FVulkanDeviceExtensionSet optionalDeviceExtensions = FVulkanInstance::RecommendedDeviceExtensions(version);

    FVulkanInstanceExtensionSet requiredInstanceExtensions = FVulkanInstance::RequiredInstanceExtensions(version);
    FVulkanInstanceExtensionSet optionalInstanceExtensions = FVulkanInstance::RecommendedInstanceExtensions(version);
    requiredInstanceExtensions |= FVulkanInstance::RequiredInstanceExtensions(version,
        pOptionalWindow ? pOptionalWindow->Hwnd : FWindowHandle{} );

    VECTORINSITU(RHIInstance, FConstChar, 8) instanceLayers;
    Append(instanceLayers, FVulkanInstance::RecommendedInstanceLayers(version));

    if (deviceInfo.Features & ERHIFeature::Debugging) {
        Append(instanceLayers, FVulkanInstance::DebuggingInstanceLayers(version));

        optionalInstanceExtensions |= FVulkanInstance::DebuggingInstanceExtensions(version);
        optionalDeviceExtensions |= FVulkanInstance::DebuggingDeviceExtensions(version);
    }
    if (deviceInfo.Features & ERHIFeature::Profiling) {
        Append(instanceLayers, FVulkanInstance::ProfilingInstanceLayers(version));

        optionalInstanceExtensions |= FVulkanInstance::ProfilingInstanceExtensions(version);
        optionalDeviceExtensions |= FVulkanInstance::ProfilingDeviceExtensions(version);
    }

    if (not _instance.Construct(
        *FString(applicationName),
        "PPE", version,
        instanceLayers,
        requiredInstanceExtensions,
            optionalInstanceExtensions,
        requiredDeviceExtensions,
            optionalDeviceExtensions )) {
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
        (deviceInfo.DeviceName.empty()
            ? _instance.PickHighPerformanceDevice()
            : _instance.PickPhysicalDeviceByName(deviceInfo.DeviceName) );
    LOG_CHECK(RHI, !!pPhysicalDevice);

    const TMemoryView<const FVulkanInstance::FQueueCreateInfo> deviceQueues =
        _instance.RecommendedDeviceQueues(version);

    _deviceInfo.Features = deviceInfo.Features;
    _deviceInfo.MaxStagingBufferMemory = deviceInfo.MaxStagingBufferMemory;
    _deviceInfo.StagingBufferSize = deviceInfo.StagingBufferSize;

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

        if (not CreateBackBufferSwapchain_(deviceInfo.Features, Default, *pOptionalWindow)) {
            RHI_LOG(Error, L"failed to create vulkan swapchain, abort!");
            return false;
        }
    }
    else {
        _features |= ERHIFeature::Headless;
    }

    if (pPhysicalDevice and pPhysicalDevice->IsDiscreteGPU())
        _features |= ERHIFeature::Discrete;

    if (!!_frameGraph->FindQueue(EQueueType::Graphics))
        _features |= ERHIFeature::Graphics + ERHIFeature::Compute;
    if (!!_frameGraph->FindQueue(EQueueType::AsyncCompute))
        _features |= ERHIFeature::AsyncCompute + ERHIFeature::Compute;

    const FVulkanDevice& device = _frameGraph->Device();

    if (device.Enabled().RayTracingNV)
        _features |= ERHIFeature::RayTracing;
    if (device.Enabled().MeshShaderNV)
        _features |= ERHIFeature::MeshDraw;
    if (device.Enabled().ImageFootprintNV)
        _features |= ERHIFeature::SamplerFeedback;
    if (device.Enabled().ShadingRateImageNV)
        _features |= ERHIFeature::VariableShadingRate;
    if (device.HasExtension(EVulkanDeviceExtension::EXT_conservative_rasterization))
        _features |= ERHIFeature::ConservativeDepth;

    if ((deviceInfo.Features & ERHIFeature::Debugging) &&
        device.AnyExtension(FVulkanInstance::DebuggingInstanceExtensions(version)) &&
        device.AnyExtension(FVulkanInstance::DebuggingDeviceExtensions(version)) )
        _features |= ERHIFeature::Debugging;

    if ((deviceInfo.Features & ERHIFeature::Profiling) &&
        device.AnyExtension(FVulkanInstance::ProfilingInstanceExtensions(version)) &&
        device.AnyExtension(FVulkanInstance::ProfilingDeviceExtensions(version)) )
        _features |= ERHIFeature::Profiling;

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
    _features = Zero;

    Assert_NoAssume(not _instance.Valid());
}
//----------------------------------------------------------------------------
RHI::SFrameGraph FVulkanRHIService::FrameGraph() const NOEXCEPT {
    return RHI::SFrameGraph{ _frameGraph };
}
//----------------------------------------------------------------------------
RHI::FWindowSurface FVulkanRHIService::BackBuffer() const NOEXCEPT {
    return RHI::FVulkanExternalObject{ _backBuffer }.WindowSurface();
}
//----------------------------------------------------------------------------
RHI::SPipelineCompiler FVulkanRHIService::Compiler(RHI::EShaderLangFormat lang) const NOEXCEPT {
    return FRHIModule::Get(FModularDomain::Get()).Compiler(lang);
}
//----------------------------------------------------------------------------
bool FVulkanRHIService::BeginFrame(FTimespan dt) {
    Assert(_frameGraph);

    using namespace RHI;

    _dt = dt;
    _elapsedTime += dt;

    const FFrameIndex previousFrame = _currentFrame;
    _currentFrame = _frameGraph->PrepareNewFrame();

    return (previousFrame != _currentFrame);
}
//----------------------------------------------------------------------------
void FVulkanRHIService::EndFrame() {
    using namespace RHI;

    _OnRenderFrame.Invoke(*this, _dt);

    LOG_CHECKVOID(RHI, _frameGraph->Flush(EQueueUsage::All));
    LOG_CHECKVOID(RHI, _frameGraph->WaitIdle(IFrameGraph::MaxTimeout));
}
//----------------------------------------------------------------------------
void FVulkanRHIService::ResizeWindow(const FRHISurfaceCreateInfo& surfaceInfo) {
    Assert(surfaceInfo.Hwnd);
    Assert(_backBuffer);

    using namespace RHI;

    if (surfaceInfo.Dimensions == uint2::Zero)
        return;

    RHI_LOG(Info, L"resizing window to ({0}, {1}) in vulkan service", surfaceInfo.Dimensions.x, surfaceInfo.Dimensions.y);

    _frameGraph->WaitIdle(IFrameGraph::MaxTimeout);

    if (not CreateBackBufferSwapchain_(_features, _swapchain.Release(), surfaceInfo))
        AssertReleaseMessage(L"failed to resize swapchain", !!_swapchain);

    _OnWindowResized.Invoke(*this, surfaceInfo);
}
//----------------------------------------------------------------------------
void FVulkanRHIService::DeviceLost() {
    using namespace RHI;

    if (_frameGraph) {
        RHI_LOG(Warning, L"device lost!");

        // #TODO: try to recreate the device
        _OnDeviceLost.Invoke(*this);
    }
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
bool FVulkanRHIService::CreateBackBufferSwapchain_(
    ERHIFeature features,
    RHI::FRawSwapchainID oldSwapchain,
    const FRHISurfaceCreateInfo& surfaceInfo ) {
    using namespace RHI;

    FSwapchainDesc swapchainDesc{};
    swapchainDesc.Surface = FVulkanExternalObject(_backBuffer).WindowSurface();
    swapchainDesc.Dimensions = surfaceInfo.Dimensions;

    if (surfaceInfo.EnableVSync)
        swapchainDesc.PresentModes.Push(GVSyncPresentMode_);

    if (features & ERHIFeature::HighDynamicRange)
        swapchainDesc.SurfaceFormats.Append(GHDRSurfaceFormats_);

    _swapchain = _frameGraph->CreateSwapchain(swapchainDesc, oldSwapchain ARGS_IF_RHIDEBUG("BackBuffer"));
    if (Unlikely(not _swapchain)) {
        RHI_LOG(Error, L"failed to create vulkan swapchain, abort!");
        return false;
    }

    // check actual surface format after construction:
    const auto& swapchainResource = _frameGraph->ResourceManager().ResourceData(_swapchain);

    if (swapchainResource.PresentMode() == VkCast(GVSyncPresentMode_))
        _features += ERHIFeature::VSync;
    else
        _features -= ERHIFeature::VSync;

    const FSurfaceFormat swapchainFormat{
        RHICast(swapchainResource.ColorFormat()),
        RHICast(swapchainResource.ColorSpace()),
    };
    if (MakeView(GHDRSurfaceFormats_).Contains(swapchainFormat))
        _features += ERHIFeature::HighDynamicRange;
    else
        _features -= ERHIFeature::HighDynamicRange;

    return true;
}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
void FVulkanRHIService::UnitTest() NOEXCEPT {
    IRHIService::UnitTest();

    RHI::UnitTest_LocalBuffer();
    RHI::UnitTest_LocalImage();
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
