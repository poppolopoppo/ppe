#include "stdafx.h"

#include "Service/RHIService.h"

#include "Window/WindowBase.h"
#include "Window/WindowRHI.h"

#include "RHIModule.h"
#include "HAL/RHIDevice.h"
#include "HAL/RHIInstance.h"
#include "HAL/TargetRHI.h"

#include "Modular/ModularDomain.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FDefaultRHIService_ final : public IRHIService {
public:
    FDefaultRHIService_();
    virtual ~FDefaultRHIService_();

    RHI::FInstance& Instance() { return _instance; }
    const RHI::FInstance& Instance() const { return _instance; }

    virtual RHI::FDevice* CreateMainDevice(FWindowRHI* window) override final;
    virtual void DestroyMainDevice(FWindowRHI* window, RHI::FDevice* device) override final;

    virtual RHI::FDevice* CreateHeadlessDevice(bool computeOnly) override final;
    virtual void DestroyHeadlessDevice(RHI::FDevice* device) override final;

    virtual RHI::FDevice* MainDevice() const NOEXCEPT override final {
        return _mainDevice;
    }

    virtual void SetMainDevice(RHI::FDevice* device) override final {
        _mainDevice = device;
    }

private:
    RHI::FInstance _instance;
    RHI::FDevice* _mainDevice{ nullptr };
};
//----------------------------------------------------------------------------
FDefaultRHIService_::FDefaultRHIService_() {
    // #TODO : handle API attach failure properly (need indirect dynamic linking see IRHIService::Make)
    VerifyRelease(RHI::FInstance::Create(&_instance));
}
//----------------------------------------------------------------------------
FDefaultRHIService_::~FDefaultRHIService_() {
    AssertRelease(nullptr == _mainDevice);
    RHI::FInstance::Destroy(&_instance);
}
//----------------------------------------------------------------------------
RHI::FDevice* FDefaultRHIService_::CreateMainDevice(FWindowRHI* window) {
    Assert(window);

    const RHI::FWindowHandle hwnd{ window->Handle() };
    const RHI::FWindowSurface surface = _instance.CreateWindowSurface(hwnd);

    RHI::FDevice* const device = _instance.CreateLogicalDevice(
        RHI::EPhysicalDeviceFlags::Default, surface );

    const TMemoryView<const RHI::EPresentMode> presentModes = device->PresentModes();
    Assert_NoAssume(Contains(presentModes, RHI::EPresentMode::Fifo));

    RHI::EPresentMode const present = (
        Contains(presentModes, RHI::EPresentMode::Mailbox)
            ? RHI::EPresentMode::Mailbox
            : (Contains(presentModes, RHI::EPresentMode::RelaxedFifo)
                ? RHI::EPresentMode::RelaxedFifo
                : RHI::EPresentMode::Fifo) );

    const TMemoryView<const RHI::FSurfaceFormat> surfaceFormats = device->SurfaceFormats();
    const size_t bestFormat = RHI::FSurfaceFormat::BestAvailable(surfaceFormats);
    Assert_NoAssume(INDEX_NONE != bestFormat);

    device->CreateSwapChain(surface, present, surfaceFormats[bestFormat]);
    window->SetSurfaceRHI(surface);

    return device;
}
//----------------------------------------------------------------------------
void FDefaultRHIService_::DestroyMainDevice(FWindowRHI* window, RHI::FDevice* device) {
    Assert(window);
    Assert(device);

    const RHI::FWindowSurface surface = window->SurfaceRHI();
    Assert_NoAssume(surface);

    window->SetSurfaceRHI(RHI::FWindowSurface{0});

    device->DestroySwapChain();

    _instance.DestroyLogicalDevice(device);
    _instance.DestroyWindowSurface(surface);
}
//----------------------------------------------------------------------------
RHI::FDevice* FDefaultRHIService_::CreateHeadlessDevice(bool computeOnly) {
    auto deviceFlags{ RHI::EPhysicalDeviceFlags::Compute };
    if (not computeOnly)
        deviceFlags = deviceFlags | RHI::EPhysicalDeviceFlags::Graphics;

    return _instance.CreateLogicalDevice(deviceFlags, RHI::FWindowSurface{0});
}
//----------------------------------------------------------------------------
void FDefaultRHIService_::DestroyHeadlessDevice(RHI::FDevice* device) {
    Assert(device);

    _instance.DestroyLogicalDevice(device);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void IRHIService::Make(URHIService* pRHI, RHI::ETargetRHI rhi) {
    Assert(pRHI);

    // #TODO: separate RHI implementation in different modules, and use dynamic linking to support multiple API
    // Note that is very low priority since Vulkan should be available one way or another
    AssertRelease(RHI::ETargetRHI::Vulkan == rhi);

    pRHI->reset<FDefaultRHIService_>();
}
//----------------------------------------------------------------------------
void IRHIService::MakeDefault(URHIService* pRHI) {
    Make(pRHI, RHI::ETargetRHI::Vulkan);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
