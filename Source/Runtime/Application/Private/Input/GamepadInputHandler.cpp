#include "stdafx.h"

#include "Input/GamepadInputHandler.h"

#include "HAL/PlatformGamepad.h"
#include "Thread/ThreadPool.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGamepadInputHandler::FGamepadInputHandler()
:   _firstConnected(INDEX_NONE) {
    _states.resize_AssumeEmpty(FPlatformGamepad::MaxNumGamepad);
}
//----------------------------------------------------------------------------
FGamepadInputHandler::~FGamepadInputHandler() = default;
//----------------------------------------------------------------------------
const FGamepadState* FGamepadInputHandler::FirstConnectedIFP() const {
    return (INDEX_NONE != _firstConnected
        ? &_states[_firstConnected]
        : nullptr );
}
//----------------------------------------------------------------------------
const FGamepadState& FGamepadInputHandler::State(size_t index) const {
    Assert(index < FPlatformGamepad::MaxNumGamepad);

    return _states[index];
}
//----------------------------------------------------------------------------
void FGamepadInputHandler::Poll() {
    forrange(controllerId, 0, FPlatformGamepad::MaxNumGamepad) {
        FGamepadState& gamepad = _states[controllerId];
        FPlatformGamepad::Poll(controllerId, &gamepad);
    }
}
//----------------------------------------------------------------------------
void FGamepadInputHandler::Update(FTimespan dt) {
    _firstConnected = INDEX_NONE;

    forrange(controllerId, 0, FPlatformGamepad::MaxNumGamepad) {
        FGamepadState& gamepad = _states[controllerId];
        gamepad.Update(float(*dt));

        if (INDEX_NONE == _firstConnected && gamepad.OnConnect())
            _firstConnected = controllerId;
    }
}
//----------------------------------------------------------------------------
void FGamepadInputHandler::Rumble(size_t index, float left, float right) {
    Assert(index < _states.size());

    if (not _states[index].IsConnected())
        return;

    // non blocking rumble :
    AsyncBackround([=](ITaskContext&) {
        FPlatformGamepad::Rumble(index, left, right);
    },  ETaskPriority::High);
}
//----------------------------------------------------------------------------
void FGamepadInputHandler::Clear() {
    for (FGamepadState& gamepad : _states)
        gamepad.Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
