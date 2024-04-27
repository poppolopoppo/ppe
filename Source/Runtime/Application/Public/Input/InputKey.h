#pragma once

#include "Application_fwd.h"

#include "IO/StringView.h"
#include "Maths/ScalarVector.h"

#include <variant>

#include "Container/Appendable.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EInputValueType : u8 {
    Digital = 0,
    Axis1D,
    Axis2D,
    Axis3D,

    Unknown = Digital,
};
//----------------------------------------------------------------------------
NODISCARD PPE_APPLICATION_API TMemoryView<const EInputValueType> EachInputValueType() NOEXCEPT;
NODISCARD PPE_APPLICATION_API FStringLiteral InputValueTypeCStr(EInputValueType value) NOEXCEPT;
//----------------------------------------------------------------------------
using FInputSourceVariant = std::variant<
    std::monostate,
    EGamepadAxis,
    EGamepadButton,
    EKeyboardKey,
    EMouseAxis,
    EMouseButton>;
//----------------------------------------------------------------------------
struct FInputKey {
    FStringLiteral Name;
    FInputSourceVariant Code;
    EInputValueType ValueType{ Default };

    FInputKey() = default;

    PPE_APPLICATION_API FInputKey(
        FStringLiteral name,
        FInputSourceVariant&& source,
        EInputValueType valueType ) NOEXCEPT;

    NODISCARD bool IsAnalog() const { return (ValueType != EInputValueType::Digital); }
    NODISCARD bool IsDigital() const { return (ValueType == EInputValueType::Digital); }

    NODISCARD bool IsGamepadKey() const { return (std::get_if<EGamepadAxis>(&Code) || std::get_if<EGamepadButton>(&Code)); }
    NODISCARD bool IsKeyboardKey() const { return std::get_if<EKeyboardKey>(&Code); }
    NODISCARD bool IsMouseKey() const { return (std::get_if<EMouseAxis>(&Code) || std::get_if<EMouseButton>(&Code)); }

    NODISCARD friend bool operator ==(const FInputKey& lhs, const FInputKey& rhs) {
        Assert_NoAssume(lhs.Code != rhs.Code || (lhs.Name == rhs.Name && lhs.ValueType == rhs.ValueType));
        return (lhs.Code == rhs.Code);
    }
    NODISCARD friend bool operator !=(const FInputKey& lhs, const FInputKey& rhs) {
        return (not operator ==(lhs, rhs));
    }

    NODISCARD friend bool operator < (const FInputKey& lhs, const FInputKey& rhs) {
        return (lhs.Code < rhs.Code);
    }
    NODISCARD friend bool operator >=(const FInputKey& lhs, const FInputKey& rhs) {
        return (not operator <(lhs, rhs));
    }

    NODISCARD friend hash_t hash_value(const FInputKey& value) {
        return std::visit([](auto key) -> hash_t {
            return hash_as_pod(key);
        }, value.Code);
    }
};
//----------------------------------------------------------------------------
namespace details {
template <typename T>
struct input_value {
    T Absolute;
    T Relative;
};
} //!details
//----------------------------------------------------------------------------
using FInputDigital = bool;
using FInputAxis1D = details::input_value<float>;
using FInputAxis2D = details::input_value<float2>;
using FInputAxis3D = details::input_value<float3>;
//----------------------------------------------------------------------------
namespace details {
using input_value_variant = std::variant<
    FInputDigital       ,   // Digital
    FInputAxis1D        ,   // Axis1D
    FInputAxis2D        ,   // Axis2D
    FInputAxis3D            // Axis3D
>;
} //!details
struct FInputValue : details::input_value_variant {
    using details::input_value_variant::input_value_variant;
};
//----------------------------------------------------------------------------
struct EInputKey {
    static PPE_APPLICATION_API void EachKey(TAppendable<FInputKey> keys) NOEXCEPT;

    NODISCARD static PPE_APPLICATION_API Meta::TOptionalReference<const FInputKey> From(EKeyboardKey value) NOEXCEPT;
    NODISCARD static PPE_APPLICATION_API Meta::TOptionalReference<const FInputKey> From(EGamepadAxis value) NOEXCEPT;
    NODISCARD static PPE_APPLICATION_API Meta::TOptionalReference<const FInputKey> From(EGamepadButton value) NOEXCEPT;
    NODISCARD static PPE_APPLICATION_API Meta::TOptionalReference<const FInputKey> From(EMouseAxis value) NOEXCEPT;
    NODISCARD static PPE_APPLICATION_API Meta::TOptionalReference<const FInputKey> From(EMouseButton value) NOEXCEPT;

    static PPE_APPLICATION_API const FInputKey AnyKey;

