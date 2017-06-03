#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/Input/InputState.h"
#include "Core.Application/Input/KeyboardKey.h"

namespace Core {
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
} //!namespace Core
