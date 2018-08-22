#pragma once

#include "Application.h"

#include "HAL/PlatformWindow.h"
#include "Input/KeyboardState.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FKeyboardInputHandler {
public:
    FKeyboardInputHandler();
    virtual ~FKeyboardInputHandler();

    FKeyboardInputHandler(const FKeyboardInputHandler& ) = delete;
    FKeyboardInputHandler& operator =(const FKeyboardInputHandler& ) = delete;

    const FKeyboardState& State() const { return _state; }

    void SetupWindow(FPlatformWindow& window);
    void Update(FTimespan dt);
    void Clear();

private:
    FKeyboardState _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
