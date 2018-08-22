#include "stdafx.h"

#include "Input/GamepadState.h"

#include "Maths/ScalarVector.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float2 FGamepadState::LeftStick() const {
    return LeftStickFiltered();
}
//----------------------------------------------------------------------------
float2 FGamepadState::LeftStickRaw() const {
    return float2(_leftStickX.Raw(), _leftStickY.Raw());
}
//----------------------------------------------------------------------------
float2 FGamepadState::LeftStickFiltered() const {
    return float2(_leftStickX.Filtered(), _leftStickY.Filtered());
}
//----------------------------------------------------------------------------
float2 FGamepadState::RightStick() const {
    return RightStickFiltered();
}
//----------------------------------------------------------------------------
float2 FGamepadState::RightStickRaw() const {
    return float2(_rightStickX.Raw(), _rightStickY.Raw());
}
//----------------------------------------------------------------------------
float2 FGamepadState::RightStickFiltered() const {
    return float2(_rightStickX.Filtered(), _rightStickY.Filtered());
}
//----------------------------------------------------------------------------
float2 FGamepadState::Triggers() const {
    return TriggersFiltered();
}
//----------------------------------------------------------------------------
float2 FGamepadState::TriggersRaw() const {
    return float2(_leftTrigger.Raw(), _rightTrigger.Raw());
}
//----------------------------------------------------------------------------
float2 FGamepadState::TriggersFiltered() const {
    return float2(_leftTrigger.Filtered(), _rightTrigger.Filtered());
}
//----------------------------------------------------------------------------
void FGamepadState::SetStatus(size_t index, bool connected) {
    _index = index;
    _connected = connected;
}
//----------------------------------------------------------------------------
bool FGamepadState::SetButtonPressed(EGamepadButton btn) {
    Assert(_connected);
    return _buttonsQueued.Add_KeepExisting(btn);
}
//----------------------------------------------------------------------------
void FGamepadState::Update(float dt) {
    _onConnect = (not _wasConnected && _connected);
    _onDisconnect = (_wasConnected && not _connected);
    _wasConnected = _connected;

    _leftStickX.Update(dt);
    _leftStickY.Update(dt);

    _rightStickX.Update(dt);
    _rightStickY.Update(dt);

    _leftTrigger.Update(dt);
    _rightTrigger.Update(dt);

    // that way we never miss an input
    _buttonsPressed.Update(&_buttonsUp, &_buttonsDown, _buttonsQueued);
    _buttonsQueued.Clear();
}
//----------------------------------------------------------------------------
void FGamepadState::Clear() {
    _connected = false;
    _onConnect = false;
    _onDisconnect = false;

    _index = INDEX_NONE; // serves as canary

    _leftStickX.Clear();
    _leftStickY.Clear();

    _rightStickX.Clear();
    _rightStickY.Clear();

    _leftTrigger.Clear();
    _rightTrigger.Clear();

    _buttonsDown.Clear();
    _buttonsPressed.Clear();
    _buttonsUp.Clear();
    _buttonsQueued.Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
