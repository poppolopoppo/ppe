// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Input/Device/KeyboardState.h"

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
void FKeyboardState::AddCharacterInput(FCharacterInput utf16) {
    _charactersQueued.Push(utf16);
}
//----------------------------------------------------------------------------
void FKeyboardState::Update(FTimespan dt) {
    Unused(dt); // key repeat ?

    // that way we never miss an input
    _keysPressed.Update(&_keysUp, &_keysDown, _keysQueued);
    _keysQueued = _keysPressed;

    _charactersDown = _charactersQueued;
    _charactersQueued.clear();
}
//----------------------------------------------------------------------------
void FKeyboardState::Clear() {
    _keysDown.Clear();
    _keysPressed.Clear();
    _keysUp.Clear();
    _keysQueued.Clear();

    _charactersDown.clear();
    _charactersQueued.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
