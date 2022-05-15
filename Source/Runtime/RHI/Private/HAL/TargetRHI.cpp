#include "stdafx.h"

#include "HAL/TargetRHI.h"

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& ExpandTargetRHI_(TBasicTextWriter<_Char>& oss, ETargetRHI rhi) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<ETargetRHI>);
    switch (rhi) {
    case ETargetRHI::Vulkan:
        return oss << STRING_LITERAL(_Char, "Vulkan");
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& ExpandRHIFeature_(TBasicTextWriter<_Char>& oss, ERHIFeature features) {
    STATIC_ASSERT(Meta::enum_is_flags_v<ERHIFeature>);

    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, "|"));

    for (auto mask = MakeEnumBitMask(features); mask; ) {
        const auto feature = static_cast<ERHIFeature>(1u << mask.PopFront_AssumeNotEmpty());

        switch (feature) {
        case ERHIFeature::Headless: oss << sep << STRING_LITERAL(_Char, "Headless"); break;
        case ERHIFeature::Discrete: oss << sep << STRING_LITERAL(_Char, "Discrete"); break;
        case ERHIFeature::Graphics: oss << sep << STRING_LITERAL(_Char, "Graphics"); break;
        case ERHIFeature::Compute: oss << sep << STRING_LITERAL(_Char, "Compute"); break;
        case ERHIFeature::AsyncCompute: oss << sep << STRING_LITERAL(_Char, "AsyncCompute"); break;
        case ERHIFeature::RayTracing: oss << sep << STRING_LITERAL(_Char, "Raytracing"); break;
        case ERHIFeature::MeshDraw: oss << sep << STRING_LITERAL(_Char, "MeshDraw"); break;
        case ERHIFeature::SamplerFeedback: oss << sep << STRING_LITERAL(_Char, "SamplerFeedback"); break;
        case ERHIFeature::TextureSpaceShading: oss << sep << STRING_LITERAL(_Char, "TextureSpaceShading"); break;
        case ERHIFeature::VariableShadingRate: oss << sep << STRING_LITERAL(_Char, "VariableShadingRate"); break;
        case ERHIFeature::ConservativeDepth: oss << sep << STRING_LITERAL(_Char, "ConservativeDepth"); break;
        case ERHIFeature::HighDynamicRange: oss << sep << STRING_LITERAL(_Char, "HighDynamicRange"); break;
        case ERHIFeature::HighDPIAwareness: oss << sep << STRING_LITERAL(_Char, "HighDPIAwareness"); break;
        case ERHIFeature::VSync: oss << sep << STRING_LITERAL(_Char, "VSync"); break;
        case ERHIFeature::Debugging: oss << sep << STRING_LITERAL(_Char, "Debugging"); break;
        case ERHIFeature::Profiling: oss << sep << STRING_LITERAL(_Char, "Profiling"); break;
        default: AssertReleaseMessage(L"unsupported ERHIFeature flag", features == ERHIFeature::All);
        }
    }

    if (features == Zero)
        oss << STRING_LITERAL(_Char, "0");

    return oss;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, ETargetRHI rhi) {
    return ExpandTargetRHI_(oss, rhi);
}
FWTextWriter& operator <<(FWTextWriter& oss, ETargetRHI rhi) {
    return ExpandTargetRHI_(oss, rhi);
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, ERHIFeature features) {
    return ExpandRHIFeature_(oss, features);
}
FWTextWriter& operator <<(FWTextWriter& oss, ERHIFeature features) {
    return ExpandRHIFeature_(oss, features);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
