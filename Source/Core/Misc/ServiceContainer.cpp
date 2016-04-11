#include "stdafx.h"

#include "ServiceContainer.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ServiceContainer::ServiceContainer() {}
//----------------------------------------------------------------------------
ServiceContainer::~ServiceContainer() {
    READSCOPELOCK(_barrierRW);
    Assert(_services.empty());
}
//----------------------------------------------------------------------------
auto ServiceContainer::Find_(ServiceId serviceId) const
    -> services_type::const_iterator {
    return std::find_if(_services.begin(), _services.end(),
        [serviceId](const UCService& s) {
            return (s->Id() == serviceId);
        });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