    static PPE_APPLICATION_API const FInputKey Mouse2D;
    static PPE_APPLICATION_API const FInputKey MouseWheelAxisX;
    static PPE_APPLICATION_API const FInputKey MouseWheelAxisY;

    static PPE_APPLICATION_API const FInputKey LeftMouseButton;
    static PPE_APPLICATION_API const FInputKey RightMouseButton;
    static PPE_APPLICATION_API const FInputKey MiddleMouseButton;
    static PPE_APPLICATION_API const FInputKey ThumbMouseButton;
    static PPE_APPLICATION_API const FInputKey ThumbMouseButton2;

    static PPE_APPLICATION_API const FInputKey BackSpace;
    static PPE_APPLICATION_API const FInputKey Tab;
    static PPE_APPLICATION_API const FInputKey Enter;
    static PPE_APPLICATION_API const FInputKey Pause;

    static PPE_APPLICATION_API const FInputKey CapsLock;
    static PPE_APPLICATION_API const FInputKey Escape;
    static PPE_APPLICATION_API const FInputKey SpaceBar;
    static PPE_APPLICATION_API const FInputKey PageUp;
    static PPE_APPLICATION_API const FInputKey PageDown;
    static PPE_APPLICATION_API const FInputKey End;
    static PPE_APPLICATION_API const FInputKey Home;

    static PPE_APPLICATION_API const FInputKey LeftArrow;
    static PPE_APPLICATION_API const FInputKey UpArrow;
    static PPE_APPLICATION_API const FInputKey RightArrow;
    static PPE_APPLICATION_API const FInputKey DownArrow;

    static PPE_APPLICATION_API const FInputKey Insert;
    static PPE_APPLICATION_API const FInputKey Delete;

    static PPE_APPLICATION_API const FInputKey Zero;
    static PPE_APPLICATION_API const FInputKey One;
    static PPE_APPLICATION_API const FInputKey Two;
    static PPE_APPLICATION_API const FInputKey Three;
    static PPE_APPLICATION_API const FInputKey Four;
    static PPE_APPLICATION_API const FInputKey Five;
    static PPE_APPLICATION_API const FInputKey Six;
    static PPE_APPLICATION_API const FInputKey Seven;
    static PPE_APPLICATION_API const FInputKey Eight;
    static PPE_APPLICATION_API const FInputKey Nine;

    static PPE_APPLICATION_API const FInputKey A;
    static PPE_APPLICATION_API const FInputKey B;
    static PPE_APPLICATION_API const FInputKey C;
    static PPE_APPLICATION_API const FInputKey D;
    static PPE_APPLICATION_API const FInputKey E;
    static PPE_APPLICATION_API const FInputKey F;
    static PPE_APPLICATION_API const FInputKey G;
    static PPE_APPLICATION_API const FInputKey H;
    static PPE_APPLICATION_API const FInputKey I;
    static PPE_APPLICATION_API const FInputKey J;
    static PPE_APPLICATION_API const FInputKey K;
    static PPE_APPLICATION_API const FInputKey L;
    static PPE_APPLICATION_API const FInputKey M;
    static PPE_APPLICATION_API const FInputKey N;
    static PPE_APPLICATION_API const FInputKey O;
    static PPE_APPLICATION_API const FInputKey P;
    static PPE_APPLICATION_API const FInputKey Q;
    static PPE_APPLICATION_API const FInputKey R;
    static PPE_APPLICATION_API const FInputKey S;
    static PPE_APPLICATION_API const FInputKey T;
    static PPE_APPLICATION_API const FInputKey U;
    static PPE_APPLICATION_API const FInputKey V;
    static PPE_APPLICATION_API const FInputKey W;
    static PPE_APPLICATION_API const FInputKey X;
    static PPE_APPLICATION_API const FInputKey Y;
    static PPE_APPLICATION_API const FInputKey Z;

    static PPE_APPLICATION_API const FInputKey NumPadZero;
    static PPE_APPLICATION_API const FInputKey NumPadOne;
    static PPE_APPLICATION_API const FInputKey NumPadTwo;
    static PPE_APPLICATION_API const FInputKey NumPadThree;
    static PPE_APPLICATION_API const FInputKey NumPadFour;
    static PPE_APPLICATION_API const FInputKey NumPadFive;
    static PPE_APPLICATION_API const FInputKey NumPadSix;
    static PPE_APPLICATION_API const FInputKey NumPadSeven;
    static PPE_APPLICATION_API const FInputKey NumPadEight;
    static PPE_APPLICATION_API const FInputKey NumPadNine;

    static PPE_APPLICATION_API const FInputKey Multiply;
    static PPE_APPLICATION_API const FInputKey Add;
    static PPE_APPLICATION_API const FInputKey Subtract;
    static PPE_APPLICATION_API const FInputKey Decimal;
    static PPE_APPLICATION_API const FInputKey Divide;

