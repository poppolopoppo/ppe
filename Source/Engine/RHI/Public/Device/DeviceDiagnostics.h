#pragma once

#include "Graphics.h"

#include "IO/StringView.h"

#ifndef FINAL_RELEASE
#   define WITH_PPE_GRAPHICS_DIAGNOSTICS
#endif

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
class IDeviceAPIDiagnostics;
//----------------------------------------------------------------------------
void PPE_GRAPHICS_API Diagnostics_BeginEvent(IDeviceAPIDiagnostics *diagnostics, const FStringView& name);
void PPE_GRAPHICS_API Diagnostics_SetMarker(IDeviceAPIDiagnostics *diagnostics, const FStringView& name);
//----------------------------------------------------------------------------
void PPE_GRAPHICS_API Diagnostics_BeginEvent(IDeviceAPIDiagnostics *diagnostics, const FWStringView& name);
void PPE_GRAPHICS_API Diagnostics_SetMarker(IDeviceAPIDiagnostics *diagnostics, const FWStringView& name);
//----------------------------------------------------------------------------
void PPE_GRAPHICS_API Diagnostics_EndEvent(IDeviceAPIDiagnostics *diagnostics);
//----------------------------------------------------------------------------
class FAbstractDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
void PPE_GRAPHICS_API Diagnostics_BeginEvent(const FAbstractDeviceAPIEncapsulator *encapsulator, const FStringView& name);
void PPE_GRAPHICS_API Diagnostics_SetMarker(const FAbstractDeviceAPIEncapsulator *encapsulator, const FStringView& name);
//----------------------------------------------------------------------------
void PPE_GRAPHICS_API Diagnostics_BeginEvent(const FAbstractDeviceAPIEncapsulator *encapsulator, const FWStringView& name);
void PPE_GRAPHICS_API Diagnostics_SetMarker(const FAbstractDeviceAPIEncapsulator *encapsulator, const FWStringView& name);
//----------------------------------------------------------------------------
void PPE_GRAPHICS_API Diagnostics_EndEvent(const FAbstractDeviceAPIEncapsulator *encapsulator);
//----------------------------------------------------------------------------
namespace details {
template <typename T>
struct TDiagnostics_EventScope;
template <> struct TDiagnostics_EventScope<IDeviceAPIDiagnostics*> {
    IDeviceAPIDiagnostics* Diagnostics;
    TDiagnostics_EventScope(IDeviceAPIDiagnostics* diagnostics, const FWStringView& name)
        : Diagnostics(diagnostics) {
        Diagnostics_BeginEvent(Diagnostics, name);
    }
    ~TDiagnostics_EventScope() {
        Diagnostics_EndEvent(Diagnostics);
    }
};
template <> struct TDiagnostics_EventScope<FAbstractDeviceAPIEncapsulator*> {
    const FAbstractDeviceAPIEncapsulator* Encapsulator;
    TDiagnostics_EventScope(const FAbstractDeviceAPIEncapsulator* encapsulator, const FWStringView& name)
        : Encapsulator(encapsulator) {
        Diagnostics_BeginEvent(Encapsulator, name);
    }
    ~TDiagnostics_EventScope() {
        Diagnostics_EndEvent(Encapsulator);
    }
};
} //!details
//----------------------------------------------------------------------------
#endif //!WITH_PPE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
#define GRAPHICS_DIAGNOSTICS_SCOPEEVENT(_DiagnosticsOrEncapsulator, _Name) \
    const PPE::Graphics::details::TDiagnostics_EventScope< Meta::TDecay<decltype(_DiagnosticsOrEncapsulator)> > \
        ANONYMIZE(Graphics_Diagnostics_ScopeEvent)((_DiagnosticsOrEncapsulator), (_Name))
#define GRAPHICS_DIAGNOSTICS_BEGINEVENT(_DiagnosticsOrEncapsulator, _Name) \
    PPE::Graphics::Diagnostics_BeginEvent((_DiagnosticsOrEncapsulator), (_Name))
#define GRAPHICS_DIAGNOSTICS_ENDEVENT(_DiagnosticsOrEncapsulator) \
    PPE::Graphics::Diagnostics_EndEvent(_DiagnosticsOrEncapsulator)
#define GRAPHICS_DIAGNOSTICS_SETMARKER(_DiagnosticsOrEncapsulator, _Name) \
    PPE::Graphics::Diagnostics_SetMarker((_DiagnosticsOrEncapsulator), (_Name))
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
#define GRAPHICS_DIAGNOSTICS_SCOPEEVENT(_DiagnosticsOrEncapsulator, _Name) NOOP()
#define GRAPHICS_DIAGNOSTICS_BEGINEVENT(_DiagnosticsOrEncapsulator, _Name) NOOP()
#define GRAPHICS_DIAGNOSTICS_ENDEVENT(_DiagnosticsOrEncapsulator) NOOP()
#define GRAPHICS_DIAGNOSTICS_SETMARKER(_DiagnosticsOrEncapsulator, _Name) NOOP()
//----------------------------------------------------------------------------
#endif //!WITH_PPE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
