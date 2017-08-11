#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/Input/InputState.h"
#include "Core.Application/Input/MouseButton.h"

#include "Core/Meta/Optional.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMouseState {
public:
    friend class FMouseInputHandler;

    class FSmoothAxis {
    public:
        static constexpr float DefaultSmoothing = 0.7f;
        FSmoothAxis() : _raw(0), _smoothing(DefaultSmoothing) {}
        float Raw() const { return _raw; }
        float Smoothed() const { return (_smoothed.has_value() ? *_smoothed : _raw); }
        float Smoothing() const { return _smoothing; }
        void SetSmoothing(float f) { _smoothing = Max(0.1f, Min(f, 1.f)); }
        void Clear() { _raw = 0; _smoothed.reset(); }
        void Set(float raw) { _raw = raw; }
        void Update() { _smoothed = Lerp(Smoothed(), _raw, _smoothing); }
    private:
        float _raw;
        Meta::TOptional<float> _smoothed;
        float _smoothing;
    };

    FMouseState() { Clear(); }

    int ClientX() const { return _clientX; }
    int ClientY() const { return _clientY; }

    int DeltaClientX() const { return _deltaClientX; }
    int DeltaClientY() const { return _deltaClientY; }

    float RelativeX() const { return _relativeX.Raw(); }
    float RelativeY() const { return _relativeY.Raw(); }

    float DeltaRelativeX() const { return _deltaRelativeX; }
    float DeltaRelativeY() const { return _deltaRelativeY; }

    float SmoothRelativeX() const { return _relativeX.Smoothed(); }
    float SmoothRelativeY() const { return _relativeY.Smoothed(); }

    float SmoothDeltaRelativeX() const { return _deltaRelativeX; }
    float SmoothDeltaRelativeY() const { return _deltaRelativeY; }

    bool HasMoved() const { return (_deltaClientX || _deltaClientY); }

    bool OnEnter() const { return _onEnter; }
    bool OnLeave() const { return _onLeave; }

    const FMouseButtonState& ButtonsDown() const { return _buttonsDown; }
    const FMouseButtonState& ButtonsPressed() const { return _buttonsPressed; }
    const FMouseButtonState& ButtonsUp() const { return _buttonsUp; }

    bool IsButtonDown(EMouseButton button) const { return _buttonsDown.Contains(button); }
    bool IsButtonPressed(EMouseButton button) const { return _buttonsPressed.Contains(button); }
    bool IsButtonUp(EMouseButton button) const { return _buttonsUp.Contains(button); }

    bool HasButtonDown() const { return (not _buttonsDown.empty()); }
    bool HasButtonPressed() const { return (not _buttonsPressed.empty()); }
    bool HasButtonUp() const { return (not _buttonsUp.empty()); }

    void Clear() {
        _clientX = _clientY = 0;
        _relativeX.Clear();
        _relativeY.Clear();
        _prevSmoothRelativeX.reset();
        _prevSmoothRelativeY.reset();
        _deltaClientX = _deltaClientX = 0;
        _deltaRelativeX = _deltaRelativeY = 0;
        _smoothDeltaRelativeX = _smoothDeltaRelativeY = 0;
        _onEnter = _onLeave = false;
        _buttonsDown.Clear();
        _buttonsPressed.Clear();
        _buttonsUp.Clear();
    }

private:
    int _clientX, _clientY;
    FSmoothAxis _relativeX, _relativeY;
    Meta::TOptional<float> _prevSmoothRelativeX, _prevSmoothRelativeY;

    int _deltaClientX, _deltaClientY;
    float _deltaRelativeX, _deltaRelativeY;
    float _smoothDeltaRelativeX, _smoothDeltaRelativeY;

    bool _onEnter;
    bool _onLeave;

    FMouseButtonState _buttonsDown;
    FMouseButtonState _buttonsPressed;
    FMouseButtonState _buttonsUp;
};
//----------------------------------------------------------------------------
typedef IInputStateProvider<FMouseState> IMouseService;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
