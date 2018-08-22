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
    using FKeyState = TInputState<EKeyboardKey, 8>;

    const FKeyState& KeysDown() const { return _keysDown; }
    const FKeyState& KeysPressed() const { return _keysPressed; }
    const FKeyState& KeysUp() const { return _keysUp; }

    bool IsKeyDown(EKeyboardKey key) const { return _keysDown.Contains(key); }
    bool IsKeyPressed(EKeyboardKey key) const { return _keysPressed.Contains(key); }
    bool IsKeyUp(EKeyboardKey key) const { return _keysUp.Contains(key); }

    bool HasKeyDown() const { return (not _keysDown.empty()); }
    bool HasKeyPressed() const { return (not _keysPressed.empty()); }
    bool HasKeyUp() const { return (not _keysUp.empty()); }

    void SetKeyDown(EKeyboardKey key);
    void SetKeyUp(EKeyboardKey key);

    void Update(float dt);
    void Clear();

private:
    FKeyState _keysDown;
    FKeyState _keysPressed;
    FKeyState _keysUp;
    FKeyState _keysQueued;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
