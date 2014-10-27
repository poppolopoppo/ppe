#include "stdafx.h"

#include "DeviceAPIDependantEntity.h"

#include "DeviceEncapsulator.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantEntity::DeviceAPIDependantEntity(IDeviceAPIEncapsulator *device)
:   _deviceAPI(device->Encapsulator()->API()) {
}
//----------------------------------------------------------------------------
DeviceAPIDependantEntity::~DeviceAPIDependantEntity() {}
//----------------------------------------------------------------------------
bool DeviceAPIDependantEntity::MatchDevice(IDeviceAPIEncapsulator *device) const {
    Assert(device);
    return device->Encapsulator()->API() == _deviceAPI;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
