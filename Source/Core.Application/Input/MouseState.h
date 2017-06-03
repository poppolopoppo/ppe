#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/Input/InputState.h"
#include "Core.Application/Input/MouseButton.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMouseState {
public:
    friend class FMouseInputHandler;

    FMouseState() : _clientX(0), _clientY(0), _relativeX(0), _relativeY(0) {}

    int ClientX() const { return _clientX; }
    int ClientY() const { return _clientY; }

    float RelativeX() const { return _relativeX; }
    float RelativeY() const { return _relativeY; }

    const FMouseButtonState& ButtonsDown() const { return _buttonsDown; }
    const FMouseButtonState& ButtonsPressed() const { return _buttonsPressed; }
    const FMouseButtonState& ButtonsUp() const { return _buttonsUp; }

    bool IsButtonDown(EMouseButton button) const { return _buttonsDown.Contains(button); }
    bool IsButtonPressed(EMouseButton button) const { return _buttonsPressed.Contains(button); }
    bool IsButtonUp(EMouseButton button) const { return _buttonsUp.Contains(button); }

    void Clear() {
        _clientX = _clientY = 0;
        _relativeX = _relativeY = 0;
        _buttonsDown.Clear();
        _buttonsPressed.Clear();
        _buttonsUp.Clear();
    }

private:
    int _clientX, _clientY;
    float _relativeX, _relativeY;

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
