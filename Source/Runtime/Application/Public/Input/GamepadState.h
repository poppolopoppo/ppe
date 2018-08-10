#pragma once

#include "Application.h"

#include "Input/InputState.h"
#include "Input/GamepadButton.h"

#include "Maths/MathHelpers.h"
#include "Maths/ScalarVector.h"
#include "Meta/Optional.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGamepadState {
public:
    using FButtonState = TInputState<EGamepadButton, 8>;

    class FSmoothAnalog {
    public:
        static constexpr float DefaultSmoothing = 0.9f;
        FSmoothAnalog() : _raw(0), _smoothing(DefaultSmoothing){}
        float Raw() const { return _raw; }
        float Smoothed() const { return (_smoothed.has_value() ? *_smoothed : _raw); }
        float Smoothing() const { return _smoothing; }
        void SetSmoothing(float f) { _smoothing = Max(0.1f, Min(f, 1.f)); }
        void Clear() { _raw = 0; _smoothed.reset(); }
        void Set(float raw) {
            _raw = raw;
            _smoothed = Lerp(Smoothed(), _raw, _smoothing);
            if (Abs(*_smoothed) < 1e-3f)
                _smoothed = 0;
        }
    private:
        float _raw;
        Meta::TOptional<float> _smoothed;
        float _smoothing;
    };

    explicit FGamepadState()
        : _connected(false)
        , _onConnect(false)
        , _onDisconnect(false)
        , _leftRumble(0)
        , _rightRumble(0)
        , _index(size_t(-1))
    {}

    size_t Index() const { return _index; }

    bool IsConnected() const { return _connected; }

    bool OnConnect() const { return _onConnect; }
    bool OnDisconnect() const { return _onDisconnect; }

    const FSmoothAnalog& LeftStickX() const { return _leftStickX; }
    const FSmoothAnalog& LeftStickY() const { return _leftStickY; }

    float2 LeftStickRaw() const { return float2(_leftStickX.Raw(), _leftStickY.Raw()); }
    float2 LeftStickSmooth() const { return float2(_leftStickX.Smoothed(), _leftStickY.Smoothed()); }

    const FSmoothAnalog& RightStickX() const { return _rightStickX; }
    const FSmoothAnalog& RightStickY() const { return _rightStickY; }

    float2 RightStickRaw() const { return float2(_rightStickX.Raw(), _rightStickY.Raw()); }
    float2 RightStickSmooth() const { return float2(_rightStickX.Smoothed(), _rightStickY.Smoothed()); }

    const FSmoothAnalog& LeftTrigger() const { return _leftTrigger; }
    const FSmoothAnalog& RightTrigger() const { return _rightTrigger; }

    float2 TriggersRaw() const { return float2(_leftTrigger.Raw(), _rightTrigger.Raw()); }
    float2 TriggersSmooth() const { return float2(_leftTrigger.Smoothed(), _rightTrigger.Smoothed()); }

    const FButtonState& ButtonsDown() const { return _buttonsDown; }
    const FButtonState& ButtonsPressed() const { return _buttonsPressed; }
    const FButtonState& ButtonsUp() const { return _buttonsUp; }

    bool IsButtonDown(EGamepadButton button) const { return _buttonsDown.Contains(button); }
    bool IsButtonPressed(EGamepadButton button) const { return _buttonsPressed.Contains(button); }
    bool IsButtonUp(EGamepadButton button) const { return _buttonsUp.Contains(button); }

    bool HasButtonDown() const { return (not _buttonsDown.empty()); }
    bool HasButtonPressed() const { return (not _buttonsPressed.empty()); }
    bool HasButtonUp() const { return (not _buttonsUp.empty()); }

    float LeftRumble() const { return _leftRumble; }
    float RightRumble() const { return _rightRumble; }

    void Clear() {
        _connected = false;
        _onConnect = false;
        _onDisconnect = false;

        _leftStickX.Clear();
        _leftStickY.Clear();

        _rightStickX.Clear();
        _rightStickY.Clear();

        _leftTrigger.Clear();
        _rightTrigger.Clear();

        _buttonsDown.Clear();
        _buttonsPressed.Clear();
        _buttonsUp.Clear();
    }

    void SetStatus(bool connected, bool onConnect, bool onDisconnect) {
        _connected = connected;
        _onConnect = onConnect;
        _onDisconnect = onDisconnect;
    }
    void SetLeftStick(float x, float y) {
        _leftStickX.Set(x);
        _leftStickY.Set(y);
    }
    void SetRightStick(float x, float y) {
        _rightStickX.Set(x);
        _rightStickY.Set(y);
    }
    void SetTrigger(float left, float right) {
        _leftTrigger.Set(left);
        _rightTrigger.Set(right);
    }
    void SetButtonsDown(FButtonState state) {
        _buttonsDown = state;
    }
    void SetButtonsPressed(FButtonState state) {
        _buttonsPressed = state;
    }
    void SetButtonsUp(FButtonState state) {
        _buttonsUp = state;
    }
    void SetRumble(float left, float right) {
        _leftRumble = left;
        _rightRumble = right;
    }

private:
    bool _connected     : 1;
    bool _onConnect     : 1;
    bool _onDisconnect  : 1;

    FSmoothAnalog _leftStickX;
    FSmoothAnalog _leftStickY;

    FSmoothAnalog _rightStickX;
    FSmoothAnalog _rightStickY;

    FSmoothAnalog _leftTrigger;
    FSmoothAnalog _rightTrigger;

    FButtonState _buttonsDown;
    FButtonState _buttonsPressed;
    FButtonState _buttonsUp;

    float _leftRumble;
    float _rightRumble;

    size_t _index;
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
} //!namespace PPE
