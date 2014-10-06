#include "stdafx.h"

#include "DeviceDiagnostics.h"

#include "DeviceAPIEncapsulator.h"
#include "DeviceEncapsulator.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
void Diagnostics_BeginEvent(const AbstractDeviceAPIEncapsulator *encapsulator, const char *cstr) {
    IDeviceAPIDiagnosticsEncapsulator *const diagnostics = encapsulator->Diagnostics();
    if (diagnostics && diagnostics->IsProfilerAttached()) {
        wchar_t buffer[1024];
        ToWCStr(buffer, cstr);
        diagnostics->BeginEvent(buffer);
    }
}
//----------------------------------------------------------------------------
void Diagnostics_SetMarker(const AbstractDeviceAPIEncapsulator *encapsulator, const char *cstr) {
    IDeviceAPIDiagnosticsEncapsulator *const diagnostics = encapsulator->Diagnostics();
    if (diagnostics && diagnostics->IsProfilerAttached()) {
        wchar_t buffer[1024];
        ToWCStr(buffer, cstr);
        diagnostics->SetMarker(buffer);
    }
}
//----------------------------------------------------------------------------
void Diagnostics_BeginEvent(const AbstractDeviceAPIEncapsulator *encapsulator, const wchar_t *wcstr) {
    IDeviceAPIDiagnosticsEncapsulator *const diagnostics = encapsulator->Diagnostics();
    if (diagnostics && diagnostics->IsProfilerAttached())
        diagnostics->BeginEvent(wcstr);
}
//----------------------------------------------------------------------------
void Diagnostics_SetMarker(const AbstractDeviceAPIEncapsulator *encapsulator, const wchar_t *wcstr) {
    IDeviceAPIDiagnosticsEncapsulator *const diagnostics = encapsulator->Diagnostics();
    if (diagnostics && diagnostics->IsProfilerAttached())
        diagnostics->SetMarker(wcstr);
}
//----------------------------------------------------------------------------
void Diagnostics_EndEvent(const AbstractDeviceAPIEncapsulator *encapsulator) {
    IDeviceAPIDiagnosticsEncapsulator *const diagnostics = encapsulator->Diagnostics();
    if (diagnostics && diagnostics->IsProfilerAttached())
        diagnostics->EndEvent();
}
//----------------------------------------------------------------------------
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
