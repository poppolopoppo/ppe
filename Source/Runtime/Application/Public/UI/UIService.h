#pragma once

#include "Application_fwd.h"

#include "RHI_fwd.h"

#include "HAL/TargetRHI_fwd.h"

#include "Memory/UniquePtr.h"
#include "Time/Timepoint.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(UIService);
class PPE_APPLICATION_API IUIService {
public:
    IUIService() = default;
    virtual ~IUIService() = default;

    IUIService(const IUIService&) = delete;
    IUIService& operator =(const IUIService&) = delete;

    virtual void OnUpdateInput(const IInputService& input, FTimespan dt) = 0;
    virtual void OnWindowFocus(const IInputService& input, const Application::FGenericWindow* previous) = 0;

    virtual void OnRenderFrame(const IRHIService& rhi, FTimespan dt) = 0;
    virtual void OnWindowResized(const IRHIService& rhi, const FRHISurfaceCreateInfo& surface) = 0;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE