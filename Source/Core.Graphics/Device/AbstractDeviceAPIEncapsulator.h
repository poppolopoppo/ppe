#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/PresentationParameters.h"

namespace Core {
namespace Graphics {
class DeviceEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AbstractDeviceAPIEncapsulator {
public:
    AbstractDeviceAPIEncapsulator(const AbstractDeviceAPIEncapsulator&) = delete;
    AbstractDeviceAPIEncapsulator& operator =(const AbstractDeviceAPIEncapsulator&) = delete;

    virtual ~AbstractDeviceAPIEncapsulator();

    DeviceAPI API() const { return _api; }
    DeviceEncapsulator *Owner() const { return _owner; }
    const PresentationParameters& Parameters() const { return _parameters; }

    virtual IDeviceAPIEncapsulator *Device() const = 0;
    virtual IDeviceAPIContext *Immediate() const = 0;
    virtual IDeviceAPIShaderCompiler *ShaderCompiler() const = 0;

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
    virtual IDeviceAPIDiagnostics *Diagnostics() const = 0;
#endif

    virtual void Reset(const PresentationParameters& pp) = 0;
    virtual void Present() = 0;
    virtual void ClearState() = 0;

protected:
    AbstractDeviceAPIEncapsulator(DeviceAPI api, DeviceEncapsulator *owner, const PresentationParameters& pp);

private:
    DeviceAPI _api;
    DeviceEncapsulator *_owner;
    PresentationParameters _parameters;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
