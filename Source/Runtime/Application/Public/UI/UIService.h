#pragma once

#include "Application_fwd.h"

#include "Memory/UniquePtr.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(UIService);
class PPE_APPLICATION_API IUIService {
public:
    virtual ~IUIService() = default;

    using EInputMode = Application::EInputListenerEvent;

    virtual EInputMode ToggleFocus(IInputService& inputs, EInputMode mode) = 0;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
