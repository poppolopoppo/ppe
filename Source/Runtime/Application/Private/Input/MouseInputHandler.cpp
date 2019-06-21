#include "stdafx.h"

#include "Input/MouseInputHandler.h"

#include "HAL/PlatformMouse.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMouseInputHandler::FMouseInputHandler()
{}
//----------------------------------------------------------------------------
FMouseInputHandler::~FMouseInputHandler() = default;
//----------------------------------------------------------------------------
void FMouseInputHandler::SetupWindow(FPlatformWindow& window) {
    FPlatformMouse::SetupMessageHandler(window, &_state).Forget();
}
//----------------------------------------------------------------------------
void FMouseInputHandler::Update(FTimespan dt) {
    _state.Update(float(*dt));
}
//----------------------------------------------------------------------------
void FMouseInputHandler::Clear() {
    _state.Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
