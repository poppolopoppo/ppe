#include "stdafx.h"

#include "Service/RHIService.h"

#include "HAL/TargetRHI.h"

#include "RHIModule.h"
#include "RHI/FrameGraph.h"
#include "RHI/SwapchainDesc.h"

#include "Window/WindowBase.h"
#include "Window/WindowRHI.h"

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
    explicit FDefaultRHIService_(ETargetRHI rhi) : _rhi(rhi) {}

    virtual RHI::PFrameGraph CreateDefaultFrameGraph(FWindowRHI* window) override;
    virtual void DestroyDefaultFrameGraph(FWindowRHI* window, RHI::PFrameGraph& frameGraph) override;

    virtual RHI::PFrameGraph CreateHeadlessFrameGraph(bool computeOnly) override;
    virtual void DestroyHeadlessFrameGraph(RHI::PFrameGraph& frameGraph) override;

    virtual RHI::PFrameGraph MainFrameGraph() const NOEXCEPT override { return _pfg; }
    virtual void SetMainFrameGraph(RHI::PFrameGraph&& frameGraph) override { _pfg = std::move(frameGraph); }

private:
    const ETargetRHI _rhi;
    RHI::PFrameGraph _pfg;
};
//----------------------------------------------------------------------------
RHI::PFrameGraph FDefaultRHIService_::CreateDefaultFrameGraph(FWindowRHI* window) {
    Assert(window);

    FRHIModule& rhi = FRHIModule::Get(FModularDomain::Get());

    RHI::PFrameGraph pfg = rhi.CreateFrameGraph(_rhi, ERHIFeature::Default, window->Handle());
    Assert(pfg);

    RHI::FSwapchainDesc swapchainDesc;
    swapchainDesc.SetWindow(window->Handle(), UMax);

    RHI::FSwapchainID swapchainId = pfg->CreateSwapchain(swapchainDesc, Default, "main-window");
    window->SetSwapchainRHI(std::move(swapchainId));

    return pfg;
}
//----------------------------------------------------------------------------
void FDefaultRHIService_::DestroyDefaultFrameGraph(FWindowRHI* window, RHI::PFrameGraph& frameGraph) {
    Assert(window);
    Assert(frameGraph);

    FRHIModule& rhi = FRHIModule::Get(FModularDomain::Get());

    window->ReleaseSwapchainRHI();

    rhi.DestroyFrameGraph(frameGraph);
}
//----------------------------------------------------------------------------
RHI::PFrameGraph FDefaultRHIService_::CreateHeadlessFrameGraph(bool computeOnly) {
    FRHIModule& rhi = FRHIModule::Get(FModularDomain::Get());

    ERHIFeature features = ERHIFeature::Headless;
    if (computeOnly)
        features += ERHIFeature::Compute;
    else
        features += ERHIFeature::Minimal;

    return rhi.CreateFrameGraph(_rhi, features);
}
//----------------------------------------------------------------------------
void FDefaultRHIService_::DestroyHeadlessFrameGraph(RHI::PFrameGraph& frameGraph) {
    Assert(frameGraph);

    FRHIModule& rhi = FRHIModule::Get(FModularDomain::Get());

    rhi.DestroyFrameGraph(frameGraph);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void IRHIService::CreateMainFrameGraph(FWindowRHI* window) {
    RHI::PFrameGraph pfg = (!!window
        ? CreateDefaultFrameGraph(window)
        : CreateHeadlessFrameGraph(false) );

    SetMainFrameGraph(std::move(pfg));
}
//----------------------------------------------------------------------------
void IRHIService::DestroyMainFrameGraph(FWindowRHI* window) {
    RHI::PFrameGraph pfg = MainFrameGraph();
    Assert(pfg);

    SetMainFrameGraph(RHI::PFrameGraph{});

    if (window)
        DestroyDefaultFrameGraph(window, pfg);
    else
        DestroyHeadlessFrameGraph(pfg);
}
//----------------------------------------------------------------------------
void IRHIService::Make(URHIService* pRHI, ETargetRHI rhi) {
    Assert(pRHI);

    pRHI->reset<FDefaultRHIService_>(rhi);
}
//----------------------------------------------------------------------------
void IRHIService::MakeDefault(URHIService* pRHI) {
    Make(pRHI, ETargetRHI::Current);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
