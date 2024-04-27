#pragma once

#include "Application_fwd.h"

#include "Input/Device/FilteredAnalog.h"
#include "Input/Device/InputState.h"
#include "Input/MouseButton.h"

#include "Maths/ScalarVector_fwd.h"
#include "Time/Time_fwd.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMouseState {
public:
    using FButtonState = TInputState<EMouseButton, 6>;

    FMouseState() { Clear(); }

    int2 Screen() const;

    int ScreenX() const { return _screenX; }
    int ScreenY() const { return _screenY; }

    int2 Client() const;

    int ClientX() const { return _clientX; }
    int ClientY() const { return _clientY; }

    float2 Relative() const;
    float2 RelativeRaw() const;
    float2 RelativeFiltered() const;

    const FFilteredAnalog& RelativeX() const { return _relativeX; }
    const FFilteredAnalog& RelativeY() const { return _relativeY; }

    int DeltaClientX() const { return _deltaClientX; }
    int DeltaClientY() const { return _deltaClientY; }

    int2 DeltaClient() const;

    float DeltaRelativeX() const { return _deltaRelativeFilteredX; }
    float DeltaRelativeY() const { return _deltaRelativeFilteredY; }

    float DeltaRelativeRawX() const { return _deltaRelativeRawX; }
    float DeltaRelativeRawY() const { return _deltaRelativeRawY; }

    float DeltaRelativeFilteredX() const { return _deltaRelativeFilteredX; }
    float DeltaRelativeFilteredY() const { return _deltaRelativeFilteredY; }

    float2 DeltaRelative() const;
    float2 DeltaRelativeRaw() const;
    float2 DeltaRelativeFiltered() const;

    bool HasMoved() const { return (_deltaClientX || _deltaClientY); }

    const FFilteredAnalog& WheelX() const { return _wheelX; }
    const FFilteredAnalog& WheelY() const { return _wheelY; }

    float2 Wheel() const;
    float2 WheelFiltered() const;
    float2 WheelRaw() const;

    bool Inside() const { return _inside; }
    bool OnEnter() const { return _onEnter; }
    bool OnLeave() const { return _onLeave; }

    const FButtonState& ButtonsDown() const { return _buttonsDown; }
    const FButtonState& ButtonsPressed() const { return _buttonsPressed; }
    const FButtonState& ButtonsUp() const { return _buttonsUp; }

    bool IsButtonDown(EMouseButton button) const { return _buttonsDown.Contains(button); }
    bool IsButtonPressed(EMouseButton button) const { return _buttonsPressed.Contains(button); }
    bool IsButtonUp(EMouseButton button) const { return _buttonsUp.Contains(button); }

    bool HasButtonDown() const { return (not _buttonsDown.empty()); }
    bool HasButtonPressed() const { return (not _buttonsPressed.empty()); }
    bool HasButtonUp() const { return (not _buttonsUp.empty()); }

    bool UseFilteredInputs() const { return _useFilteredInputs; }
    void SetUseFilteredInputs(bool value) { _useFilteredInputs = value; }

    void SetInside(bool inside);
    void SetPosition(int screenX, int screenY, int clientX, int clientY, float relativeX, float relativeY);
    void SetButtonDown(EMouseButton btn);
    void SetButtonUp(EMouseButton btn);
    void SetWheelDeltaX(float dx);
    void SetWheelDeltaY(float dy);

    void Update(FTimespan dt);
    void Clear();

private:
    bool _inside    : 1;
    bool _wasInside : 1;

    bool _onEnter   : 1;
    bool _onLeave   : 1;

    int _screenX{0}, _screenY{0};
    int _clientX{0}, _clientY{0};
    FFilteredAnalog _relativeX, _relativeY;

    int _prevClientX{0}, _prevClientY{0};
    float _prevRelativeRawX{0}, _prevRelativeRawY{0};
    float _prevRelativeFilteredX{0}, _prevRelativeFilteredY{0};

    int _deltaClientX{0}, _deltaClientY{0};
    float _deltaRelativeRawX{0}, _deltaRelativeRawY{0};
    float _deltaRelativeFilteredX{0}, _deltaRelativeFilteredY{0};

    FFilteredAnalog _wheelX;
    FFilteredAnalog _wheelY;

    FFilteredAnalog _wheelDeltaX;
    FFilteredAnalog _wheelDeltaY;
    float _wheelDeltaAccumX{0};
    float _wheelDeltaAccumY{0};

    FButtonState _buttonsDown;
    FButtonState _buttonsPressed;
    FButtonState _buttonsUp;
    FButtonState _buttonsQueued;

    bool _useFilteredInputs{ true };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
