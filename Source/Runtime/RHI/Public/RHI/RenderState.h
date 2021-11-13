#pragma once

#include "RHI_fwd.h"

#include "RHI/Config.h"
#include "RHI/ImageHelpers.h"
#include "RHI/RenderStateEnums.h"

#include "Container/Array.h"
#include "Container/Hash.h"
#include "Container/Stack.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FColorBufferState
//----------------------------------------------------------------------------
struct FColorBufferState {
    template <typename T>
    struct TColorState {
        T Color{ Default };
        T Alpha{ Default };

        TColorState() = default;

        TColorState(T rgba) : Color(rgba), Alpha(rgba) {}
        TColorState(T rgb, T a) : Color(rgb), Alpha(a) {}

        bool operator ==(const TColorState& other) const { return (Color == other.Color && Alpha == other.Alpha); }
        bool operator !=(const TColorState& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const TColorState& state) {
            return hash_tuple(state.Color, state.Alpha);
        }
    };

    TColorState<EBlendFactor> SrcBlendFactor{ EBlendFactor::One };
    TColorState<EBlendFactor> DstBlendFactor{ EBlendFactor::Zero };
    TColorState<EBlendOp> BlendOp{ EBlendOp::Add };
    EColorMask ColorMask{ Default };
    bool EnableAlphaBlending{ false };

    bool operator ==(const FColorBufferState& other) const {
        return SrcBlendFactor == other.SrcBlendFactor
            && DstBlendFactor == other.DstBlendFactor
            && BlendOp == other.BlendOp
            && ColorMask == other.ColorMask
            && EnableAlphaBlending == other.EnableAlphaBlending;
    }
    bool operator !=(const FColorBufferState& other) const {
        return (not operator ==(other));
    }

    friend hash_t hash_value(const FColorBufferState& buffer) {
        return hash_tuple(
            buffer.SrcBlendFactor,
            buffer.DstBlendFactor,
            buffer.BlendOp,
            buffer.ColorMask,
            buffer.EnableAlphaBlending);
    }
};
PPE_ASSUME_TYPE_AS_POD(FColorBufferState);
//----------------------------------------------------------------------------
// FBlendState
//----------------------------------------------------------------------------
struct FBlendState {
    using FColorBuffers = TStaticArray<FColorBufferState, MaxColorBuffers>;

    FColorBuffers Buffers;
    FRgba32f BlendColor{ 1.0f };
    ELogicOp LogicOp{ ELogicOp::None };

    bool operator ==(const FBlendState& other) const {
        return (Buffers == other.Buffers && BlendColor == other.BlendColor && LogicOp == other.LogicOp);
    }
    bool operator !=(const FBlendState& other) const {
        return (not operator ==(other));
    }

    friend hash_t hash_value(const FBlendState& state) {
        return hash_tuple(
            hash_range(state.Buffers.data(), state.Buffers.size()),
            state.BlendColor, state.LogicOp );
    }

};
PPE_ASSUME_TYPE_AS_POD(FBlendState);
//----------------------------------------------------------------------------
// FDepthBufferState
//----------------------------------------------------------------------------
struct FDepthBufferState {
    ECompareOp CompareOp{ ECompareOp::LessEqual };
    float2 Bounds{ 0, 1 };
    bool EnableBounds{ false };
    bool EnableDepthWrite{ false };
    bool EnableDepthTest{ false };

    bool operator ==(const FDepthBufferState& other) const {
        return (CompareOp == other.CompareOp
            && Bounds == other.Bounds
            && EnableBounds == other.EnableBounds
            && EnableDepthWrite == other.EnableDepthWrite
            && EnableDepthTest == other.EnableDepthTest );
    }
    bool operator !=(const FDepthBufferState& other) const {
        return (not operator ==(other));
    }

    friend hash_t hash_value(const FDepthBufferState& state) {
        return hash_tuple(state.CompareOp, state.Bounds, state.EnableBounds, state.EnableDepthWrite, state.EnableDepthTest);
    }

};
PPE_ASSUME_TYPE_AS_POD(FDepthBufferState);
//----------------------------------------------------------------------------
// FInputAssemblyState
//----------------------------------------------------------------------------
struct FInputAssemblyState {
    EPrimitiveTopology Topology{ Default };
    bool EnablePrimitiveRestart{ false }; // if 'true' then index with -1 value will restarting the assembly of primitives

    bool operator ==(const FInputAssemblyState& other) const { return (Topology == other.Topology && EnablePrimitiveRestart == other.EnablePrimitiveRestart); }
    bool operator !=(const FInputAssemblyState& other) const { return (not operator ==(other)); }

    friend hash_t hash_value(const FInputAssemblyState& state) {
        return hash_tuple(state.Topology, state.EnablePrimitiveRestart);
    }
};
PPE_ASSUME_TYPE_AS_POD(FInputAssemblyState);
//----------------------------------------------------------------------------
// FMultisampleState
//----------------------------------------------------------------------------
struct FMultisampleState {
    uint4 SampleMask{ 0 };
    FMultiSamples Samples{ 1 };
    float MinSampleShading{ 0 };
    bool EnableAlphaToCoverage{ false };
    bool EnableAlphaToOne{ false };
    bool EnableSampleShading{ false };

    bool operator ==(const FMultisampleState& other) const {
        return (SampleMask == other.SampleMask
            && Samples == other.Samples
            && MinSampleShading == other.MinSampleShading
            && EnableAlphaToCoverage == other.EnableAlphaToCoverage
            && EnableAlphaToOne == other.EnableAlphaToOne
            && EnableSampleShading == other.EnableSampleShading );
    }
    bool operator !=(const FMultisampleState& other) const {
        return (not operator ==(other));
    }

