#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/PresentationParameters.h"

#include "Core/Memory/MemoryTracking.h"

namespace Core {
namespace Graphics {
class FDeviceAPIDependantEntity;
class FDeviceEncapsulator;
class FDeviceResource;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAbstractDeviceAPIEncapsulator {
public:
    FAbstractDeviceAPIEncapsulator(const FAbstractDeviceAPIEncapsulator&) = delete;
    FAbstractDeviceAPIEncapsulator& operator =(const FAbstractDeviceAPIEncapsulator&) = delete;

    virtual ~FAbstractDeviceAPIEncapsulator();

    EDeviceAPI API() const { return _api; }
    FDeviceEncapsulator *Owner() const { return _owner; }
    const FPresentationParameters& Parameters() const { return _parameters; }

    virtual IDeviceAPIEncapsulator *Device() const = 0;
    virtual IDeviceAPIContext *Immediate() const = 0;

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
    virtual IDeviceAPIDiagnostics *Diagnostics() const = 0;
#endif

    virtual void Reset(const FPresentationParameters& pp) = 0;
    virtual void Present() = 0;
    virtual void ClearState() = 0;

    virtual void OnCreateEntity(const FDeviceResource *resource, FDeviceAPIDependantEntity *entity);
    virtual void OnDestroyEntity(const FDeviceResource *resource, FDeviceAPIDependantEntity *entity);

protected:
    FAbstractDeviceAPIEncapsulator(EDeviceAPI api, FDeviceEncapsulator *owner, const FPresentationParameters& pp);

private:
    EDeviceAPI _api;

    FDeviceEncapsulator *_owner;
    FPresentationParameters _parameters;

    FMemoryTrackingData _usedMemory;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
