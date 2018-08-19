#pragma once

#include "Application_fwd.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericCursorType {
    AppStarting,
    Arrow,
    Cross,
    Default,
    Hand,
    Help,
    HSplit,
    IBeam,
    No,
    NoMove2D,
    NoMoveHoriz,
    NoMoveVert,
    PanEast,
    PanNE,
    PanNorth,
    PanNW,
    PanSE,
    PanSouth,
    PanSW,
    PanWest,
    SizeAll,
    SizeNESW,
    SizeNS,
    SizeNWSE,
    SizeWE,
    UpArrow,
    VSplit,
    WaitCursor,
};
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FGenericPlatformMouse {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasKeyboard, false);

    using ECursorType = EGenericCursorType;

    static ECursorType CursorType() = delete;
    static void SetCursorType(ECursorType type) = delete;

    static bool Visible() = delete;
    static void SetVisible(bool value) = delete;

    static void ResetCapture() = delete;
    static void SetCapture(FGenericWindow& window) = delete;

    static bool Poll(FGenericWindow& window, FMouseState* mouse) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
