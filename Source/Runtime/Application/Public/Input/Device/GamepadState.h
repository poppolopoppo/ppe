#pragma once

#include "Application_fwd.h"

#include "Input/Device/FilteredAnalog.h"
#include "Input/Device/InputState.h"
#include "Input/GamepadButton.h"

#include "Maths/ScalarVector_fwd.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FGamepadState {
public:
    using FButtonState = TInputState<EGamepadButton, 8>;

    explicit FGamepadState()
        : _connected(false)
        , _wasConnected(false)
        , _onConnect(false)
        , _onDisconnect(false)
        , _index(INDEX_NONE)
        , _leftRumble(0)
        , _rightRumble(0)
    {}

    size_t Index() const { return _index; }

    bool IsConnected() const { return _connected; }

    bool OnConnect() const { return _onConnect; }
    bool OnDisconnect() const { return _onDisconnect; }

    const FFilteredAnalog& LeftStickX() const { return _leftStickX; }
    const FFilteredAnalog& LeftStickY() const { return _leftStickY; }

    float2 LeftStick() const;
    float2 LeftStickDelta() const;
    float2 LeftStickRaw() const;
    float2 LeftStickFiltered() const;

    const FFilteredAnalog& RightStickX() const { return _rightStickX; }
    const FFilteredAnalog& RightStickY() const { return _rightStickY; }

    float2 RightStick() const;
    float2 RightStickDelta() const;
    float2 RightStickRaw() const;
    float2 RightStickFiltered() const;

    const FFilteredAnalog& TriggerX() const { return _leftTrigger; }
    const FFilteredAnalog& TriggerY() const { return _rightTrigger; }

    float LeftTrigger() const;
    float RightTrigger() const;

    float2 Triggers() const;
    float2 TriggersDelta() const;
    float2 TriggersRaw() const;
    float2 TriggersFiltered() const;

    const FButtonState& ButtonsDown() const { return _buttonsDown; }
    const FButtonState& ButtonsPressed() const { return _buttonsPressed; }
    const FButtonState& ButtonsUp() const { return _buttonsUp; }

    bool IsButtonDown(EGamepadButton button) const { return _buttonsDown.Contains(button); }
    bool IsButtonPressed(EGamepadButton button) const { return _buttonsPressed.Contains(button); }
    bool IsButtonUp(EGamepadButton button) const { return _buttonsUp.Contains(button); }

    bool HasButtonDown() const { return (not _buttonsDown.empty()); }
    bool HasButtonPressed() const { return (not _buttonsPressed.empty()); }
    bool HasButtonUp() const { return (not _buttonsUp.empty()); }

    bool UseFilteredInputs() const { return _useFilteredInputs; }
    void SetUseFilteredInputs(bool value) { _useFilteredInputs = value; }

    float LeftRumble() const { return _leftRumble; }
    float RightRumble() const { return _rightRumble; }

    void SetStatus(size_t index, bool connected);
    bool SetButtonPressed(EGamepadButton btn);

    void SetLeftStick(float x, float y) {
        _leftStickX.SetRaw(x);
        _leftStickY.SetRaw(y);
    }
    void SetRightStick(float x, float y) {
        _rightStickX.SetRaw(x);
        _rightStickY.SetRaw(y);
    }
    void SetTriggers(float left, float right) {
        _leftTrigger.SetRaw(left);
        _rightTrigger.SetRaw(right);
    }
    void SetRumble(float left, float right) {
        _leftRumble = left;
        _rightRumble = right;
    }

    void Update(FTimespan dt);
    void Clear();

private:
    bool _connected     : 1;
    bool _wasConnected  : 1;

    bool _onConnect     : 1;
    bool _onDisconnect  : 1;

    size_t _index;

    FFilteredAnalog _leftStickX;
    FFilteredAnalog _leftStickY;

    FFilteredAnalog _rightStickX;
    FFilteredAnalog _rightStickY;

    FFilteredAnalog _leftTrigger;
    FFilteredAnalog _rightTrigger;

    FButtonState _buttonsDown;
    FButtonState _buttonsPressed;
    FButtonState _buttonsUp;
    FButtonState _buttonsQueued;

    float _leftRumble;
    float _rightRumble;

    bool _useFilteredInputs{ true };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