    static PPE_APPLICATION_API const FInputKey F1;
    static PPE_APPLICATION_API const FInputKey F2;
    static PPE_APPLICATION_API const FInputKey F3;
    static PPE_APPLICATION_API const FInputKey F4;
    static PPE_APPLICATION_API const FInputKey F5;
    static PPE_APPLICATION_API const FInputKey F6;
    static PPE_APPLICATION_API const FInputKey F7;
    static PPE_APPLICATION_API const FInputKey F8;
    static PPE_APPLICATION_API const FInputKey F9;
    static PPE_APPLICATION_API const FInputKey F10;
    static PPE_APPLICATION_API const FInputKey F11;
    static PPE_APPLICATION_API const FInputKey F12;

    static PPE_APPLICATION_API const FInputKey NumLock;
    static PPE_APPLICATION_API const FInputKey PrintScreen;
    static PPE_APPLICATION_API const FInputKey ScrollLock;

    static PPE_APPLICATION_API const FInputKey LeftShift;
    static PPE_APPLICATION_API const FInputKey RightShift;
    static PPE_APPLICATION_API const FInputKey LeftControl;
    static PPE_APPLICATION_API const FInputKey RightControl;
    static PPE_APPLICATION_API const FInputKey LeftAlt;
    static PPE_APPLICATION_API const FInputKey RightAlt;
    static PPE_APPLICATION_API const FInputKey LeftSuper;
    static PPE_APPLICATION_API const FInputKey RightSuper;

    static PPE_APPLICATION_API const FInputKey Semicolon;
    static PPE_APPLICATION_API const FInputKey Equals;
    static PPE_APPLICATION_API const FInputKey Underscore;
    static PPE_APPLICATION_API const FInputKey Hyphen;
    static PPE_APPLICATION_API const FInputKey Period;
    static PPE_APPLICATION_API const FInputKey Tilde;
    static PPE_APPLICATION_API const FInputKey LeftBracket;
    static PPE_APPLICATION_API const FInputKey Backslash;
    static PPE_APPLICATION_API const FInputKey RightBracket;
    static PPE_APPLICATION_API const FInputKey Apostrophe;

    static PPE_APPLICATION_API const FInputKey Ampersand;
    static PPE_APPLICATION_API const FInputKey Caret;
    static PPE_APPLICATION_API const FInputKey Colon;
    static PPE_APPLICATION_API const FInputKey Dollar;
    static PPE_APPLICATION_API const FInputKey Exclamation;
    static PPE_APPLICATION_API const FInputKey LeftParantheses;
    static PPE_APPLICATION_API const FInputKey RightParantheses;
    static PPE_APPLICATION_API const FInputKey Quote;

    // Gamepad Keys
    static PPE_APPLICATION_API const FInputKey Gamepad_Left2D;
    static PPE_APPLICATION_API const FInputKey Gamepad_Right2D;
    static PPE_APPLICATION_API const FInputKey Gamepad_LeftTriggerAxis;
    static PPE_APPLICATION_API const FInputKey Gamepad_RightTriggerAxis;

    static PPE_APPLICATION_API const FInputKey Gamepad_LeftThumbstick;
    static PPE_APPLICATION_API const FInputKey Gamepad_RightThumbstick;
    static PPE_APPLICATION_API const FInputKey Gamepad_FaceButton_Bottom;
    static PPE_APPLICATION_API const FInputKey Gamepad_FaceButton_Right;
    static PPE_APPLICATION_API const FInputKey Gamepad_FaceButton_Left;
    static PPE_APPLICATION_API const FInputKey Gamepad_FaceButton_Top;
    static PPE_APPLICATION_API const FInputKey Gamepad_LeftShoulder;
    static PPE_APPLICATION_API const FInputKey Gamepad_RightShoulder;
    static PPE_APPLICATION_API const FInputKey Gamepad_DPad_Up;
    static PPE_APPLICATION_API const FInputKey Gamepad_DPad_Down;
    static PPE_APPLICATION_API const FInputKey Gamepad_DPad_Right;
    static PPE_APPLICATION_API const FInputKey Gamepad_DPad_Left;
    static PPE_APPLICATION_API const FInputKey Gamepad_Start;
    static PPE_APPLICATION_API const FInputKey Gamepad_Back;
};
//----------------------------------------------------------------------------
PPE_APPLICATION_API FTextWriter& operator <<(FTextWriter& oss, const FInputKey& value);
PPE_APPLICATION_API FWTextWriter& operator <<(FWTextWriter& oss, const FInputKey& value);
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, EInputValueType value) {
    return oss << InputValueTypeCStr(value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
