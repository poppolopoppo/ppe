#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/Input/InputState.h"
#include "Core.Application/Input/GamepadButton.h"

#include "Core/Maths/ScalarVector.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGamepadState {
public:
    friend class FGamepadInputHandler;

    explicit FGamepadState()
        : _connected(false)
        , _onConnect(false)
        , _onDisconnect(false)
        , _leftStickX(0), _leftStickY(0)
        , _rightStickX(0), _rightStickY(0)
        , _leftTrigger(0), _rightTrigger(0) {}

    bool IsConnected() const { return _connected; }

    bool OnConnect() const { return _onConnect; }
    bool OnDisconnect() const { return _onDisconnect; }

    float LeftStickX() const { return _leftStickX; }
    float LeftStickY() const { return _leftStickY; }

    float2 LeftStick() const { return float2(_leftStickX, _leftStickY); }

    float RightStickX() const { return _rightStickX; }
    float RightStickY() const { return _rightStickY; }

    float2 RightStick() const { return float2(_rightStickX, _rightStickY); }

    float LeftTrigger() const { return _leftTrigger; }
    float RightTrigger() const { return _rightTrigger; }

    float2 Triggers() const { return float2(_leftTrigger, _rightTrigger); }

    const GamepadButtonState& ButtonsDown() const { return _buttonsDown; }
    const GamepadButtonState& ButtonsPressed() const { return _buttonsPressed; }
    const GamepadButtonState& ButtonsUp() const { return _buttonsUp; }

    void Clear() {
        _connected = false;
        _onConnect = false;
        _onDisconnect = false;

        _leftStickX = _leftStickY = 0;
        _rightStickX = _rightStickY = 0;
        _leftTrigger = _rightTrigger = 0;

        _buttonsDown.Clear();
        _buttonsPressed.Clear();
        _buttonsUp.Clear();
    }

private:
    bool _connected     : 1;
    bool _onConnect     : 1;
    bool _onDisconnect  : 1;

    float _leftStickX;
    float _leftStickY;

    float _rightStickX;
    float _rightStickY;

    float _leftTrigger;
    float _rightTrigger;

    GamepadButtonState _buttonsDown;
    GamepadButtonState _buttonsPressed;
    GamepadButtonState _buttonsUp;
};
//----------------------------------------------------------------------------
class FMultiGamepadState {
public:
    STATIC_CONST_INTEGRAL(size_t, MaxConnected, 8);

    FMultiGamepadState() {}

    const FGamepadState& First() const { return _gamepads[0]; }

    TMemoryView<FGamepadState> Gamepads() { return MakeView(_gamepads); }
    TMemoryView<const FGamepadState> Gamepads() const { return MakeView(_gamepads); }

    void Clear() {
        forrange(i, 0, MaxConnected)
            _gamepads[i].Clear();
    }

private:
    FGamepadState _gamepads[MaxConnected];
};
//----------------------------------------------------------------------------
typedef IInputStateProvider<FMultiGamepadState> IGamepadService;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
