// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "DeviceEncapsulator.h"

#include "AbstractDeviceAPIEncapsulator.h"

#include "Diagnostic/Logger.h"

// IDeviceAPIDiagnostics

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Diagnostics
//----------------------------------------------------------------------------
#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
bool FDeviceEncapsulator::UseDebugDrawEvents() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return _deviceAPIEncapsulator->Diagnostics()->UseDebugDrawEvents();
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::ToggleDebugDrawEvents(bool enabled) {
    THIS_THREADRESOURCE_CHECKACCESS();
    _deviceAPIEncapsulator->Diagnostics()->ToggleDebugDrawEvents(enabled);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetMarker(const FWStringView& name) {
    THIS_THREADRESOURCE_CHECKACCESS();
    _deviceAPIEncapsulator->Diagnostics()->SetMarker(name);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::BeginEvent(const FWStringView& name) {
    THIS_THREADRESOURCE_CHECKACCESS();
    _deviceAPIEncapsulator->Diagnostics()->BeginEvent(name);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::EndEvent() {
    THIS_THREADRESOURCE_CHECKACCESS();
    _deviceAPIEncapsulator->Diagnostics()->EndEvent();
}
//----------------------------------------------------------------------------
bool FDeviceEncapsulator::IsProfilerAttached() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return _deviceAPIEncapsulator->Diagnostics()->IsProfilerAttached();
}
//----------------------------------------------------------------------------
bool FDeviceEncapsulator::LaunchProfiler() {
    THIS_THREADRESOURCE_CHECKACCESS();
    return _deviceAPIEncapsulator->Diagnostics()->LaunchProfiler();
}
//----------------------------------------------------------------------------
bool FDeviceEncapsulator::LaunchProfilerAndTriggerCapture() {
    THIS_THREADRESOURCE_CHECKACCESS();
    return _deviceAPIEncapsulator->Diagnostics()->LaunchProfilerAndTriggerCapture();
}
//----------------------------------------------------------------------------
bool FDeviceEncapsulator::IsCapturingFrame() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return _deviceAPIEncapsulator->Diagnostics()->IsCapturingFrame();
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetCaptureWindow(void* hwnd) {
    THIS_THREADRESOURCE_CHECKACCESS();
    return _deviceAPIEncapsulator->Diagnostics()->SetCaptureWindow(hwnd);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::TriggerCapture() {
    THIS_THREADRESOURCE_CHECKACCESS();
    return _deviceAPIEncapsulator->Diagnostics()->TriggerCapture();
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::TriggerMultiFrameCapture(size_t numFrames) {
    THIS_THREADRESOURCE_CHECKACCESS();
    return _deviceAPIEncapsulator->Diagnostics()->TriggerMultiFrameCapture(numFrames);
}
//----------------------------------------------------------------------------
#endif //!WITH_PPE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
