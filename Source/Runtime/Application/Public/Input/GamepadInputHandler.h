#pragma once

#include "Application.h"

#include "Container/Vector.h"
#include "Input/GamepadState.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGamepadInputHandler {
public:
    using FMultiGamepadState = VECTOR(Input, FGamepadState);

    FGamepadInputHandler();
    virtual ~FGamepadInputHandler();

    FGamepadInputHandler(const FGamepadInputHandler& ) = delete;
    FGamepadInputHandler& operator =(const FGamepadInputHandler& ) = delete;

    const FGamepadState* FirstConnectedIFP() const;
    const FGamepadState& State(size_t index) const;
    const FMultiGamepadState& States() const { return _states; }

    void Poll();
    void Update(FTimespan dt);
    void Rumble(size_t index, float left, float right);
    void Clear();

private:
    FMultiGamepadState _states;
    size_t _firstConnected;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
