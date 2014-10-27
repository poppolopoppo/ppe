#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
enum class DeviceAPI;
FWD_REFPTR(DeviceAPIDependantEntity);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantEntity : public RefCountable {
protected:
    DeviceAPIDependantEntity(IDeviceAPIEncapsulator *device);

public:
    virtual ~DeviceAPIDependantEntity();

    DeviceAPIDependantEntity(const DeviceAPIDependantEntity& ) = delete;
    DeviceAPIDependantEntity& operator =(const DeviceAPIDependantEntity& ) = delete;

    Graphics::DeviceAPI DeviceAPI() const { return _deviceAPI; }
    bool MatchDevice(IDeviceAPIEncapsulator *device) const;

private:
    Graphics::DeviceAPI _deviceAPI;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
