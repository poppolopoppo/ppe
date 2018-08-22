#include "stdafx.h"

#include "Input/KeyboardInputHandler.h"

#include "HAL/PlatformKeyboard.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FKeyboardInputHandler::FKeyboardInputHandler()
{}
//----------------------------------------------------------------------------
FKeyboardInputHandler::~FKeyboardInputHandler()
{}
//----------------------------------------------------------------------------
void FKeyboardInputHandler::SetupWindow(FPlatformWindow& window) {
    FPlatformKeyboard::SetupMessageHandler(window, &_state).Forget();
}
//----------------------------------------------------------------------------
void FKeyboardInputHandler::Update(FTimespan dt) {
    _state.Update(float(*dt));
}
//----------------------------------------------------------------------------
void FKeyboardInputHandler::Clear() {
    _state.Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
