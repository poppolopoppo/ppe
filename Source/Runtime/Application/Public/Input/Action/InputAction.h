#pragma once

#include "Application_fwd.h"

#include "Input/InputKey.h"

#include "IO/StringView.h"
#include "Memory/RefPtr.h"
#include "Misc/Event.h"
#include "Misc/Function_fwd.h"
#include "Time/Time_fwd.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EInputActionFlags : u8 {
    None                = 0,

    ConsumeInput        = 1 << 0,
    TriggerWhenPaused   = 1 << 1,

    Unknown             = ConsumeInput
};
ENUM_FLAGS(EInputActionFlags);
//----------------------------------------------------------------------------
enum class EInputTriggerEvent : u8 {
    Inactive            = 0,
    Started,
    Triggered,
    Completed,

    Unknown             = Inactive,
};
//----------------------------------------------------------------------------
class FInputAction : public FRefCountable {
public:
    FInputAction() = default;

    explicit PPE_APPLICATION_API FInputAction(
        FStringLiteral description,
        EInputValueType valueType = Default,
        EInputActionFlags flags = Default) NOEXCEPT;

    NODISCARD FStringLiteral Description() const { return _description; }
    NODISCARD EInputActionFlags Flags() const { return _flags; };
    NODISCARD EInputValueType ValueType() const { return _valueType; };

    NODISCARD bool HasConsumeInput() const { return (_flags & EInputActionFlags::ConsumeInput); }
    void SetConsumeInput(bool value) { _flags = Meta::EnumSet(_flags, EInputActionFlags::ConsumeInput, value); }

    NODISCARD bool HasTriggerWhenPaused() const { return (_flags & EInputActionFlags::TriggerWhenPaused); }
    void SetTriggerWhenPaused(bool value) { _flags = Meta::EnumSet(_flags, EInputActionFlags::TriggerWhenPaused, value); }

    using FModifierEvent = TFunction<void(FTimespan, FInputValue&)>;
    PUBLIC_EVENT(Modifiers,     FModifierEvent);

    using FTriggerEvent = TFunction<void(const FInputActionInstance&, const FInputKey&)>;
    PUBLIC_EVENT(OnStarted,     FTriggerEvent);
    PUBLIC_EVENT(OnTriggered,   FTriggerEvent);
    PUBLIC_EVENT(OnCompleted,   FTriggerEvent);

    NODISCARD PPE_APPLICATION_API static FModifierEvent Modulate(float value) NOEXCEPT;
    NODISCARD PPE_APPLICATION_API static FModifierEvent Modulate(const float2& value) NOEXCEPT;
    NODISCARD PPE_APPLICATION_API static FModifierEvent Modulate(const float3& value) NOEXCEPT;

    NODISCARD PPE_APPLICATION_API static FModifierEvent Modulate(TTinyFunction<float(FTimespan dt)>&& functor) NOEXCEPT;
    NODISCARD PPE_APPLICATION_API static FModifierEvent Modulate(TTinyFunction<float2(FTimespan dt)>&& functor) NOEXCEPT;
    NODISCARD PPE_APPLICATION_API static FModifierEvent Modulate(TTinyFunction<float3(FTimespan dt)>&& functor) NOEXCEPT;

private:
    friend class FInputListener;

    FStringLiteral _description;

    EInputActionFlags _flags{ Default };
    EInputValueType _valueType{ Default };
};
//----------------------------------------------------------------------------
class FInputActionInstance {
public:
    FInputActionInstance() = default;

    explicit PPE_APPLICATION_API FInputActionInstance(const SCInputAction& sourceAction) NOEXCEPT;

    NODISCARD const FInputValue& ActionValue() const { return _value; }
    NODISCARD FTimespan ElapsedTriggeredTime() const { return _elapsedTriggeredTime; }
    NODISCARD const FInputAction& SourceAction() const { return (*_sourceAction); }
    NODISCARD EInputTriggerEvent TriggerState() const { return _triggerState; }

    NODISCARD FInputDigital Digital() const { return std::get<FInputDigital>(_value); }
    NODISCARD FInputAxis1D Axis1D() const { return std::get<FInputAxis1D>(_value); }
    NODISCARD const FInputAxis2D& Axis2D() const { return std::get<FInputAxis2D>(_value); }
    NODISCARD const FInputAxis3D& Axis3D() const { return std::get<FInputAxis3D>(_value); }

    NODISCARD bool IsTriggerInactive() const { return (_triggerState == EInputTriggerEvent::Inactive); }
    NODISCARD bool IsTriggerStarted() const { return (_triggerState == EInputTriggerEvent::Started); }
    NODISCARD bool IsTriggerActive() const { return (_triggerState == EInputTriggerEvent::Triggered); }
    NODISCARD bool IsTriggerCompleted() const { return (_triggerState == EInputTriggerEvent::Completed); }

private:
    friend class FInputListener; // all logic here

    FInputValue _value;
    SCInputAction _sourceAction;
    FTimespan _elapsedTriggeredTime{ 0 };
    EInputTriggerEvent _triggerState{ Default };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
