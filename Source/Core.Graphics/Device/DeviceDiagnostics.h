#pragma once

#include "Core.Graphics/Graphics.h"

#ifndef FINAL_RELEASE
#   define WITH_CORE_GRAPHICS_DIAGNOSTICS
#endif

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
class IDeviceAPIDiagnostics;
//----------------------------------------------------------------------------
void Diagnostics_BeginEvent(const IDeviceAPIDiagnostics *diagnostics, const char *cstr);
void Diagnostics_SetMarker(const IDeviceAPIDiagnostics *diagnostics, const char *cstr);
//----------------------------------------------------------------------------
void Diagnostics_BeginEvent(const IDeviceAPIDiagnostics *diagnostics, const wchar_t *wcstr);
void Diagnostics_SetMarker(const IDeviceAPIDiagnostics *diagnostics, const wchar_t *wcstr);
//----------------------------------------------------------------------------
void Diagnostics_EndEvent(const IDeviceAPIDiagnostics *encapsulator);
//----------------------------------------------------------------------------
class FAbstractDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
void Diagnostics_BeginEvent(const FAbstractDeviceAPIEncapsulator *encapsulator, const char *cstr);
void Diagnostics_SetMarker(const FAbstractDeviceAPIEncapsulator *encapsulator, const char *cstr);
//----------------------------------------------------------------------------
void Diagnostics_BeginEvent(const FAbstractDeviceAPIEncapsulator *encapsulator, const wchar_t *wcstr);
void Diagnostics_SetMarker(const FAbstractDeviceAPIEncapsulator *encapsulator, const wchar_t *wcstr);
//----------------------------------------------------------------------------
void Diagnostics_EndEvent(const FAbstractDeviceAPIEncapsulator *encapsulator);
//----------------------------------------------------------------------------
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
#define GRAPHICS_DIAGNOSTICS_BEGINEVENT(_DiagnosticsOrEncapsulator, _Name) \
    Core::Graphics::Diagnostics_BeginEvent(_DiagnosticsOrEncapsulator, _Name)
#define GRAPHICS_DIAGNOSTICS_ENDEVENT(_DiagnosticsOrEncapsulator) \
    Core::Graphics::Diagnostics_EndEvent(_DiagnosticsOrEncapsulator)
#define GRAPHICS_DIAGNOSTICS_SETMARKER(_DiagnosticsOrEncapsulator, _Name) \
    Core::Graphics::Diagnostics_SetMarker(_DiagnosticsOrEncapsulator, _Name)
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
#define GRAPHICS_DIAGNOSTICS_BEGINEVENT(_DiagnosticsOrEncapsulator, _Name) NOOP
#define GRAPHICS_DIAGNOSTICS_ENDEVENT(_DiagnosticsOrEncapsulator) NOOP
#define GRAPHICS_DIAGNOSTICS_SETMARKER(_DiagnosticsOrEncapsulator, _Name) NOOP
//----------------------------------------------------------------------------
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
