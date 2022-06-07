#pragma once

#include "RHI_fwd.h"
#include "HAL/TargetRHI_fwd.h"

#include "Misc/Event.h"
#include "Time/Timepoint.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHI_API IRHIService {
protected:
    IRHIService() NOEXCEPT;

public: // virtual
    virtual ~IRHIService() = default;

    virtual void TearDown() = 0;

    virtual const ITargetRHI& Target() const NOEXCEPT = 0;
    virtual ERHIFeature Features() const NOEXCEPT = 0;

    virtual RHI::SFrameGraph FrameGraph() const NOEXCEPT = 0;
    virtual RHI::FWindowSurface BackBuffer() const NOEXCEPT = 0;
    virtual const RHI::FSwapchainID& Swapchain() const NOEXCEPT = 0;

    virtual RHI::FFrameIndex CurrentFrame() const NOEXCEPT = 0;
    virtual FTimespan ElapsedTime() const NOEXCEPT = 0;

    virtual RHI::SPipelineCompiler Compiler(RHI::EShaderLangFormat lang) const NOEXCEPT = 0;

    virtual void RenderFrame(FTimespan dt) = 0;
    virtual void ResizeWindow(const FRHISurfaceCreateInfo& window) = 0;

    virtual void DeviceLost() = 0;
    virtual void ReleaseMemory() NOEXCEPT = 0;

#if USE_PPE_RHIDEBUG
    virtual void UnitTest() NOEXCEPT;
#endif

public: // for all services
    using FRHIEvent = TFunction<void(const IRHIService&)>;

    THREADSAFE_EVENT(OnDeviceLost, FRHIEvent);

    using FRHIRenderEvent = TFunction<void(const IRHIService&, FTimespan)>;

    THREADSAFE_EVENT(OnRenderFrame, FRHIRenderEvent);

    using FRHISurfaceEvent = TFunction<void(const IRHIService&, const FRHISurfaceCreateInfo&)>;

    THREADSAFE_EVENT(OnWindowResized, FRHISurfaceEvent);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //! namespace PPE
