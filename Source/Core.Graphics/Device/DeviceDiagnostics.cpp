#include "stdafx.h"

#include "DeviceDiagnostics.h"

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
#   include "AbstractDeviceAPIEncapsulator.h"
#   include "IDeviceAPIDiagnostics.h"
#   include "Core/IO/Format.h"
#   include "Core/IO/String.h"
#endif

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
void Diagnostics_BeginEvent(IDeviceAPIDiagnostics *diagnostics, const char *cstr) {
    if (diagnostics && diagnostics->IsProfilerAttached()) {
        wchar_t buffer[1024];
        ToWCStr(buffer, cstr);
        diagnostics->BeginEvent(buffer);
    }
}
//----------------------------------------------------------------------------
void Diagnostics_SetMarker(IDeviceAPIDiagnostics *diagnostics, const char *cstr) {
    if (diagnostics && diagnostics->IsProfilerAttached()) {
        wchar_t buffer[1024];
        ToWCStr(buffer, cstr);
        diagnostics->SetMarker(buffer);
    }
}
//----------------------------------------------------------------------------
void Diagnostics_BeginEvent(IDeviceAPIDiagnostics *diagnostics, const wchar_t *wcstr) {
    if (diagnostics && diagnostics->IsProfilerAttached())
        diagnostics->BeginEvent(wcstr);
}
//----------------------------------------------------------------------------
void Diagnostics_SetMarker(IDeviceAPIDiagnostics *diagnostics, const wchar_t *wcstr) {
    if (diagnostics && diagnostics->IsProfilerAttached())
        diagnostics->SetMarker(wcstr);
}
//----------------------------------------------------------------------------
void Diagnostics_EndEvent(IDeviceAPIDiagnostics *diagnostics) {
    if (diagnostics && diagnostics->IsProfilerAttached())
        diagnostics->EndEvent();
}
//----------------------------------------------------------------------------
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
void Diagnostics_BeginEvent(const FAbstractDeviceAPIEncapsulator *encapsulator, const char *cstr) {
    Diagnostics_BeginEvent(encapsulator->Diagnostics(), cstr);
}
//----------------------------------------------------------------------------
void Diagnostics_SetMarker(const FAbstractDeviceAPIEncapsulator *encapsulator, const char *cstr) {
    Diagnostics_SetMarker(encapsulator->Diagnostics(), cstr);
}
//----------------------------------------------------------------------------
void Diagnostics_BeginEvent(const FAbstractDeviceAPIEncapsulator *encapsulator, const wchar_t *wcstr) {
    Diagnostics_BeginEvent(encapsulator->Diagnostics(), wcstr);
}
//----------------------------------------------------------------------------
void Diagnostics_SetMarker(const FAbstractDeviceAPIEncapsulator *encapsulator, const wchar_t *wcstr) {
    Diagnostics_SetMarker(encapsulator->Diagnostics(), wcstr);
}
//----------------------------------------------------------------------------
void Diagnostics_EndEvent(const FAbstractDeviceAPIEncapsulator *encapsulator) {
    Diagnostics_EndEvent(encapsulator->Diagnostics());
}
//----------------------------------------------------------------------------
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
