#include "stdafx.h"

#include "HAL/TargetRHI.h"

#include "HAL/Vulkan/VulkanTargetRHI.h"

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const ITargetRHI* GAllRHIs[] = {
    /* Vulkan  */&FVulkanTargetRHI::Get(),
    // #TODO add future RHIs here
};
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& ExpandTargetRHI_(TBasicTextWriter<_Char>& oss, ETargetRHI rhi) {
    switch (rhi) {
    case PPE::RHI::ETargetRHI::Vulkan:
        return oss << STRING_LITERAL(_Char, "Vulkan");
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& ExpandRHIFeature_(TBasicTextWriter<_Char>& oss, ERHIFeature feature) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, "|"));
    if (feature ^ ERHIFeature::HighEndGraphics)
        oss << sep << STRING_LITERAL(_Char, "HighEndGraphics");
    if (feature ^ ERHIFeature::Compute)
        oss << sep << STRING_LITERAL(_Char, "Compute");
    if (feature ^ ERHIFeature::AsyncCompute)
        oss << sep << STRING_LITERAL(_Char, "AsyncCompute");
    if (feature ^ ERHIFeature::Raytracing)
        oss << sep << STRING_LITERAL(_Char, "Raytracing");
    if (feature ^ ERHIFeature::Meshlet)
        oss << sep << STRING_LITERAL(_Char, "Meshlet");
    if (feature ^ ERHIFeature::SamplerFeedback)
        oss << sep << STRING_LITERAL(_Char, "SamplerFeedback");
    if (feature ^ ERHIFeature::TextureSpaceShading)
        oss << sep << STRING_LITERAL(_Char, "TextureSpaceShading");
    return oss;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TMemoryView<const ITargetRHI* const> AllTargetRHIs() {
    return MakeConstView(GAllRHIs);
}
//----------------------------------------------------------------------------
const ITargetRHI& TargetRHI(ETargetRHI rhi) {
    AssertRelease(rhi == ETargetRHI::Vulkan); // #TODO
    STATIC_ASSERT(size_t(ETargetRHI::Vulkan) == 0);
    return (*GAllRHIs[size_t(ETargetRHI::Vulkan)]);
}
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
} //!namespace RHI
} //!namespace PPE
