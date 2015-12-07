#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/Input/InputState.h"
#include "Core.Application/Input/KeyboardKey.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class KeyboardState {
public:
    friend class KeyboardInputHandler;

    const KeyboardKeyState& KeysDown() const { return _keysDown; }
    const KeyboardKeyState& KeysPressed() const { return _keysPressed; }
    const KeyboardKeyState& KeysUp() const { return _keysUp; }

    bool IsKeyDown(KeyboardKey key) const { return _keysDown.Contains(key); }
    bool IsKeyPressed(KeyboardKey key) const { return _keysPressed.Contains(key); }
    bool IsKeyUp(KeyboardKey key) const { return _keysUp.Contains(key); }

    void Clear() {
        _keysDown.Clear();
        _keysPressed.Clear();
        _keysUp.Clear();
    }

private:
    KeyboardKeyState _keysDown;
    KeyboardKeyState _keysPressed;
    KeyboardKeyState _keysUp;
};
//----------------------------------------------------------------------------
typedef IInputStateProvider<KeyboardState> IKeyboardService;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
