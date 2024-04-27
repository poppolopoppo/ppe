// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Input/InputKey.h"

#include "Input/GamepadButton.h"
#include "Input/KeyboardKey.h"
#include "Input/MouseButton.h"
#include "Meta/Functor.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static constexpr EInputValueType GEachInputValueType[] = {
    EInputValueType::Digital,
    EInputValueType::Axis1D,
    EInputValueType::Axis2D,
    EInputValueType::Axis3D,
};
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, const FInputKey& value) {
    oss << value.Name
        << STRING_LITERAL(_Char, " : <")
        << value.ValueType
        << STRING_LITERAL(_Char, "> = ");
    Meta::Visit(value.Code,
        [&oss](auto key) {
            oss << key;
        },
        [&oss](std::monostate) {
            oss << "nil";
        });
    return oss;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FInputKey::FInputKey(FStringLiteral name, FInputSourceVariant&& source, EInputValueType valueType) NOEXCEPT
:   Name(name)
,   Code(source)
,   ValueType(valueType) {
    Assert_NoAssume(not Name.empty());
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FInputKey& value) {
    return ToString_(oss, value);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FInputKey& value) {
    return ToString_(oss, value);
}
//----------------------------------------------------------------------------
TMemoryView<const EInputValueType> EachInputValueType() NOEXCEPT {
    return GEachInputValueType;
}
//----------------------------------------------------------------------------
FStringLiteral InputValueTypeCStr(EInputValueType value) NOEXCEPT {
    switch (value) {
    case EInputValueType::Digital: return "Digital";
    case EInputValueType::Axis1D: return "Axis1D";
    case EInputValueType::Axis2D: return "Axis2D";
    case EInputValueType::Axis3D: return "Axis3D";
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Meta::TOptionalReference<const FInputKey> EInputKey::From(EKeyboardKey value) NOEXCEPT {
    switch (value) {
    case EKeyboardKey::_0: return &EInputKey::Zero;
    case EKeyboardKey::_1: return &EInputKey::One;
    case EKeyboardKey::_2: return &EInputKey::Two;
    case EKeyboardKey::_3: return &EInputKey::Three;
    case EKeyboardKey::_4: return &EInputKey::Four;
    case EKeyboardKey::_5: return &EInputKey::Five;
    case EKeyboardKey::_6: return &EInputKey::Six;
    case EKeyboardKey::_7: return &EInputKey::Seven;
    case EKeyboardKey::_8: return &EInputKey::Eight;
    case EKeyboardKey::_9: return &EInputKey::Nine;
    case EKeyboardKey::A: return &EInputKey::A;
    case EKeyboardKey::B: return &EInputKey::B;
    case EKeyboardKey::C: return &EInputKey::C;
    case EKeyboardKey::D: return &EInputKey::D;
    case EKeyboardKey::E: return &EInputKey::E;
    case EKeyboardKey::F: return &EInputKey::F;
    case EKeyboardKey::G: return &EInputKey::G;
    case EKeyboardKey::H: return &EInputKey::H;
    case EKeyboardKey::I: return &EInputKey::I;
    case EKeyboardKey::J: return &EInputKey::J;
    case EKeyboardKey::K: return &EInputKey::K;
    case EKeyboardKey::L: return &EInputKey::L;
    case EKeyboardKey::M: return &EInputKey::M;
    case EKeyboardKey::N: return &EInputKey::N;
    case EKeyboardKey::O: return &EInputKey::O;
    case EKeyboardKey::P: return &EInputKey::P;
    case EKeyboardKey::Q: return &EInputKey::Q;
    case EKeyboardKey::R: return &EInputKey::R;
    case EKeyboardKey::S: return &EInputKey::S;
    case EKeyboardKey::_T: return &EInputKey::T;
    case EKeyboardKey::U: return &EInputKey::U;
    case EKeyboardKey::V: return &EInputKey::V;
    case EKeyboardKey::W: return &EInputKey::W;
    case EKeyboardKey::X: return &EInputKey::X;
    case EKeyboardKey::Y: return &EInputKey::Y;
    case EKeyboardKey::Z: return &EInputKey::Z;
    case EKeyboardKey::Numpad0: return &EInputKey::NumPadZero;
    case EKeyboardKey::Numpad1: return &EInputKey::NumPadOne;
    case EKeyboardKey::Numpad2: return &EInputKey::NumPadTwo;
    case EKeyboardKey::Numpad3: return &EInputKey::NumPadThree;
    case EKeyboardKey::Numpad4: return &EInputKey::NumPadFour;
    case EKeyboardKey::Numpad5: return &EInputKey::NumPadFive;
    case EKeyboardKey::Numpad6: return &EInputKey::NumPadSix;
    case EKeyboardKey::Numpad7: return &EInputKey::NumPadSeven;
    case EKeyboardKey::Numpad8: return &EInputKey::NumPadEight;
    case EKeyboardKey::Numpad9: return &EInputKey::NumPadNine;
    case EKeyboardKey::F1: return &EInputKey::F1;
    case EKeyboardKey::F2: return &EInputKey::F2;
    case EKeyboardKey::F3: return &EInputKey::F3;
    case EKeyboardKey::F4: return &EInputKey::F4;
    case EKeyboardKey::F5: return &EInputKey::F5;
    case EKeyboardKey::F6: return &EInputKey::F6;
    case EKeyboardKey::F7: return &EInputKey::F7;
    case EKeyboardKey::F8: return &EInputKey::F8;
    case EKeyboardKey::F9: return &EInputKey::F9;
    case EKeyboardKey::F10: return &EInputKey::F10;
    case EKeyboardKey::F11: return &EInputKey::F11;
    case EKeyboardKey::F12: return &EInputKey::F12;
    case EKeyboardKey::UpArrow: return &EInputKey::UpArrow;
    case EKeyboardKey::DownArrow: return &EInputKey::DownArrow;
    case EKeyboardKey::LeftArrow: return &EInputKey::LeftArrow;
    case EKeyboardKey::RightArrow: return &EInputKey::RightArrow;
    case EKeyboardKey::Escape: return &EInputKey::Escape;
    case EKeyboardKey::Space: return &EInputKey::SpaceBar;
    case EKeyboardKey::Pause: return &EInputKey::Pause;
    case EKeyboardKey::PrintScreen: return &EInputKey::PrintScreen;
    case EKeyboardKey::ScrollLock: return &EInputKey::ScrollLock;
    case EKeyboardKey::Backspace: return &EInputKey::BackSpace;
    case EKeyboardKey::Enter: return &EInputKey::Enter;
    case EKeyboardKey::Tab: return &EInputKey::Tab;
    case EKeyboardKey::Home: return &EInputKey::Home;
    case EKeyboardKey::End: return &EInputKey::End;
    case EKeyboardKey::Insert: return &EInputKey::Insert;
    case EKeyboardKey::Delete: return &EInputKey::Delete;
    case EKeyboardKey::PageUp: return &EInputKey::PageUp;
    case EKeyboardKey::PageDown: return &EInputKey::PageDown;
    case EKeyboardKey::Comma: return &EInputKey::Decimal;
    case EKeyboardKey::Equals: return &EInputKey::Equals;
    case EKeyboardKey::Plus: return &EInputKey::Add;
    case EKeyboardKey::Minus: return &EInputKey::Subtract;
    case EKeyboardKey::Period: return &EInputKey::Period;
    case EKeyboardKey::Semicolon: return &EInputKey::Semicolon;
    case EKeyboardKey::Underscore: return &EInputKey::Underscore;
    case EKeyboardKey::Ampersand: return &EInputKey::Ampersand;
    case EKeyboardKey::Apostrophe: return &EInputKey::Apostrophe;
    case EKeyboardKey::Caret: return &EInputKey::Caret;
    case EKeyboardKey::Colon: return &EInputKey::Colon;
    case EKeyboardKey::Dollar: return &EInputKey::Dollar;
    case EKeyboardKey::Exclamation: return &EInputKey::Exclamation;
    case EKeyboardKey::Tilde: return &EInputKey::Tilde;
    case EKeyboardKey::Quote: return &EInputKey::Quote;
    case EKeyboardKey::Slash: return &EInputKey::Divide;
    case EKeyboardKey::Backslash: return &EInputKey::Backslash;
    case EKeyboardKey::LeftBracket: return &EInputKey::LeftBracket;
    case EKeyboardKey::RightBracket: return &EInputKey::RightBracket;
    case EKeyboardKey::LeftParantheses: return &EInputKey::LeftParantheses;
    case EKeyboardKey::RightParantheses: return &EInputKey::RightParantheses;
    case EKeyboardKey::CapsLock: return &EInputKey::CapsLock;
    case EKeyboardKey::NumLock: return &EInputKey::NumLock;
    case EKeyboardKey::LeftAlt: return &EInputKey::LeftAlt;
    case EKeyboardKey::RightAlt: return &EInputKey::RightAlt;
    case EKeyboardKey::LeftControl: return &EInputKey::LeftControl;
    case EKeyboardKey::RightControl: return &EInputKey::RightControl;
    case EKeyboardKey::LeftShift: return &EInputKey::LeftShift;
    case EKeyboardKey::RightShift: return &EInputKey::RightShift;
    case EKeyboardKey::LeftSuper: return &EInputKey::LeftSuper;
    case EKeyboardKey::RightSuper: return &EInputKey::RightSuper;
    case EKeyboardKey::Asterix: return &EInputKey::Multiply;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
Meta::TOptionalReference<const FInputKey> EInputKey::From(EGamepadAxis value) NOEXCEPT {
    switch (value) {
    case EGamepadAxis::LeftStick: return &Gamepad_Left2D;
    case EGamepadAxis::RightStick: return &Gamepad_Right2D;
    case EGamepadAxis::LeftTrigger: return &Gamepad_LeftTriggerAxis;
    case EGamepadAxis::RightTrigger: return &Gamepad_RightTriggerAxis;
    default:
        return nullptr;
    }
}
//----------------------------------------------------------------------------
Meta::TOptionalReference<const FInputKey> EInputKey::From(EGamepadButton value) NOEXCEPT {
    switch (value) {
    case EGamepadButton::Button0: return &Gamepad_FaceButton_Bottom;
    case EGamepadButton::Button1: return &Gamepad_FaceButton_Right;
    case EGamepadButton::Button2: return &Gamepad_FaceButton_Left;
    case EGamepadButton::Button3: return &Gamepad_FaceButton_Top;
    case EGamepadButton::DPadUp: return &Gamepad_DPad_Up;
    case EGamepadButton::DPadLeft: return &Gamepad_DPad_Left;
    case EGamepadButton::DPadRight: return &Gamepad_DPad_Right;
    case EGamepadButton::DPadDown: return &Gamepad_DPad_Down;
    case EGamepadButton::LeftThumb: return &Gamepad_LeftThumbstick;
    case EGamepadButton::RightThumb: return &Gamepad_RightThumbstick;
    case EGamepadButton::LeftShoulder: return &Gamepad_LeftShoulder;
    case EGamepadButton::RightShoulder: return &Gamepad_RightShoulder;
    case EGamepadButton::Start: return &Gamepad_Start;
    case EGamepadButton::Back: return &Gamepad_Back;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
Meta::TOptionalReference<const FInputKey> EInputKey::From(EMouseAxis value) NOEXCEPT {
    switch (value) {
    case EMouseAxis::Pointer: return &Mouse2D;
    case EMouseAxis::ScrollWheelY: return &MouseWheelAxisY;
    case EMouseAxis::ScrollWheelX: return &MouseWheelAxisX;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
Meta::TOptionalReference<const FInputKey> EInputKey::From(EMouseButton value) NOEXCEPT {
    switch (value) {
    case EMouseButton::Thumb0: return &ThumbMouseButton;
    case EMouseButton::Thumb1: return &ThumbMouseButton2;
    case EMouseButton::Left: return &LeftMouseButton;
    case EMouseButton::Right: return &RightMouseButton;
    case EMouseButton::Middle: return &MiddleMouseButton;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
void EInputKey::EachKey(TAppendable<FInputKey> keys) NOEXCEPT {
    // keys.push_back(AnyKey); // do *NOT* enumerate AnyKey here, we only want actual keys
    keys.push_back(Mouse2D);
    keys.push_back(MouseWheelAxisX);
    keys.push_back(MouseWheelAxisY);
    keys.push_back(LeftMouseButton);
    keys.push_back(RightMouseButton);
    keys.push_back(MiddleMouseButton);
    keys.push_back(ThumbMouseButton);
    keys.push_back(ThumbMouseButton2);
    keys.push_back(BackSpace);
    keys.push_back(Tab);
    keys.push_back(Enter);
    keys.push_back(Pause);
    keys.push_back(CapsLock);
    keys.push_back(Escape);
    keys.push_back(SpaceBar);
    keys.push_back(PageUp);
    keys.push_back(PageDown);
    keys.push_back(End);
    keys.push_back(Home);
    keys.push_back(LeftArrow);
    keys.push_back(UpArrow);
    keys.push_back(RightArrow);
    keys.push_back(DownArrow);
    keys.push_back(Insert);
    keys.push_back(Delete);
    keys.push_back(Zero);
    keys.push_back(One);
    keys.push_back(Two);
    keys.push_back(Three);
    keys.push_back(Four);
    keys.push_back(Five);
    keys.push_back(Six);
    keys.push_back(Seven);
    keys.push_back(Eight);
    keys.push_back(Nine);
    keys.push_back(A);
    keys.push_back(B);
    keys.push_back(C);
    keys.push_back(D);
    keys.push_back(E);
    keys.push_back(F);
    keys.push_back(G);
    keys.push_back(H);
    keys.push_back(I);
    keys.push_back(J);
    keys.push_back(K);
    keys.push_back(L);
    keys.push_back(M);
    keys.push_back(N);
    keys.push_back(O);
    keys.push_back(P);
    keys.push_back(Q);
    keys.push_back(R);
    keys.push_back(S);
    keys.push_back(T);
    keys.push_back(U);
    keys.push_back(V);
    keys.push_back(W);
    keys.push_back(X);
    keys.push_back(Y);
    keys.push_back(Z);
    keys.push_back(NumPadZero);
    keys.push_back(NumPadOne);
    keys.push_back(NumPadTwo);
    keys.push_back(NumPadThree);
    keys.push_back(NumPadFour);
    keys.push_back(NumPadFive);
    keys.push_back(NumPadSix);
    keys.push_back(NumPadSeven);
    keys.push_back(NumPadEight);
    keys.push_back(NumPadNine);
    keys.push_back(Multiply);
    keys.push_back(Add);
    keys.push_back(Subtract);
    keys.push_back(Decimal);
    keys.push_back(Divide);
    keys.push_back(F1);
    keys.push_back(F2);
    keys.push_back(F3);
    keys.push_back(F4);
    keys.push_back(F5);
    keys.push_back(F6);
    keys.push_back(F7);
    keys.push_back(F8);
    keys.push_back(F9);
    keys.push_back(F10);
    keys.push_back(F11);
    keys.push_back(F12);
    keys.push_back(NumLock);
    keys.push_back(PrintScreen);
    keys.push_back(ScrollLock);
    keys.push_back(LeftShift);
    keys.push_back(RightShift);
    keys.push_back(LeftControl);
    keys.push_back(RightControl);
    keys.push_back(LeftAlt);
    keys.push_back(RightAlt);
    keys.push_back(LeftSuper);
    keys.push_back(RightSuper);
    keys.push_back(Semicolon);
    keys.push_back(Equals);
    keys.push_back(Underscore);
    keys.push_back(Period);
    keys.push_back(Tilde);
    keys.push_back(LeftBracket);
    keys.push_back(Backslash);
    keys.push_back(RightBracket);
    keys.push_back(Apostrophe);
    keys.push_back(Ampersand);
    keys.push_back(Caret);
    keys.push_back(Colon);
    keys.push_back(Dollar);
    keys.push_back(Exclamation);
    keys.push_back(LeftParantheses);
    keys.push_back(RightParantheses);
    keys.push_back(Quote);
    keys.push_back(Gamepad_Left2D);
    keys.push_back(Gamepad_Right2D);
    keys.push_back(Gamepad_LeftTriggerAxis);
    keys.push_back(Gamepad_RightTriggerAxis);
    keys.push_back(Gamepad_LeftThumbstick);
    keys.push_back(Gamepad_RightThumbstick);
    keys.push_back(Gamepad_FaceButton_Bottom);
    keys.push_back(Gamepad_FaceButton_Right);
    keys.push_back(Gamepad_FaceButton_Left);
    keys.push_back(Gamepad_FaceButton_Top);
    keys.push_back(Gamepad_LeftShoulder);
    keys.push_back(Gamepad_RightShoulder);
    keys.push_back(Gamepad_DPad_Up);
    keys.push_back(Gamepad_DPad_Down);
    keys.push_back(Gamepad_DPad_Right);
    keys.push_back(Gamepad_DPad_Left);
    keys.push_back(Gamepad_Start);
    keys.push_back(Gamepad_Back);
}
//----------------------------------------------------------------------------
/*static*/ const FInputKey EInputKey::AnyKey{ "AnyKey", std::monostate{}, Default };
/*static*/ const FInputKey EInputKey::Mouse2D{ "Mouse2D", EMouseAxis::Pointer, EInputValueType::Axis2D };
/*static*/ const FInputKey EInputKey::MouseWheelAxisX{ "MouseWheelAxisX", EMouseAxis::ScrollWheelX, EInputValueType::Axis1D };
/*static*/ const FInputKey EInputKey::MouseWheelAxisY{ "MouseWheelAxisY", EMouseAxis::ScrollWheelY, EInputValueType::Axis1D };
/*static*/ const FInputKey EInputKey::LeftMouseButton{ "LeftMouseButton", EMouseButton::Left, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::RightMouseButton{ "RightMouseButton", EMouseButton::Right, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::MiddleMouseButton{ "MiddleMouseButton", EMouseButton::Middle, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::ThumbMouseButton{ "ThumbMouseButton", EMouseButton::Thumb0, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::ThumbMouseButton2{ "ThumbMouseButton2", EMouseButton::Thumb1, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::BackSpace{ "BackSpace", EKeyboardKey::Backspace, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Tab{ "Tab", EKeyboardKey::Tab, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Enter{ "Enter", EKeyboardKey::Enter, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Pause{ "Pause", EKeyboardKey::Pause, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::CapsLock{ "CapsLock", EKeyboardKey::CapsLock, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Escape{ "Escape", EKeyboardKey::Escape, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::SpaceBar{ "SpaceBar", EKeyboardKey::Space, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::PageUp{ "PageUp", EKeyboardKey::PageUp, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::PageDown{ "PageDown", EKeyboardKey::PageDown, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::End{ "End", EKeyboardKey::End, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Home{ "Home", EKeyboardKey::Home, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::LeftArrow{ "LeftArrow", EKeyboardKey::LeftArrow, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::UpArrow{ "UpArrow", EKeyboardKey::UpArrow, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::RightArrow{ "RightArrow", EKeyboardKey::RightArrow, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::DownArrow{ "DownArrow", EKeyboardKey::DownArrow, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Insert{ "Insert", EKeyboardKey::Insert, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Delete{ "Delete", EKeyboardKey::Delete, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Zero{ "Zero", EKeyboardKey::_0, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::One{ "One", EKeyboardKey::_1, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Two{ "Two", EKeyboardKey::_2, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Three{ "Three", EKeyboardKey::_3, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Four{ "Four", EKeyboardKey::_4, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Five{ "Five", EKeyboardKey::_5, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Six{ "Six", EKeyboardKey::_6, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Seven{ "Seven", EKeyboardKey::_7, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Eight{ "Eight", EKeyboardKey::_8, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Nine{ "Nine", EKeyboardKey::_9, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::A{ "A", EKeyboardKey::A, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::B{ "B", EKeyboardKey::B, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::C{ "C", EKeyboardKey::C, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::D{ "D", EKeyboardKey::D, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::E{ "E", EKeyboardKey::E, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::F{ "F", EKeyboardKey::F, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::G{ "G", EKeyboardKey::G, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::H{ "H", EKeyboardKey::H, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::I{ "I", EKeyboardKey::I, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::J{ "J", EKeyboardKey::J, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::K{ "K", EKeyboardKey::K, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::L{ "L", EKeyboardKey::L, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::M{ "M", EKeyboardKey::M, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::N{ "N", EKeyboardKey::N, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::O{ "O", EKeyboardKey::O, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::P{ "P", EKeyboardKey::P, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Q{ "Q", EKeyboardKey::Q, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::R{ "R", EKeyboardKey::R, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::S{ "S", EKeyboardKey::S, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::T{ "T", EKeyboardKey::_T, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::U{ "U", EKeyboardKey::U, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::V{ "V", EKeyboardKey::V, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::W{ "W", EKeyboardKey::W, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::X{ "X", EKeyboardKey::X, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Y{ "Y", EKeyboardKey::Y, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Z{ "Z", EKeyboardKey::Z, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::NumPadZero{ "NumPadZero", EKeyboardKey::Numpad0, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::NumPadOne{ "NumPadOne", EKeyboardKey::Numpad1, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::NumPadTwo{ "NumPadTwo", EKeyboardKey::Numpad2, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::NumPadThree{ "NumPadThree", EKeyboardKey::Numpad3, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::NumPadFour{ "NumPadFour", EKeyboardKey::Numpad4, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::NumPadFive{ "NumPadFive", EKeyboardKey::Numpad5, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::NumPadSix{ "NumPadSix", EKeyboardKey::Numpad6, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::NumPadSeven{ "NumPadSeven", EKeyboardKey::Numpad7, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::NumPadEight{ "NumPadEight", EKeyboardKey::Numpad8, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::NumPadNine{ "NumPadNine", EKeyboardKey::Numpad9, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Multiply{ "Multiply", EKeyboardKey::Asterix, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Add{ "Add", EKeyboardKey::Plus, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Subtract{ "Subtract", EKeyboardKey::Minus, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Decimal{ "Decimal", EKeyboardKey::Comma, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Divide{ "Divide", EKeyboardKey::Slash, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::F1{ "F1", EKeyboardKey::F1, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::F2{ "F2", EKeyboardKey::F2, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::F3{ "F3", EKeyboardKey::F3, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::F4{ "F4", EKeyboardKey::F4, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::F5{ "F5", EKeyboardKey::F5, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::F6{ "F6", EKeyboardKey::F6, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::F7{ "F7", EKeyboardKey::F7, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::F8{ "F8", EKeyboardKey::F8, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::F9{ "F9", EKeyboardKey::F9, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::F10{ "F10", EKeyboardKey::F10, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::F11{ "F11", EKeyboardKey::F11, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::F12{ "F12", EKeyboardKey::F12, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::NumLock{ "NumLock", EKeyboardKey::NumLock, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::PrintScreen{ "PrintScreen", EKeyboardKey::PrintScreen, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::ScrollLock{ "ScrollLock", EKeyboardKey::ScrollLock, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::LeftShift{ "LeftShift", EKeyboardKey::LeftShift, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::RightShift{ "RightShift", EKeyboardKey::RightShift, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::LeftControl{ "LeftControl", EKeyboardKey::LeftControl, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::RightControl{ "RightControl", EKeyboardKey::RightControl, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::LeftAlt{ "LeftAlt", EKeyboardKey::LeftAlt, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::RightAlt{ "RightAlt", EKeyboardKey::RightAlt, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::LeftSuper{ "LeftSuper", EKeyboardKey::LeftSuper, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::RightSuper{ "RightSuper", EKeyboardKey::RightSuper, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Semicolon{ "Semicolon", EKeyboardKey::Semicolon, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Equals{ "Equals", EKeyboardKey::Equals, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Underscore{ "Underscore", EKeyboardKey::Underscore, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Period{ "Period", EKeyboardKey::Period, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Tilde{ "Tilde", EKeyboardKey::Tilde, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::LeftBracket{ "LeftBracket", EKeyboardKey::LeftBracket, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Backslash{ "Backslash", EKeyboardKey::Backslash, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::RightBracket{ "RightBracket", EKeyboardKey::RightBracket, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Apostrophe{ "Apostrophe", EKeyboardKey::Apostrophe, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Ampersand{ "Ampersand", EKeyboardKey::Ampersand, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Caret{ "Caret", EKeyboardKey::Caret, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Colon{ "Colon", EKeyboardKey::Colon, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Dollar{ "Dollar", EKeyboardKey::Dollar, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Exclamation{ "Exclamation", EKeyboardKey::Exclamation, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::LeftParantheses{ "LeftParantheses", EKeyboardKey::LeftParantheses, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::RightParantheses{ "RightParantheses", EKeyboardKey::RightParantheses, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Quote{ "Quote", EKeyboardKey::Quote, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Gamepad_Left2D{ "Gamepad_Left2D", EGamepadAxis::LeftStick, EInputValueType::Axis2D };
/*static*/ const FInputKey EInputKey::Gamepad_Right2D{ "Gamepad_Right2D", EGamepadAxis::RightStick, EInputValueType::Axis2D };
/*static*/ const FInputKey EInputKey::Gamepad_LeftTriggerAxis{ "Gamepad_LeftTriggerAxis", EGamepadAxis::LeftTrigger, EInputValueType::Axis1D };
/*static*/ const FInputKey EInputKey::Gamepad_RightTriggerAxis{ "Gamepad_RightTriggerAxis", EGamepadAxis::RightTrigger, EInputValueType::Axis1D };
/*static*/ const FInputKey EInputKey::Gamepad_LeftThumbstick{ "Gamepad_LeftThumbstick", EGamepadButton::LeftThumb, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Gamepad_RightThumbstick{ "Gamepad_RightThumbstick", EGamepadButton::RightThumb, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Gamepad_FaceButton_Bottom{ "Gamepad_FaceButton_Bottom", EGamepadButton::Button0, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Gamepad_FaceButton_Right{ "Gamepad_FaceButton_Right", EGamepadButton::Button1, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Gamepad_FaceButton_Left{ "Gamepad_FaceButton_Left", EGamepadButton::Button2, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Gamepad_FaceButton_Top{ "Gamepad_FaceButton_Top", EGamepadButton::Button3, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Gamepad_LeftShoulder{ "Gamepad_LeftShoulder", EGamepadButton::LeftShoulder, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Gamepad_RightShoulder{ "Gamepad_RightShoulder", EGamepadButton::RightShoulder, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Gamepad_DPad_Up{ "Gamepad_DPad_Up", EGamepadButton::DPadUp, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Gamepad_DPad_Down{ "Gamepad_DPad_Down", EGamepadButton::DPadDown, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Gamepad_DPad_Right{ "Gamepad_DPad_Right", EGamepadButton::DPadRight, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Gamepad_DPad_Left{ "Gamepad_DPad_Left", EGamepadButton::DPadLeft, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Gamepad_Start{ "Gamepad_Start", EGamepadButton::Start, EInputValueType::Digital };
/*static*/ const FInputKey EInputKey::Gamepad_Back{ "Gamepad_Back", EGamepadButton::Back, EInputValueType::Digital };
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!n " //!", EKeyboardKey{}amespace Applic::amespace, EInputValueType::Digitalion
} //!namespace PPE
