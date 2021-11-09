#pragma once

#include "RHI_fwd.h"
#include "HAL/TargetRHI_fwd.h"

#include "Misc/Event.h"

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

    virtual void ResizeWindow(const FRHISurfaceCreateInfo& window) = 0;
    virtual void ReleaseMemory() NOEXCEPT = 0;

#if USE_PPE_RHIDEBUG
    virtual void UnitTest() NOEXCEPT;
#endif

public: // for all services
    using FRHIEvent = TFunction<void(const IRHIService&)>;

    THREADSAFE_EVENT(OnDeviceLost, FRHIEvent);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //! namespace PPE
