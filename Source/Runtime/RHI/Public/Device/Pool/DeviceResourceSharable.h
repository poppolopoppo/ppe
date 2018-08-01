#pragma once

#include "Graphics.h"

#include "Device/DeviceResource.h"
#include "Device/Pool/DeviceSharedEntityKey.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(DeviceResourceSharable);
FWD_WEAKPTR(DeviceResourceSharable);
class FDeviceResourceSharable : public FDeviceResource {
public:
    bool Sharable() const { return FDeviceResource::Sharable_(); }

    const FDeviceSharedEntityKey& SharedKey() const;
    bool MatchTerminalEntity(const FDeviceAPIDependantEntity *entity) const;

protected:
    FDeviceResourceSharable(EDeviceResourceType resourceType, bool sharable);

    virtual void FreezeImpl() override;
    virtual void UnfreezeImpl() override;

    virtual size_t VirtualSharedKeyHashValue() const = 0;
    virtual bool VirtualMatchTerminalEntity(const FDeviceAPIDependantEntity *entity) const = 0;

private:
    FDeviceSharedEntityKey _sharedKey;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