    friend hash_t hash_value(const FMultisampleState& state) {
        return hash_tuple(
            state.SampleMask, state.Samples, state.MinSampleShading,
            state.EnableAlphaToCoverage, state.EnableAlphaToOne, state.EnableSampleShading );
    }

};
PPE_ASSUME_TYPE_AS_POD(FMultisampleState);
//----------------------------------------------------------------------------
// FRasterizationState
//----------------------------------------------------------------------------
struct FRasterizationState {
    ECullMode CullMode{ ECullMode::None };
    EPolygonMode PolygonMode{ EPolygonMode::Fill };
    float LineWidth{ 1.0f };
    float DepthBiasConstantFactor{ 0.0f };
    float DepthBiasClamp{ 0.0f };
    float DepthBiasSlopeFactor{ 0.0f };
    bool EnableDepthBias{ false };
    bool EnableDepthClamp{ false };
    bool EnableDiscard{ false };
    bool EnableFrontFaceCCW{ false };

    FRasterizationState& SetDepthBias(float constFactor, float clamp, float slopeFactor) {
        DepthBiasConstantFactor = constFactor;
        DepthBiasClamp = clamp;
        DepthBiasSlopeFactor = slopeFactor;
        return (*this);
    }

    bool operator ==(const FRasterizationState& other) const {
        return (CullMode == other.CullMode
            && PolygonMode == other.PolygonMode
            && LineWidth == other.LineWidth
            && DepthBiasConstantFactor == other.DepthBiasConstantFactor
            && DepthBiasClamp == other.DepthBiasClamp
            && DepthBiasSlopeFactor == other.DepthBiasSlopeFactor
            && EnableDepthBias == other.EnableDepthBias
            && EnableDepthClamp == other.EnableDepthClamp
            && EnableDiscard == other.EnableDiscard
            && EnableFrontFaceCCW == other.EnableFrontFaceCCW);
    }
    bool operator !=(const FRasterizationState& other) const {
        return (not operator ==(other));
    }

    friend hash_t hash_value(const FRasterizationState& state) {
        return hash_tuple(
            state.CullMode, state.PolygonMode, state.LineWidth,
            state.DepthBiasConstantFactor, state.DepthBiasClamp, state.DepthBiasSlopeFactor,
            state.EnableDepthBias, state.EnableDepthClamp, state.EnableDiscard, state.EnableFrontFaceCCW );
    }
};
PPE_ASSUME_TYPE_AS_POD(FRasterizationState);
//----------------------------------------------------------------------------
// FStencilBufferState
//----------------------------------------------------------------------------
struct FStencilBufferState {

    struct FFaceState {
        EStencilOp FailOp{ EStencilOp::Keep };
        EStencilOp DepthFailOp{ EStencilOp::Keep };
        EStencilOp PassOp{ EStencilOp::Keep };
        ECompareOp CompareOp{ ECompareOp::Always };
        FStencilValue Reference{ 0 };
        FStencilValue WriteMask{ UMax };
        FStencilValue CompareMask{ 0 };

        bool ReadOnly() const {
            if (CompareOp == ECompareOp::Never and FailOp == EStencilOp::Keep)
                return true;

            return (FailOp == EStencilOp::Keep and
                    DepthFailOp == EStencilOp::Keep and
                    PassOp == EStencilOp::Keep );
        }

        bool operator ==(const FFaceState& other) const {
            return (FailOp == other.FailOp
                && DepthFailOp == other.DepthFailOp
                && PassOp == other.PassOp
                && CompareOp == other.CompareOp
                && Reference == other.Reference
                && WriteMask == other.WriteMask
                && CompareMask == other.CompareMask );
        }
        bool operator !=(const FFaceState& other) const {
            return (not operator ==(other));
        }

        friend hash_t hash_value(const FFaceState& face) {
            return hash_tuple(
                face.FailOp, face.DepthFailOp, face.PassOp, face.CompareOp,
                face.Reference, face.WriteMask, face.CompareMask );
        }
    };

    FFaceState Front;
    FFaceState Back;
    bool EnabledStencilTests{ false };

    bool ReadOnly() const {
        return (not EnabledStencilTests or (
            Front.ReadOnly() and Back.ReadOnly() ));
    }

    bool operator ==(const FStencilBufferState& other) const {
        return (Front == other.Front
            && Back == other.Back
            && EnabledStencilTests == other.EnabledStencilTests);
    }
    bool operator !=(const FStencilBufferState& other) const {
        return (not operator ==(other));
    }

    friend hash_t hash_value(const FStencilBufferState& state) {
        return hash_tuple(state.Front, state.Back, state.EnabledStencilTests);
    }

};
PPE_ASSUME_TYPE_AS_POD(FStencilBufferState);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FRenderState
//----------------------------------------------------------------------------
struct FRenderState {
    FBlendState Blend;
    FDepthBufferState Depth;
    FStencilBufferState Stencil;
    FInputAssemblyState InputAssembly;
    FRasterizationState Rasterization;
    FMultisampleState Multisample;

    bool operator ==(const FRenderState& other) const NOEXCEPT {
        return (Blend == other.Blend
            && Depth == other.Depth
            && Stencil == other.Stencil
            && InputAssembly == other.InputAssembly
            && Rasterization == other.Rasterization
            && Multisample == other.Multisample );
    }
    bool operator !=(const FRenderState& other) const NOEXCEPT {
        return (not operator ==(other));
    }

    friend hash_t hash_value(const FRenderState& state) NOEXCEPT {
        return hash_tuple(state.Blend, state.Depth, state.Stencil, state.InputAssembly, state.Rasterization, state.Multisample);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
