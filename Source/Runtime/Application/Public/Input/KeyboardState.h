#pragma once

#include "Application.h"

#include "Input/InputState.h"
#include "Input/KeyboardKey.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FKeyboardState {
public:
    friend class FKeyboardInputHandler;

    const FKeyboardKeyState& KeysDown() const { return _keysDown; }
    const FKeyboardKeyState& KeysPressed() const { return _keysPressed; }
    const FKeyboardKeyState& KeysUp() const { return _keysUp; }

    bool IsKeyDown(EKeyboardKey key) const { return _keysDown.Contains(key); }
    bool IsKeyPressed(EKeyboardKey key) const { return _keysPressed.Contains(key); }
    bool IsKeyUp(EKeyboardKey key) const { return _keysUp.Contains(key); }

    bool HasKeyDown() const { return (not _keysDown.empty()); }
    bool HasKeyPressed() const { return (not _keysPressed.empty()); }
    bool HasKeyUp() const { return (not _keysUp.empty()); }

    void Clear() {
        _keysDown.Clear();
        _keysPressed.Clear();
        _keysUp.Clear();
    }

private:
    FKeyboardKeyState _keysDown;
    FKeyboardKeyState _keysPressed;
    FKeyboardKeyState _keysUp;
};
//----------------------------------------------------------------------------
typedef IInputStateProvider<FKeyboardState> IKeyboardService;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE