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
    virtual bool IsProfilerAttached() const = 0;

    virtual void BeginEvent(const wchar_t *name) = 0;
    virtual void EndEvent() = 0;

    virtual void SetMarker(const wchar_t *name) = 0;
};
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
