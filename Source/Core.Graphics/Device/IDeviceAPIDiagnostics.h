#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPI_fwd.h"
#include "Core.Graphics/Device/DeviceDiagnostics.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
class IDeviceAPIDiagnostics {
public:
    virtual ~IDeviceAPIDiagnostics() {}

    virtual bool UseDebugDrawEvents() const = 0;
    virtual void ToggleDebugDrawEvents(bool enabled) = 0;

    virtual void SetMarker(const FWStringView& name) = 0;
    virtual void BeginEvent(const FWStringView& name) = 0;
    virtual void EndEvent() = 0;

    virtual bool IsProfilerAttached() const = 0;
    virtual bool LaunchProfiler() = 0;
    virtual bool LaunchProfilerAndTriggerCapture() = 0;

    virtual bool IsCapturingFrame() const = 0;
    virtual void SetCaptureWindow(void* hwnd) = 0;
    virtual void TriggerCapture() = 0;
    virtual void TriggerMultiFrameCapture(size_t numFrames) = 0;
};
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
