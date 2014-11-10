#pragma once

#include "Core.Graphics/Graphics.h"

#ifndef FINAL_RELEASE
#   define WITH_CORE_GRAPHICS_DIAGNOSTICS
#endif

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
#   include "Core/IO/Format.h"
#endif

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AbstractDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS

void Diagnostics_BeginEvent(const AbstractDeviceAPIEncapsulator *encapsulator, const char *cstr);
void Diagnostics_SetMarker(const AbstractDeviceAPIEncapsulator *encapsulator, const char *cstr);

void Diagnostics_BeginEvent(const AbstractDeviceAPIEncapsulator *encapsulator, const wchar_t *wcstr);
void Diagnostics_SetMarker(const AbstractDeviceAPIEncapsulator *encapsulator, const wchar_t *wcstr);

void Diagnostics_EndEvent(const AbstractDeviceAPIEncapsulator *encapsulator);

#define GRAPHICS_DIAGNOSTICS_BEGINEVENT(_Encapsulator, _Name) \
    Core::Graphics::Diagnostics_BeginEvent(_Encapsulator, _Name)
#define GRAPHICS_DIAGNOSTICS_ENDEVENT(_Encapsulator) \
    Core::Graphics::Diagnostics_EndEvent(_Encapsulator)
#define GRAPHICS_DIAGNOSTICS_SETMARKER(_Encapsulator, _Name) \
    Core::Graphics::Diagnostics_SetMarker(_Encapsulator, _Name)

#else

#define GRAPHICS_DIAGNOSTICS_BEGINEVENT(_Encapsulator, _Name) NOOP
#define GRAPHICS_DIAGNOSTICS_ENDEVENT(_Encapsulator) NOOP
#define GRAPHICS_DIAGNOSTICS_SETMARKER(_Encapsulator, _Name) NOOP

#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
