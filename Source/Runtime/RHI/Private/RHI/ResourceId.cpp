#include "stdafx.h"

#include "RHI/ResourceId.h"

#include "IO/StringView.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace RHI {
STATIC_ASSERT(FVertexBufferID{}.HashValue._value == Zero);
STATIC_ASSERT(FVertexBufferID(EmptyKey).HashValue == UMax);
STATIC_ASSERT(std::is_constructible_v<FVertexBufferID, Meta::FEmptyKey>);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char, u32 _Uid, bool _KeepName>
TBasicTextWriter<_Char>& WriteValue_(const TBasicStringView<_Char>& name, TBasicTextWriter<_Char>& oss, const details::TNamedId<_Uid, _KeepName>& value) {
    oss << name << STRING_LITERAL(_Char, '<') << _Uid << STRING_LITERAL(_Char, ">(");
    IF_CONSTEXPR(_KeepName) oss << value.Name.Str();
    return oss << STRING_LITERAL(_Char, ", ") << value.HashValue << STRING_LITERAL(_Char, ')');
}
//----------------------------------------------------------------------------
template <typename _Char, u32 _Uid>
TBasicTextWriter<_Char>& WriteValue_(const TBasicStringView<_Char>& name, TBasicTextWriter<_Char>& oss, const details::TResourceId<_Uid>& value) {
    return oss << name
        << STRING_LITERAL(_Char, '<') << _Uid << STRING_LITERAL(_Char, ">(")
        << value.Index << STRING_LITERAL(_Char, ", ") << value.InstanceID
        << STRING_LITERAL(_Char, ')');
}
//----------------------------------------------------------------------------
template <typename _Char, u32 _Uid>
TBasicTextWriter<_Char>& WriteValue_(const TBasicStringView<_Char>& name, TBasicTextWriter<_Char>& oss, const details::TResourceWrappedId<details::TResourceId<_Uid>>& value) {
    return oss << name
        << STRING_LITERAL(_Char, '<') << _Uid << STRING_LITERAL(_Char, ">(")
        << value.Id.Index << STRING_LITERAL(_Char, " / ") << value.Id.InstanceID
        << STRING_LITERAL(_Char, ')');
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
#define PPE_RHI_TEXTWRITEROPERATOR_DEF(_TYPE) \
    PPE_RHI_API FTextWriter& operator <<(FTextWriter& oss, const _TYPE& value) { \
        return WriteValue_(MakeStringView(STRINGIZE(_TYPE)), oss, value); \
    } \
    PPE_RHI_API FWTextWriter& operator <<(FWTextWriter& oss, const _TYPE& value) { \
        return WriteValue_(MakeStringView(WSTRINGIZE(_TYPE)), oss, value); \
    }
//----------------------------------------------------------------------------
PPE_RHI_TEXTWRITEROPERATOR_DEF(FUniformID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FPushConstantID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FDescriptorSetID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FSpecializationID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FVertexID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FVertexBufferID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FMemPoolID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRTShaderID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FGeometryID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FInstanceID)
//----------------------------------------------------------------------------
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawBufferID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawImageID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawGPipelineID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawMPipelineID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawCPipelineID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawRTPipelineID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawSamplerID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawDescriptorSetLayoutID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawPipelineResourcesID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawRTSceneID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawRTGeometryID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawRTShaderTableID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawSwapchainID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FLogicalPassID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawMemoryID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawPipelineLayoutID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawRenderPassID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRawFramebufferID)
//----------------------------------------------------------------------------
PPE_RHI_TEXTWRITEROPERATOR_DEF(FBufferID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FImageID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FGPipelineID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FMPipelineID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FCPipelineID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRTPipelineID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FSamplerID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRTSceneID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRTGeometryID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRTShaderTableID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FSwapchainID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FMemoryID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FPipelineLayoutID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FRenderPassID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FFramebufferID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FPipelineResourcesID)
PPE_RHI_TEXTWRITEROPERATOR_DEF(FDescriptorSetLayoutID)
//----------------------------------------------------------------------------
#undef PPE_RHI_TEXTWRITEROPERATOR_DEF
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TNamedId<   1, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TNamedId<   2, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TNamedId<   3, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TNamedId<   4, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TNamedId<   5, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TNamedId<   6, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TNamedId<   7, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TNamedId<   8, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TNamedId<   9, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TNamedId<  10, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId<  1 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId<  2 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId<  3 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId<  4 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId<  5 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId<  6 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId<  7 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId<  8 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId<  9 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId< 10 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId< 11 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId< 12 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId< 13 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId< 14 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId< 15 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId< 16 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId< 17 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceId< 18 >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawBufferID >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawImageID >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawGPipelineID >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawMPipelineID >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawCPipelineID >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawRTPipelineID >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawSamplerID >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawRTSceneID >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawRTGeometryID >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawRTShaderTableID >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawSwapchainID >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawMemoryID >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawPipelineLayoutID >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawRenderPassID >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawFramebufferID >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawPipelineResourcesID >;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_RHI_API) TResourceWrappedId< FRawDescriptorSetLayoutID >;
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
