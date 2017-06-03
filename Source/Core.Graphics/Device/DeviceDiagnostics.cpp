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
void Diagnostics_BeginEvent(IDeviceAPIDiagnostics *diagnostics, const FStringView& name) {
    if (diagnostics && diagnostics->UseDebugDrawEvents()) {
        wchar_t buffer[512];
        ToWCStr(buffer, name.data());
        diagnostics->BeginEvent(buffer);
    }
}
//----------------------------------------------------------------------------
void Diagnostics_SetMarker(IDeviceAPIDiagnostics *diagnostics, const FStringView& name) {
    if (diagnostics && diagnostics->UseDebugDrawEvents()) {
        wchar_t buffer[512];
        ToWCStr(buffer, name.data());
        diagnostics->SetMarker(buffer);
    }
}
//----------------------------------------------------------------------------
void Diagnostics_BeginEvent(IDeviceAPIDiagnostics *diagnostics, const FWStringView& name) {
    if (diagnostics && diagnostics->UseDebugDrawEvents())
        diagnostics->BeginEvent(name);
}
//----------------------------------------------------------------------------
void Diagnostics_SetMarker(IDeviceAPIDiagnostics *diagnostics, const FWStringView& name) {
    if (diagnostics && diagnostics->UseDebugDrawEvents())
        diagnostics->SetMarker(name);
}
//----------------------------------------------------------------------------
void Diagnostics_EndEvent(IDeviceAPIDiagnostics *diagnostics) {
    if (diagnostics && diagnostics->UseDebugDrawEvents())
        diagnostics->EndEvent();
}
//----------------------------------------------------------------------------
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
void Diagnostics_BeginEvent(const FAbstractDeviceAPIEncapsulator *encapsulator, const FStringView& name) {
    Diagnostics_BeginEvent(encapsulator->Diagnostics(), name);
}
//----------------------------------------------------------------------------
void Diagnostics_SetMarker(const FAbstractDeviceAPIEncapsulator *encapsulator, const FStringView& name) {
    Diagnostics_SetMarker(encapsulator->Diagnostics(), name);
}
//----------------------------------------------------------------------------
void Diagnostics_BeginEvent(const FAbstractDeviceAPIEncapsulator *encapsulator, const FWStringView& name) {
    Diagnostics_BeginEvent(encapsulator->Diagnostics(), name);
}
//----------------------------------------------------------------------------
void Diagnostics_SetMarker(const FAbstractDeviceAPIEncapsulator *encapsulator, const FWStringView& name) {
    Diagnostics_SetMarker(encapsulator->Diagnostics(), name);
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
