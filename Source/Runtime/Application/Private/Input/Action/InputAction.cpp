// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Input/Action/InputAction.h"

#include "Meta/Functor.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FInputAction::FInputAction(FStringLiteral description, EInputValueType valueType, EInputActionFlags flags) NOEXCEPT
:   _description(description)
,   _flags(flags)
,   _valueType(valueType) {
    Assert_NoAssume(not _description.empty());
}
//----------------------------------------------------------------------------
FInputActionInstance::FInputActionInstance(const SCInputAction& sourceAction) NOEXCEPT
:   _sourceAction(sourceAction) {
    Assert_NoAssume(_sourceAction);
}
//----------------------------------------------------------------------------
// Create modulate modifier from value
//----------------------------------------------------------------------------
FInputAction::FModifierEvent FInputAction::Modulate(float value) NOEXCEPT {
    return [value](FTimespan dt, FInputValue& output) {
        Meta::Visit(output,
            [&](FInputDigital input) {
                output = FInputAxis1D{
                    .Absolute = (input ? static_cast<float>(double(value) * *dt) : 0.0f),
                    .Relative = (input ? static_cast<float>(double(value) * *dt) : 0.0f),
                };
            },
            [&](FInputAxis1D input) {
                output = FInputAxis1D{
                    .Absolute = static_cast<float>(double(input.Absolute * value) * *dt),
                    .Relative = static_cast<float>(double(input.Relative * value) * *dt),
                };
            },
            [&](const FInputAxis2D& input) {
                output = FInputAxis2D{
                    .Absolute = float2(double2(input.Absolute * value) * *dt),
                    .Relative = float2(double2(input.Relative * value) * *dt),
                };
            },
            [&](const FInputAxis3D& input) {
                output = FInputAxis3D{
                    .Absolute = float3(double3(input.Absolute * value) * *dt),
                    .Relative = float3(double3(input.Relative * value) * *dt),
                };
            });
    };
}
//----------------------------------------------------------------------------
FInputAction::FModifierEvent FInputAction::Modulate(const float2& value) NOEXCEPT {
    return [value](FTimespan dt, FInputValue& output) {
        Meta::Visit(output,
            [&](FInputDigital input) {
                output = FInputAxis2D{
                    .Absolute = input ? float2(double2(value) * *dt) : float2::Zero,
                    .Relative = input ? float2(double2(value) * *dt) : float2::Zero,
                };
            },
            [&](FInputAxis1D input) {
                output = FInputAxis2D{
                    .Absolute = float2(double2(input.Absolute * value) * *dt),
                    .Relative = float2(double2(input.Relative * value) * *dt),
                };
            },
            [&](const FInputAxis2D& input) {
                output = FInputAxis2D{
                    .Absolute = float2(double2(input.Absolute * value) * *dt),
                    .Relative = float2(double2(input.Relative * value) * *dt),
                };
            },
            [&](const FInputAxis3D& input) {
                output = FInputAxis2D{
                    .Absolute = float2(double2(input.Absolute.xz * value) * *dt),
                    .Relative = float2(double2(input.Relative.xz * value) * *dt),
                };
            });
    };
}
//----------------------------------------------------------------------------
FInputAction::FModifierEvent FInputAction::Modulate(const float3& value) NOEXCEPT {
    return [value](FTimespan dt, FInputValue& output) {
        Meta::Visit(output,
             [&](FInputDigital input) {
                output = FInputAxis3D{
                    .Absolute = input ? float3(double3(value) * *dt) : float3::Zero,
                    .Relative = input ? float3(double3(value) * *dt) : float3::Zero,
                };
            },
            [&](FInputAxis1D input) {
                output = FInputAxis3D{
                    .Absolute = float3(double3(input.Absolute * value) * *dt),
                    .Relative = float3(double3(input.Relative * value) * *dt),
                };
            },
            [&](const FInputAxis2D& input) {
                output = FInputAxis3D{
                    .Absolute = float3(double3(float3(input.Absolute, 0).xzy * value) * *dt),
                    .Relative = float3(double3(float3(input.Relative, 0).xzy * value) * *dt),
                };
            },
            [&](const FInputAxis3D& input) {
                output = FInputAxis3D{
                    .Absolute = float3(double3(input.Absolute * value) * *dt),
                    .Relative = float3(double3(input.Relative * value) * *dt),
                };
            });
    };
}
//----------------------------------------------------------------------------
// Create modulate modifier from functor
//----------------------------------------------------------------------------
FInputAction::FModifierEvent FInputAction::Modulate(TTinyFunction<float(FTimespan dt)>&& functor) NOEXCEPT {
    return [functor](FTimespan dt, FInputValue& output) {
        const float d = functor(dt);
        Meta::Visit(output,
            [&](FInputDigital input) {
                output = FInputAxis1D{
                    .Absolute = (input ? d : 0.0f),
                    .Relative = (input ? d : 0.0f),
                };
            },
            [&](FInputAxis1D input) {
                output = FInputAxis1D{
                    .Absolute = input.Absolute * d,
                    .Relative = input.Relative * d,
                };
            },
            [&](const FInputAxis2D& input) {
                output = FInputAxis2D{
                    .Absolute = input.Absolute * d,
                    .Relative = input.Relative * d,
                };
            },
            [&](const FInputAxis3D& input) {
                output = FInputAxis3D{
                    .Absolute = input.Absolute * d,
                    .Relative = input.Relative * d,
                };
            });
    };
}
//----------------------------------------------------------------------------
FInputAction::FModifierEvent FInputAction::Modulate(TTinyFunction<float2(FTimespan dt)>&& functor) NOEXCEPT {
    return [functor](FTimespan dt, FInputValue& output) {
        const float2 d = functor(dt);
        Meta::Visit(output,
            [&](FInputDigital input) {
                output = FInputAxis2D{
                    .Absolute = (input ? d : float2::Zero),
                    .Relative = (input ? d : float2::Zero),
                };
            },
            [&](FInputAxis1D input) {
                output = FInputAxis2D{
                    .Absolute = input.Absolute * d,
                    .Relative = input.Relative * d,
                };
            },
            [&](const FInputAxis2D& input) {
                output = FInputAxis2D{
                    .Absolute = input.Absolute * d,
                    .Relative = input.Relative * d,
                };
            },
            [&](const FInputAxis3D& input) {
                output = FInputAxis2D{
                    .Absolute = input.Absolute.xz * d,
                    .Relative = input.Relative.xz * d,
                };
            });
    };
}
//----------------------------------------------------------------------------
FInputAction::FModifierEvent FInputAction::Modulate(TTinyFunction<float3(FTimespan dt)>&& functor) NOEXCEPT {
    return [functor](FTimespan dt, FInputValue& output) {
        const float3 d = functor(dt);
        Meta::Visit(output,
            [&](FInputDigital input) {
                output = FInputAxis3D{
                    .Absolute = (input ? d : float3::Zero),
                    .Relative = (input ? d : float3::Zero),
                };
            },
            [&](FInputAxis1D input) {
                output = FInputAxis3D{
                    .Absolute = input.Absolute * d,
                    .Relative = input.Relative * d,
                };
            },
            [&](const FInputAxis2D& input) {
                output = FInputAxis3D{
                    .Absolute = float3(input.Absolute, 0).xzy * d,
                    .Relative = float3(input.Relative, 0).xzy * d,
                };
            },
            [&](const FInputAxis3D& input) {
                output = FInputAxis3D{
                    .Absolute = input.Absolute * d,
                    .Relative = input.Relative * d,
                };
            });
    };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
