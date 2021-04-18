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
TBasicTextWriter<_Char>& ExpandRHIFeature_(TBasicTextWriter<_Char>& oss, ERHIFeature feature) {
    STATIC_ASSERT(Meta::enum_is_flags_v<ERHIFeature>);
    if (ERHIFeature::All == feature) return oss << "All";
    if (ERHIFeature::Recommended == feature) return oss << "Recommended";
    if (ERHIFeature::Minimal == feature) return oss << "Minimal";

    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, "|"));

    if (ERHIFeature::Headless & feature) oss << sep << STRING_LITERAL(_Char, "Headless");
    if (ERHIFeature::Graphics & feature) oss << sep << STRING_LITERAL(_Char, "Graphics");
    if (ERHIFeature::Compute & feature) oss << sep << STRING_LITERAL(_Char, "Compute");
    if (ERHIFeature::AsyncCompute & feature) oss << sep << STRING_LITERAL(_Char, "AsyncCompute");
    if (ERHIFeature::Raytracing & feature) oss << sep << STRING_LITERAL(_Char, "Raytracing");
    if (ERHIFeature::MeshDraw & feature) oss << sep << STRING_LITERAL(_Char, "MeshDraw");
    if (ERHIFeature::SamplerFeedback & feature) oss << sep << STRING_LITERAL(_Char, "SamplerFeedback");
    if (ERHIFeature::TextureSpaceShading & feature) oss << sep << STRING_LITERAL(_Char, "TextureSpaceShading");
    if (ERHIFeature::VariableShadingRate & feature) oss << sep << STRING_LITERAL(_Char, "VariableShadingRate");
    if (ERHIFeature::ConservativeDepth & feature) oss << sep << STRING_LITERAL(_Char, "ConservativeDepth");
    if (ERHIFeature::HighDynamicRange & feature) oss << sep << STRING_LITERAL(_Char, "HighDynamicRange");
    if (ERHIFeature::VSync & feature) oss << sep << STRING_LITERAL(_Char, "VSync");

    if (ERHIFeature::Debugging & feature) oss << sep << STRING_LITERAL(_Char, "Debugging");
    if (ERHIFeature::Profiling & feature) oss << sep << STRING_LITERAL(_Char, "Profiling");

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
