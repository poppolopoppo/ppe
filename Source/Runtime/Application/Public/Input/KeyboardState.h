#pragma once

#include "Application.h"

#include "Input/InputState.h"
#include "Input/KeyboardKey.h"

#include "Container/Stack.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FKeyboardState {
public:
    using FCharacterInput = u16; // utf-16
    using FInputStack = TFixedSizeStack<FCharacterInput, 16>;
    using FKeyState = TInputState<EKeyboardKey, 8>;

    const FKeyState& KeysDown() const { return _keysDown; }
    const FKeyState& KeysPressed() const { return _keysPressed; }
    const FKeyState& KeysUp() const { return _keysUp; }

    const FInputStack& CharacterInputs() const { return _charactersDown; }

    bool IsKeyDown(EKeyboardKey key) const { return _keysDown.Contains(key); }
    bool IsKeyPressed(EKeyboardKey key) const { return _keysPressed.Contains(key); }
    bool IsKeyUp(EKeyboardKey key) const { return _keysUp.Contains(key); }

    bool HasKeyDown() const { return (not _keysDown.empty()); }
    bool HasKeyPressed() const { return (not _keysPressed.empty()); }
    bool HasKeyUp() const { return (not _keysUp.empty()); }

    void SetKeyDown(EKeyboardKey key);
    void SetKeyUp(EKeyboardKey key);

    void AddCharacterInput(FCharacterInput utf16);

    void Update(float dt);
    void Clear();

private:
    FKeyState _keysDown;
    FKeyState _keysPressed;
    FKeyState _keysUp;
    FKeyState _keysQueued;

    FInputStack _charactersDown;
    FInputStack _charactersQueued;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
