#pragma once

#include "Application_fwd.h"

#include "RHI_fwd.h"

#include "HAL/TargetRHI_fwd.h"

#include "Memory/UniquePtr.h"
#include "Time/Timepoint.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(UIService);
class PPE_APPLICATION_API IUIService {
public:
    virtual ~IUIService() = default;

    virtual void ToggleFocus(IInputService& inputs, bool selected, int priority = 0) = 0;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
