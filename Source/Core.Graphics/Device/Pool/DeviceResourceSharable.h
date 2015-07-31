#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceResource.h"
#include "Core.Graphics/Device/Pool/DeviceSharedEntityKey.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(DeviceResourceSharable);
FWD_WEAKPTR(DeviceResourceSharable);
class DeviceResourceSharable : public DeviceResource {
public:
    bool Sharable() const { return DeviceResource::Sharable_(); }

    const DeviceSharedEntityKey& SharedKey() const;
    bool MatchTerminalEntity(const DeviceAPIDependantEntity *entity) const;

protected:
    DeviceResourceSharable(DeviceResourceType resourceType, bool sharable);

    virtual void FreezeImpl() override;
    virtual void UnfreezeImpl() override;

    virtual size_t VirtualSharedKeyHashValue() const = 0;
    virtual bool VirtualMatchTerminalEntity(const DeviceAPIDependantEntity *entity) const = 0;

private:
    DeviceSharedEntityKey _sharedKey;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
