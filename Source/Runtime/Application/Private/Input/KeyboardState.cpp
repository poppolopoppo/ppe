#include "stdafx.h"

#include "Input/KeyboardState.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FKeyboardState::SetKeyDown(EKeyboardKey key) {
    _keysQueued.Add_KeepExisting(key);
}
//----------------------------------------------------------------------------
void FKeyboardState::SetKeyUp(EKeyboardKey key) {
    _keysQueued.Remove_ReturnIfExists(key);
}
//----------------------------------------------------------------------------
void FKeyboardState::Update(float dt) {
    Unused(dt); // key repeat ?

    // that way we never miss an input
    _keysPressed.Update(&_keysUp, &_keysDown, _keysQueued);
    _keysQueued = _keysPressed;
}
//----------------------------------------------------------------------------
void FKeyboardState::Clear() {
    _keysDown.Clear();
    _keysPressed.Clear();
    _keysUp.Clear();
    _keysQueued.Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
