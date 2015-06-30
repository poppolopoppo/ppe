#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceResource.h"

namespace Core {
namespace Graphics {
struct DeviceSharedEntityKey;
FWD_REFPTR(DeviceAPIDependantEntity);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(DeviceResourceSharable);
class DeviceResourceSharable : public DeviceResource {
public:
    bool Sharable() const { return _sharable; }

    virtual DeviceSharedEntityKey SharedKey() const = 0;

    bool MatchTerminalEntity(const DeviceAPIDependantEntity *entity) const;

protected:
    DeviceResourceSharable(DeviceResourceType resourceType, bool sharable);

    virtual bool VirtualMatchTerminalEntity(const DeviceAPIDependantEntity *entity) const = 0;

private:
    bool _sharable;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
