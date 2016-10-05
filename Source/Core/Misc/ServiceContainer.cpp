#include "stdafx.h"

#include "ServiceContainer.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FServiceContainer::FServiceContainer() {}
//----------------------------------------------------------------------------
FServiceContainer::~FServiceContainer() {
    READSCOPELOCK(_barrierRW);
    Assert(_services.empty());
}
//----------------------------------------------------------------------------
auto FServiceContainer::Find_(ServiceId serviceId) const
    -> services_type::const_iterator {
    return std::find_if(_services.begin(), _services.end(),
        [serviceId](const TService& service) {
            return (service.Id() == serviceId);
        });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
