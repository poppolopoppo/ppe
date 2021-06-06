#pragma once

#include "RHI_fwd.h"
#include "HAL/TargetRHI_fwd.h"

#include "Misc/Event.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IRHIService {
protected:
    IRHIService() = default;

public: // virtual
    virtual ~IRHIService() = default;

    virtual void TearDown() = 0;

    virtual const ITargetRHI& Target() const NOEXCEPT = 0;
    virtual ERHIFeature Features() const NOEXCEPT = 0;

    virtual RHI::SFrameGraph FrameGraph() const NOEXCEPT = 0;
    virtual RHI::FWindowSurface BackBuffer() const NOEXCEPT = 0;

    virtual void ResizeWindow(const FRHISurfaceCreateInfo& window) = 0;
    virtual void ReleaseMemory() NOEXCEPT = 0;

public: // for all services
    using FRHIEvent = TFunction<void(const IRHIService&)>;

    PUBLIC_EVENT(OnDeviceLost, FRHIEvent);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //! namespace PPE
