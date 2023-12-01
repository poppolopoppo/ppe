#pragma once

#include "Application_fwd.h"

#include "Modular/Modular_fwd.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API IApplicationService {
public:
    virtual ~IApplicationService() = default;

    virtual const FModularServices& Services() const NOEXCEPT = 0;

    virtual bool PumpMessages() NOEXCEPT = 0;
    virtual void ReleaseMemory() NOEXCEPT = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
