#pragma once

#include "Application.h"

#include "HAL/PlatformWindow.h"
#include "Input/MouseState.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMouseInputHandler {
public:
    FMouseInputHandler();
    virtual ~FMouseInputHandler();

    FMouseInputHandler(const FMouseInputHandler& ) = delete;
    FMouseInputHandler& operator =(const FMouseInputHandler& ) = delete;

    const FMouseState& State() const { return _state; }

    void SetupWindow(FPlatformWindow& window);
    void Update(FTimespan dt);
    void Clear();

private:
    FMouseState _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
