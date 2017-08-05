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

	FMouseState() { Clear(); }

    int ClientX() const { return _clientX; }
    int ClientY() const { return _clientY; }

	int DeltaClientX() const { return _deltaClientX; }
	int DeltaClientY() const { return _deltaClientY; }

    float RelativeX() const { return _relativeX; }
    float RelativeY() const { return _relativeY; }

	float DeltaRelativeX() const { return _deltaRelativeX; }
	float DeltaRelativeY() const { return _deltaRelativeY; }

	bool HasMoved() const { return (_deltaClientX || _deltaClientY); }

	bool OnEnter() const { return _onEnter; }
	bool OnLeave() const { return _onLeave; }

    const FMouseButtonState& ButtonsDown() const { return _buttonsDown; }
    const FMouseButtonState& ButtonsPressed() const { return _buttonsPressed; }
    const FMouseButtonState& ButtonsUp() const { return _buttonsUp; }

    bool IsButtonDown(EMouseButton button) const { return _buttonsDown.Contains(button); }
    bool IsButtonPressed(EMouseButton button) const { return _buttonsPressed.Contains(button); }
    bool IsButtonUp(EMouseButton button) const { return _buttonsUp.Contains(button); }

	bool HasButtonDown() const { return (_buttonsDown.size() > 0); }
	bool HasButtonPressed() const { return (_buttonsPressed.size() > 0); }
	bool HasButtonUp() const { return (_buttonsUp.size() > 0); }

    void Clear() {
        _clientX = _clientY = 0;
        _relativeX = _relativeY = 0;
		_deltaClientX = _deltaClientX = 0;
		_deltaRelativeX = _deltaRelativeY = 0;
		_onEnter = _onLeave = false;
        _buttonsDown.Clear();
        _buttonsPressed.Clear();
        _buttonsUp.Clear();
    }

private:
    int _clientX, _clientY;
    float _relativeX, _relativeY;

	int _deltaClientX, _deltaClientY;
	float _deltaRelativeX, _deltaRelativeY;

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
