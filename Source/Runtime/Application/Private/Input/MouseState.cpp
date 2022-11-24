// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Input/MouseState.h"

#include "Maths/ScalarVector.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
int2 FMouseState::Screen() const {
    return int2(_screenX, _screenY);
}
//----------------------------------------------------------------------------
int2 FMouseState::Client() const {
    return int2(_clientX, _clientY);
}
//----------------------------------------------------------------------------
float2 FMouseState::Relative() const {
    return RelativeFiltered();
}
//----------------------------------------------------------------------------
float2 FMouseState::RelativeRaw() const {
    return float2(_relativeX.Raw(), _relativeY.Raw());
}
//----------------------------------------------------------------------------
float2 FMouseState::RelativeFiltered() const {
    return float2(_relativeX.Filtered(), _relativeY.Filtered());
}
//----------------------------------------------------------------------------
int2 FMouseState::DeltaClient() const {
    return int2(_deltaClientX, _deltaClientY);
}
//----------------------------------------------------------------------------
float2 FMouseState::DeltaRelative() const {
    return DeltaRelativeFiltered();
}
//----------------------------------------------------------------------------
float2 FMouseState::DeltaRelativeRaw() const {
    return float2(_deltaRelativeRawX, _deltaRelativeRawY);
}
//----------------------------------------------------------------------------
float2 FMouseState::DeltaRelativeFiltered() const {
    return float2(_deltaRelativeFilteredX, _deltaRelativeFilteredY);
}
//----------------------------------------------------------------------------
void FMouseState::SetInside(bool inside) {
    _inside = inside;
}
//----------------------------------------------------------------------------
void FMouseState::SetPosition(
    int screenX, int screenY,
    int clientX, int clientY,
    float relativeX, float relativeY ) {
    _screenX = screenX;
    _screenY = screenY;

    _clientX = clientX;
    _clientY = clientY;

    _relativeX.SetRaw(relativeX);
    _relativeY.SetRaw(relativeY);
}
//----------------------------------------------------------------------------
void FMouseState::SetButtonDown(EMouseButton btn) {
    _buttonsQueued.Add_KeepExisting(btn);
}
//----------------------------------------------------------------------------
void FMouseState::SetButtonUp(EMouseButton btn) {
    _buttonsQueued.Remove_ReturnIfExists(btn);
}
//----------------------------------------------------------------------------
void FMouseState::SetWheelDeltaX(float delta) {
    _wheelDeltaAccumX += delta;
}
//----------------------------------------------------------------------------
void FMouseState::SetWheelDeltaY(float delta) {
    _wheelDeltaAccumY += delta;
}
//----------------------------------------------------------------------------
void FMouseState::Update(float dt) {
    _onEnter = (not _wasInside && _inside);
    _onLeave = (_wasInside && not _inside);
    _wasInside = _inside;

    _wheelDeltaX.SetRaw(_wheelDeltaAccumX);
    _wheelDeltaX.Update(dt);
    _wheelDeltaAccumX = 0;

    _wheelDeltaY.SetRaw(_wheelDeltaAccumY);
    _wheelDeltaY.Update(dt);
    _wheelDeltaAccumY = 0;

    const float rxRaw = _relativeX.Raw();
    const float ryRaw = _relativeY.Raw();

    const float rxFiltered = _relativeX.Filtered();
    const float ryFiltered = _relativeY.Filtered();

    _relativeX.Update(dt);
    _relativeY.Update(dt);

    _deltaClientX = (_clientX - _prevClientX);
    _deltaClientY = (_clientY - _prevClientY);

    _deltaRelativeRawX = (_relativeX.Raw() - rxRaw);
    _deltaRelativeRawY = (_relativeY.Raw() - ryRaw);

    _deltaRelativeFilteredX = (_relativeX.Filtered() - rxFiltered);
    _deltaRelativeFilteredY = (_relativeY.Filtered() - ryFiltered);

    _prevClientX = _clientX;
    _prevClientY = _clientY;
    _prevRelativeRawX = _relativeX.Raw();
    _prevRelativeRawY = _relativeY.Raw();
    _prevRelativeFilteredX = _relativeX.Filtered();
    _prevRelativeFilteredY = _relativeY.Filtered();

    // that way we never miss an input
    _buttonsPressed.Update(&_buttonsUp, &_buttonsDown, _buttonsQueued);
    _buttonsQueued = _buttonsPressed;
}
//----------------------------------------------------------------------------
void FMouseState::Clear() {
    _inside = _wasInside = false;
    _onEnter = _onLeave = false;

    _wheelDeltaX.Clear();
    _wheelDeltaAccumX = 0;

    _wheelDeltaY.Clear();
    _wheelDeltaAccumY = 0;

    _screenX = _screenY = 0;
    _clientX = _clientY = 0;
    _relativeX.Clear();
    _relativeY.Clear();

    _deltaClientX = _deltaClientY = 0;
    _deltaRelativeRawX = _deltaRelativeRawY = 0.f;
    _deltaRelativeFilteredX = _deltaRelativeFilteredY = 0.f;

    _prevClientX = _clientX;
    _prevClientY = _clientY;
    _prevRelativeRawX = _relativeX.Raw();
    _prevRelativeRawY = _relativeY.Raw();
    _prevRelativeFilteredX = _relativeX.Filtered();
    _prevRelativeFilteredY = _relativeY.Filtered();

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
