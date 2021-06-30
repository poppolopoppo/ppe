#include "stdafx.h"

#include "RHI/EnumToString.h"

#include "RHI/PixelFormatHelpers.h"
#include "RHI/RayTracingEnums.h"
#include "RHI/RenderStateEnums.h"
#include "RHI/ResourceEnums.h"
#include "RHI/SamplerEnums.h"
#include "RHI/ShaderEnums.h"
#include "RHI/SwapchainDesc.h"
#include "RHI/VertexEnums.h"

#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
auto Separator_() { return Fmt::NotFirstTime(STRING_LITERAL(_Char, "|")); }
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EQueueType value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EQueueType>);
    switch (value) {
    case EQueueType::Graphics: return oss << STRING_LITERAL(_Char, "Graphics");
    case EQueueType::AsyncCompute: return oss << STRING_LITERAL(_Char, "AsyncCompute");
    case EQueueType::AsyncTransfer: return oss << STRING_LITERAL(_Char, "AsyncTransfer");
    case EQueueType::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    default: AssertNotReached();
    }
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EQueueUsage value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<EQueueUsage>);
    if (value == EQueueUsage::Unknown) return oss << STRING_LITERAL(_Char, "Unknown");

    auto sep = Separator_<_Char>();

    if (EQueueUsage::Graphics & value) oss << sep << STRING_LITERAL(_Char, "Graphics");
    if (EQueueUsage::AsyncCompute & value) oss << sep << STRING_LITERAL(_Char, "AsyncCompute");
    if (EQueueUsage::AsyncTransfer & value) oss << sep << STRING_LITERAL(_Char, "AsyncTransfer");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EMemoryType value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<EMemoryType>);
    if (value == EMemoryType::Default) return oss << STRING_LITERAL(_Char, "Default");

    auto sep = Separator_<_Char>();

    if (EMemoryType::HostRead & value) oss << sep << STRING_LITERAL(_Char, "HostRead");
    if (EMemoryType::HostWrite & value) oss << sep << STRING_LITERAL(_Char, "HostWrite");
    if (EMemoryType::Dedicated & value) oss << sep << STRING_LITERAL(_Char, "Dedicated");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EBufferUsage value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<EBufferUsage>);
    if (EBufferUsage::All == value) return oss << STRING_LITERAL(_Char, "All");
    if (EBufferUsage::Transfer == value) return oss << STRING_LITERAL(_Char, "Transfer");
    if (EBufferUsage::Unknown == value) return oss << STRING_LITERAL(_Char, "Unknown");

    auto sep = Separator_<_Char>();

    if (EBufferUsage::TransferSrc & value) oss << sep << STRING_LITERAL(_Char, "TransferSrc");
    if (EBufferUsage::TransferDst & value) oss << sep << STRING_LITERAL(_Char, "TransferDst");
    if (EBufferUsage::UniformTexel & value) oss << sep << STRING_LITERAL(_Char, "UniformTexel");
    if (EBufferUsage::StorageTexel & value) oss << sep << STRING_LITERAL(_Char, "StorageTexel");
    if (EBufferUsage::Uniform & value) oss << sep << STRING_LITERAL(_Char, "Uniform");
    if (EBufferUsage::Storage & value) oss << sep << STRING_LITERAL(_Char, "Storage");
    if (EBufferUsage::Index & value) oss << sep << STRING_LITERAL(_Char, "Index");
    if (EBufferUsage::Vertex & value) oss << sep << STRING_LITERAL(_Char, "Vertex");
    if (EBufferUsage::Indirect & value) oss << sep << STRING_LITERAL(_Char, "Indirect");
    if (EBufferUsage::RayTracing & value) oss << sep << STRING_LITERAL(_Char, "RayTracing");


    if (EBufferUsage::VertexPplnStore & value) oss << sep << STRING_LITERAL(_Char, "VertexPplnStore");
    if (EBufferUsage::FragmentPplnStore & value) oss << sep << STRING_LITERAL(_Char, "FragmentPplnStore");
    if (EBufferUsage::StorageTexelAtomic & value) oss << sep << STRING_LITERAL(_Char, "StorageTexelAtomic");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EImageDim value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EImageDim>);
    switch (value) {
    case EImageDim::_1D: return oss << STRING_LITERAL(_Char, "1D");
    case EImageDim::_2D: return oss << STRING_LITERAL(_Char, "2D");
    case EImageDim::_3D: return oss << STRING_LITERAL(_Char, "3D");
    case EImageDim::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EImageView value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EImageView>);
    switch (value) {
    case EImageView::_1D: return oss << STRING_LITERAL(_Char, "1D");
    case EImageView::_1DArray: return oss << STRING_LITERAL(_Char, "1DArray");
    case EImageView::_2D: return oss << STRING_LITERAL(_Char, "2D");
    case EImageView::_2DArray: return oss << STRING_LITERAL(_Char, "2DArray");
    case EImageView::_Cube: return oss << STRING_LITERAL(_Char, "Cube");
    case EImageView::_CubeArray: return oss << STRING_LITERAL(_Char, "CubeArray");
    case EImageView::_3D: return oss << STRING_LITERAL(_Char, "3D");
    case EImageView::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EImageFlags value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<EImageFlags>);
    if (EImageFlags::Unknown == value) return oss << STRING_LITERAL(_Char, "Unknown");

    auto sep = Separator_<_Char>();

    if (EImageFlags::MutableFormat == value) oss << sep << STRING_LITERAL(_Char, "MutableFormat");
    if (EImageFlags::CubeCompatible == value) oss << sep << STRING_LITERAL(_Char, "CubeCompatible");
    if (EImageFlags::Array2DCompatible == value) oss << sep << STRING_LITERAL(_Char, "Array2DCompatible");
    if (EImageFlags::BlockTexelViewCompatible == value) oss << sep << STRING_LITERAL(_Char, "BlockTexelViewCompatible");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EImageUsage value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<EImageUsage>);
    if (EImageUsage::All == value) return oss << STRING_LITERAL(_Char, "All");
    if (EImageUsage::Transfer == value) return oss << STRING_LITERAL(_Char, "Transfer");
    if (EImageUsage::Unknown == value) return oss << STRING_LITERAL(_Char, "Unknown");

    auto sep = Separator_<_Char>();

    if (EImageUsage::TransferSrc & value) oss << sep << STRING_LITERAL(_Char, "TransferSrc");
    if (EImageUsage::TransferDst & value) oss << sep << STRING_LITERAL(_Char, "TransferDst");
    if (EImageUsage::Sampled & value) oss << sep << STRING_LITERAL(_Char, "Sampled");
    if (EImageUsage::Storage & value) oss << sep << STRING_LITERAL(_Char, "Storage");
    if (EImageUsage::ColorAttachment & value) oss << sep << STRING_LITERAL(_Char, "ColorAttachment");
    if (EImageUsage::DepthStencilAttachment & value) oss << sep << STRING_LITERAL(_Char, "DepthStencilAttachment");
    if (EImageUsage::TransientAttachment & value) oss << sep << STRING_LITERAL(_Char, "TransientAttachment");
    if (EImageUsage::InputAttachment & value) oss << sep << STRING_LITERAL(_Char, "InputAttachment");
    if (EImageUsage::ShadingRate & value) oss << sep << STRING_LITERAL(_Char, "ShadingRate");

    if (EImageUsage::StorageAtomic & value) oss << sep << STRING_LITERAL(_Char, "StorageAtomic");
    if (EImageUsage::ColorAttachmentBlend & value) oss << sep << STRING_LITERAL(_Char, "ColorAttachmentBlend");
    if (EImageUsage::SampledMinMax & value) oss << sep << STRING_LITERAL(_Char, "ColorAttachmentBlend");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EImageAspect value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<EImageAspect>);
    if (EImageAspect::Auto == value) return oss << STRING_LITERAL(_Char, "Auto");
    if (EImageAspect::Unknown == value) return oss << STRING_LITERAL(_Char, "Unknown");

    auto sep = Separator_<_Char>();

    if (EImageAspect::Color & value) oss << sep << STRING_LITERAL(_Char, "Color");
    if (EImageAspect::Depth & value) oss << sep << STRING_LITERAL(_Char, "Depth");
    if (EImageAspect::Stencil & value) oss << sep << STRING_LITERAL(_Char, "Stencil");
    if (EImageAspect::Metadata & value) oss << sep << STRING_LITERAL(_Char, "Metadata");
    if (EImageAspect::DepthStencil & value) oss << sep << STRING_LITERAL(_Char, "DepthStencil");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EImageSampler value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<EImageSampler>); // still treat as a regular enum

    switch (value) {
    case EImageSampler::Float1D: return oss << STRING_LITERAL(_Char, "Float1D");
    case EImageSampler::Float1DArray: return oss << STRING_LITERAL(_Char, "Float1DArray");
    case EImageSampler::Float2D: return oss << STRING_LITERAL(_Char, "Float2D");
    case EImageSampler::Float2DArray: return oss << STRING_LITERAL(_Char, "Float2DArray");
    case EImageSampler::Float2DMS: return oss << STRING_LITERAL(_Char, "Float2DMS");
    case EImageSampler::Float2DMSArray: return oss << STRING_LITERAL(_Char, "Float2DMSArray");
    case EImageSampler::FloatCube: return oss << STRING_LITERAL(_Char, "FloatCube");
    case EImageSampler::FloatCubeArray: return oss << STRING_LITERAL(_Char, "FloatCubeArray");
    case EImageSampler::Float3D: return oss << STRING_LITERAL(_Char, "Float3D");
    case EImageSampler::Int1D: return oss << STRING_LITERAL(_Char, "Int1D");
    case EImageSampler::Int1DArray: return oss << STRING_LITERAL(_Char, "Int1DArray");
    case EImageSampler::Int2D: return oss << STRING_LITERAL(_Char, "Int2D");
    case EImageSampler::Int2DArray: return oss << STRING_LITERAL(_Char, "Int2DArray");
    case EImageSampler::Int2DMS: return oss << STRING_LITERAL(_Char, "Int2DMS");
    case EImageSampler::Int2DMSArray: return oss << STRING_LITERAL(_Char, "Int2DMSArray");
    case EImageSampler::IntCube: return oss << STRING_LITERAL(_Char, "IntCube");
    case EImageSampler::IntCubeArray: return oss << STRING_LITERAL(_Char, "IntCubeArray");
    case EImageSampler::Int3D: return oss << STRING_LITERAL(_Char, "Int3D");
    case EImageSampler::Uint1D: return oss << STRING_LITERAL(_Char, "Uint1D");
    case EImageSampler::Uint1DArray: return oss << STRING_LITERAL(_Char, "Uint1DArray");
    case EImageSampler::Uint2D: return oss << STRING_LITERAL(_Char, "Uint2D");
    case EImageSampler::Uint2DArray: return oss << STRING_LITERAL(_Char, "Uint2DArray");
    case EImageSampler::Uint2DMS: return oss << STRING_LITERAL(_Char, "Uint2DMS");
    case EImageSampler::Uint2DMSArray: return oss << STRING_LITERAL(_Char, "Uint2DMSArray");
    case EImageSampler::UintCube: return oss << STRING_LITERAL(_Char, "UintCube");
    case EImageSampler::UintCubeArray: return oss << STRING_LITERAL(_Char, "UintCubeArray");
    case EImageSampler::Uint3D: return oss << STRING_LITERAL(_Char, "Uint3D");
    case EImageSampler::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EAttachmentStoreOp value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EAttachmentStoreOp>);
    switch (value) {
    case EAttachmentStoreOp::Invalidate: return oss << STRING_LITERAL(_Char, "Invalidate");
    case EAttachmentStoreOp::Store: return oss << STRING_LITERAL(_Char, "Store");
    case EAttachmentStoreOp::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EShadingRatePalette value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EShadingRatePalette>);
    switch (value) {
    case EShadingRatePalette::NoInvocations: return oss << STRING_LITERAL(_Char, "NoInvocations");
    case EShadingRatePalette::Block_1x1_16: return oss << STRING_LITERAL(_Char, "Block_1x1_16");
    case EShadingRatePalette::Block_1x1_8: return oss << STRING_LITERAL(_Char, "Block_1x1_8");
    case EShadingRatePalette::Block_1x1_4: return oss << STRING_LITERAL(_Char, "Block_1x1_4");
    case EShadingRatePalette::Block_1x1_2: return oss << STRING_LITERAL(_Char, "Block_1x1_2");
    case EShadingRatePalette::Block_1x1_1: return oss << STRING_LITERAL(_Char, "Block_1x1_1");
    case EShadingRatePalette::Block_2x1_1: return oss << STRING_LITERAL(_Char, "Block_2x1_1");
    case EShadingRatePalette::Block_1x2_1: return oss << STRING_LITERAL(_Char, "Block_1x2_1");
    case EShadingRatePalette::Block_2x2_1: return oss << STRING_LITERAL(_Char, "Block_2x2_1");
    case EShadingRatePalette::Block_4x2_1: return oss << STRING_LITERAL(_Char, "Block_4x2_1");
    case EShadingRatePalette::Block_2x4_1: return oss << STRING_LITERAL(_Char, "Block_2x4_1");
    case EShadingRatePalette::Block_4x4_1: return oss << STRING_LITERAL(_Char, "Block_4x4_1");
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EPixelFormat value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EPixelFormat>);
    switch (value) {
    case EPixelFormat::RGBA16_SNorm: return oss << STRING_LITERAL(_Char, "RGBA16_SNorm");
    case EPixelFormat::RGBA8_SNorm: return oss << STRING_LITERAL(_Char, "RGBA8_SNorm");
    case EPixelFormat::RGB16_SNorm: return oss << STRING_LITERAL(_Char, "RGB16_SNorm");
    case EPixelFormat::RGB8_SNorm: return oss << STRING_LITERAL(_Char, "RGB8_SNorm");
    case EPixelFormat::RG16_SNorm: return oss << STRING_LITERAL(_Char, "RG16_SNorm");
    case EPixelFormat::RG8_SNorm: return oss << STRING_LITERAL(_Char, "RG8_SNorm");
    case EPixelFormat::R16_SNorm: return oss << STRING_LITERAL(_Char, "R16_SNorm");
    case EPixelFormat::R8_SNorm: return oss << STRING_LITERAL(_Char, "R8_SNorm");
    case EPixelFormat::RGBA16_UNorm: return oss << STRING_LITERAL(_Char, "RGBA16_UNorm");
    case EPixelFormat::RGBA8_UNorm: return oss << STRING_LITERAL(_Char, "RGBA8_UNorm");
    case EPixelFormat::RGB16_UNorm: return oss << STRING_LITERAL(_Char, "RGB16_UNorm");
    case EPixelFormat::RGB8_UNorm: return oss << STRING_LITERAL(_Char, "RGB8_UNorm");
    case EPixelFormat::RG16_UNorm: return oss << STRING_LITERAL(_Char, "RG16_UNorm");
    case EPixelFormat::RG8_UNorm: return oss << STRING_LITERAL(_Char, "RG8_UNorm");
    case EPixelFormat::R16_UNorm: return oss << STRING_LITERAL(_Char, "R16_UNorm");
    case EPixelFormat::R8_UNorm: return oss << STRING_LITERAL(_Char, "R8_UNorm");
    case EPixelFormat::RGB10_A2_UNorm: return oss << STRING_LITERAL(_Char, "RGB10_A2_UNorm");
    case EPixelFormat::RGBA4_UNorm: return oss << STRING_LITERAL(_Char, "RGBA4_UNorm");
    case EPixelFormat::RGB5_A1_UNorm: return oss << STRING_LITERAL(_Char, "RGB5_A1_UNorm");
    case EPixelFormat::RGB_5_6_5_UNorm: return oss << STRING_LITERAL(_Char, "RGB_5_6_5_UNorm");
    case EPixelFormat::BGR8_UNorm: return oss << STRING_LITERAL(_Char, "BGR8_UNorm");
    case EPixelFormat::BGRA8_UNorm: return oss << STRING_LITERAL(_Char, "BGRA8_UNorm");
    case EPixelFormat::sRGB8: return oss << STRING_LITERAL(_Char, "sRGB8");
    case EPixelFormat::sRGB8_A8: return oss << STRING_LITERAL(_Char, "sRGB8_A8");
    case EPixelFormat::sBGR8: return oss << STRING_LITERAL(_Char, "sBGR8");
    case EPixelFormat::sBGR8_A8: return oss << STRING_LITERAL(_Char, "sBGR8_A8");
    case EPixelFormat::R8i: return oss << STRING_LITERAL(_Char, "R8i");
    case EPixelFormat::RG8i: return oss << STRING_LITERAL(_Char, "RG8i");
    case EPixelFormat::RGB8i: return oss << STRING_LITERAL(_Char, "RGB8i");
    case EPixelFormat::RGBA8i: return oss << STRING_LITERAL(_Char, "RGBA8i");
    case EPixelFormat::R16i: return oss << STRING_LITERAL(_Char, "R16i");
    case EPixelFormat::RG16i: return oss << STRING_LITERAL(_Char, "RG16i");
    case EPixelFormat::RGB16i: return oss << STRING_LITERAL(_Char, "RGB16i");
    case EPixelFormat::RGBA16i: return oss << STRING_LITERAL(_Char, "RGBA16i");
    case EPixelFormat::R32i: return oss << STRING_LITERAL(_Char, "R32i");
    case EPixelFormat::RG32i: return oss << STRING_LITERAL(_Char, "RG32i");
    case EPixelFormat::RGB32i: return oss << STRING_LITERAL(_Char, "RGB32i");
    case EPixelFormat::RGBA32i: return oss << STRING_LITERAL(_Char, "RGBA32i");
    case EPixelFormat::R8u: return oss << STRING_LITERAL(_Char, "R8u");
    case EPixelFormat::RG8u: return oss << STRING_LITERAL(_Char, "RG8u");
    case EPixelFormat::RGB8u: return oss << STRING_LITERAL(_Char, "RGB8u");
    case EPixelFormat::RGBA8u: return oss << STRING_LITERAL(_Char, "RGBA8u");
    case EPixelFormat::R16u: return oss << STRING_LITERAL(_Char, "R16u");
    case EPixelFormat::RG16u: return oss << STRING_LITERAL(_Char, "RG16u");
    case EPixelFormat::RGB16u: return oss << STRING_LITERAL(_Char, "RGB16u");
    case EPixelFormat::RGBA16u: return oss << STRING_LITERAL(_Char, "RGBA16u");
    case EPixelFormat::R32u: return oss << STRING_LITERAL(_Char, "R32u");
    case EPixelFormat::RG32u: return oss << STRING_LITERAL(_Char, "RG32u");
    case EPixelFormat::RGB32u: return oss << STRING_LITERAL(_Char, "RGB32u");
    case EPixelFormat::RGBA32u: return oss << STRING_LITERAL(_Char, "RGBA32u");
    case EPixelFormat::RGB10_A2u: return oss << STRING_LITERAL(_Char, "RGB10_A2u");
    case EPixelFormat::R16f: return oss << STRING_LITERAL(_Char, "R16f");
    case EPixelFormat::RG16f: return oss << STRING_LITERAL(_Char, "RG16f");
    case EPixelFormat::RGB16f: return oss << STRING_LITERAL(_Char, "RGB16f");
    case EPixelFormat::RGBA16f: return oss << STRING_LITERAL(_Char, "RGBA16f");
    case EPixelFormat::R32f: return oss << STRING_LITERAL(_Char, "R32f");
    case EPixelFormat::RG32f: return oss << STRING_LITERAL(_Char, "RG32f");
    case EPixelFormat::RGB32f: return oss << STRING_LITERAL(_Char, "RGB32f");
    case EPixelFormat::RGBA32f: return oss << STRING_LITERAL(_Char, "RGBA32f");
    case EPixelFormat::RGB_11_11_10f: return oss << STRING_LITERAL(_Char, "RGB_11_11_10f");
    case EPixelFormat::Depth16: return oss << STRING_LITERAL(_Char, "Depth16");
    case EPixelFormat::Depth24: return oss << STRING_LITERAL(_Char, "Depth24");
    case EPixelFormat::Depth32f: return oss << STRING_LITERAL(_Char, "Depth32f");
    case EPixelFormat::Depth16_Stencil8: return oss << STRING_LITERAL(_Char, "Depth16_Stencil8");
    case EPixelFormat::Depth24_Stencil8: return oss << STRING_LITERAL(_Char, "Depth24_Stencil8");
    case EPixelFormat::Depth32F_Stencil8: return oss << STRING_LITERAL(_Char, "Depth32F_Stencil8");
    case EPixelFormat::BC1_RGB8_UNorm: return oss << STRING_LITERAL(_Char, "BC1_RGB8_UNorm");
    case EPixelFormat::BC1_sRGB8: return oss << STRING_LITERAL(_Char, "BC1_sRGB8");
    case EPixelFormat::BC1_RGB8_A1_UNorm: return oss << STRING_LITERAL(_Char, "BC1_RGB8_A1_UNorm");
    case EPixelFormat::BC1_sRGB8_A1: return oss << STRING_LITERAL(_Char, "BC1_sRGB8_A1");
    case EPixelFormat::BC2_RGBA8_UNorm: return oss << STRING_LITERAL(_Char, "BC2_RGBA8_UNorm");
    case EPixelFormat::BC2_sRGB8_A8: return oss << STRING_LITERAL(_Char, "BC2_sRGB8_A8");
    case EPixelFormat::BC3_RGBA8_UNorm: return oss << STRING_LITERAL(_Char, "BC3_RGBA8_UNorm");
    case EPixelFormat::BC3_sRGB8: return oss << STRING_LITERAL(_Char, "BC3_sRGB8");
    case EPixelFormat::BC4_R8_SNorm: return oss << STRING_LITERAL(_Char, "BC4_R8_SNorm");
    case EPixelFormat::BC4_R8_UNorm: return oss << STRING_LITERAL(_Char, "BC4_R8_UNorm");
    case EPixelFormat::BC5_RG8_SNorm: return oss << STRING_LITERAL(_Char, "BC5_RG8_SNorm");
    case EPixelFormat::BC5_RG8_UNorm: return oss << STRING_LITERAL(_Char, "BC5_RG8_UNorm");
    case EPixelFormat::BC7_RGBA8_UNorm: return oss << STRING_LITERAL(_Char, "BC7_RGBA8_UNorm");
    case EPixelFormat::BC7_sRGB8_A8: return oss << STRING_LITERAL(_Char, "BC7_sRGB8_A8");
    case EPixelFormat::BC6H_RGB16F: return oss << STRING_LITERAL(_Char, "BC6H_RGB16F");
    case EPixelFormat::BC6H_RGB16UF: return oss << STRING_LITERAL(_Char, "BC6H_RGB16UF");
    case EPixelFormat::ETC2_RGB8_UNorm: return oss << STRING_LITERAL(_Char, "ETC2_RGB8_UNorm");
    case EPixelFormat::ECT2_sRGB8: return oss << STRING_LITERAL(_Char, "ECT2_sRGB8");
    case EPixelFormat::ETC2_RGB8_A1_UNorm: return oss << STRING_LITERAL(_Char, "ETC2_RGB8_A1_UNorm");
    case EPixelFormat::ETC2_sRGB8_A1: return oss << STRING_LITERAL(_Char, "ETC2_sRGB8_A1");
    case EPixelFormat::ETC2_RGBA8_UNorm: return oss << STRING_LITERAL(_Char, "ETC2_RGBA8_UNorm");
    case EPixelFormat::ETC2_sRGB8_A8: return oss << STRING_LITERAL(_Char, "ETC2_sRGB8_A8");
    case EPixelFormat::EAC_R11_SNorm: return oss << STRING_LITERAL(_Char, "EAC_R11_SNorm");
    case EPixelFormat::EAC_R11_UNorm: return oss << STRING_LITERAL(_Char, "EAC_R11_UNorm");
    case EPixelFormat::EAC_RG11_SNorm: return oss << STRING_LITERAL(_Char, "EAC_RG11_SNorm");
    case EPixelFormat::EAC_RG11_UNorm: return oss << STRING_LITERAL(_Char, "EAC_RG11_UNorm");
    case EPixelFormat::ASTC_RGBA_4x4: return oss << STRING_LITERAL(_Char, "ASTC_RGBA_4x4");
    case EPixelFormat::ASTC_RGBA_5x4: return oss << STRING_LITERAL(_Char, "ASTC_RGBA_5x4");
    case EPixelFormat::ASTC_RGBA_5x5: return oss << STRING_LITERAL(_Char, "ASTC_RGBA_5x5");
    case EPixelFormat::ASTC_RGBA_6x5: return oss << STRING_LITERAL(_Char, "ASTC_RGBA_6x5");
    case EPixelFormat::ASTC_RGBA_6x6: return oss << STRING_LITERAL(_Char, "ASTC_RGBA_6x6");
    case EPixelFormat::ASTC_RGBA_8x5: return oss << STRING_LITERAL(_Char, "ASTC_RGBA_8x5");
    case EPixelFormat::ASTC_RGBA_8x6: return oss << STRING_LITERAL(_Char, "ASTC_RGBA_8x6");
    case EPixelFormat::ASTC_RGBA_8x8: return oss << STRING_LITERAL(_Char, "ASTC_RGBA_8x8");
    case EPixelFormat::ASTC_RGBA_10x5: return oss << STRING_LITERAL(_Char, "ASTC_RGBA_10x5");
    case EPixelFormat::ASTC_RGBA_10x6: return oss << STRING_LITERAL(_Char, "ASTC_RGBA_10x6");
    case EPixelFormat::ASTC_RGBA_10x8: return oss << STRING_LITERAL(_Char, "ASTC_RGBA_10x8");
    case EPixelFormat::ASTC_RGBA_10x10: return oss << STRING_LITERAL(_Char, "ASTC_RGBA_10x10");
    case EPixelFormat::ASTC_RGBA_12x10: return oss << STRING_LITERAL(_Char, "ASTC_RGBA_12x10");
    case EPixelFormat::ASTC_RGBA_12x12: return oss << STRING_LITERAL(_Char, "ASTC_RGBA_12x12");
    case EPixelFormat::ASTC_sRGB8_A8_4x4: return oss << STRING_LITERAL(_Char, "ASTC_sRGB8_A8_4x4");
    case EPixelFormat::ASTC_sRGB8_A8_5x4: return oss << STRING_LITERAL(_Char, "ASTC_sRGB8_A8_5x4");
    case EPixelFormat::ASTC_sRGB8_A8_5x5: return oss << STRING_LITERAL(_Char, "ASTC_sRGB8_A8_5x5");
    case EPixelFormat::ASTC_sRGB8_A8_6x5: return oss << STRING_LITERAL(_Char, "ASTC_sRGB8_A8_6x5");
    case EPixelFormat::ASTC_sRGB8_A8_6x6: return oss << STRING_LITERAL(_Char, "ASTC_sRGB8_A8_6x6");
    case EPixelFormat::ASTC_sRGB8_A8_8x5: return oss << STRING_LITERAL(_Char, "ASTC_sRGB8_A8_8x5");
    case EPixelFormat::ASTC_sRGB8_A8_8x6: return oss << STRING_LITERAL(_Char, "ASTC_sRGB8_A8_8x6");
    case EPixelFormat::ASTC_sRGB8_A8_8x8: return oss << STRING_LITERAL(_Char, "ASTC_sRGB8_A8_8x8");
    case EPixelFormat::ASTC_sRGB8_A8_10x5: return oss << STRING_LITERAL(_Char, "ASTC_sRGB8_A8_10x5");
    case EPixelFormat::ASTC_sRGB8_A8_10x6: return oss << STRING_LITERAL(_Char, "ASTC_sRGB8_A8_10x6");
    case EPixelFormat::ASTC_sRGB8_A8_10x8: return oss << STRING_LITERAL(_Char, "ASTC_sRGB8_A8_10x8");
    case EPixelFormat::ASTC_sRGB8_A8_10x10: return oss << STRING_LITERAL(_Char, "ASTC_sRGB8_A8_10x10");
    case EPixelFormat::ASTC_sRGB8_A8_12x10: return oss << STRING_LITERAL(_Char, "ASTC_sRGB8_A8_12x10");
    case EPixelFormat::ASTC_sRGB8_A8_12x12: return oss << STRING_LITERAL(_Char, "ASTC_sRGB8_A8_12x12");
    case EPixelFormat::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EColorSpace value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EColorSpace>);
    switch (value) {
    case EColorSpace::PASS_THROUGH: return oss << STRING_LITERAL(_Char, "PASS_THROUGH");
    case EColorSpace::SRGB_NONLINEAR: return oss << STRING_LITERAL(_Char, "SRGB_NONLINEAR");
    case EColorSpace::DISPLAY_P3_NONLINEAR: return oss << STRING_LITERAL(_Char, "DISPLAY_P3_NONLINEAR");
    case EColorSpace::EXTENDED_SRGB_LINEAR: return oss << STRING_LITERAL(_Char, "EXTENDED_SRGB_LINEAR");
    case EColorSpace::DISPLAY_P3_LINEAR: return oss << STRING_LITERAL(_Char, "DISPLAY_P3_LINEAR");
    case EColorSpace::DCI_P3_NONLINEAR: return oss << STRING_LITERAL(_Char, "DCI_P3_NONLINEAR");
    case EColorSpace::BT709_LINEAR: return oss << STRING_LITERAL(_Char, "BT709_LINEAR");
    case EColorSpace::BT709_NONLINEAR: return oss << STRING_LITERAL(_Char, "BT709_NONLINEAR");
    case EColorSpace::BT2020_LINEAR: return oss << STRING_LITERAL(_Char, "BT2020_LINEAR");
    case EColorSpace::HDR10_ST2084: return oss << STRING_LITERAL(_Char, "HDR10_ST2084");
    case EColorSpace::DOLBYVISION: return oss << STRING_LITERAL(_Char, "DOLBYVISION");
    case EColorSpace::HDR10_HLG: return oss << STRING_LITERAL(_Char, "HDR10_HLG");
    case EColorSpace::ADOBERGB_LINEAR: return oss << STRING_LITERAL(_Char, "ADOBERGB_LINEAR");
    case EColorSpace::ADOBERGB_NONLINEAR: return oss << STRING_LITERAL(_Char, "ADOBERGB_NONLINEAR");
    case EColorSpace::EXTENDED_SRGB_NONLINEAR: return oss << STRING_LITERAL(_Char, "EXTENDED_SRGB_NONLINEAR");
    case EColorSpace::DISPLAY_NATIVE_AMD: return oss << STRING_LITERAL(_Char, "DISPLAY_NATIVE_AMD");
    case EColorSpace::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EFragmentOutput value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EFragmentOutput>);
    switch (value) {
    case EFragmentOutput::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    case EFragmentOutput::Int4: return oss << STRING_LITERAL(_Char, "Int4");
    case EFragmentOutput::UInt4: return oss << STRING_LITERAL(_Char, "UInt4");
    case EFragmentOutput::Float4: return oss << STRING_LITERAL(_Char, "Float4");
    }
    return oss;
}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EDebugFlags value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<EDebugFlags>);
    if (EDebugFlags::Unknown == value) return oss << STRING_LITERAL(_Char, "Unknown");
    if (EDebugFlags::Default == value) return oss << STRING_LITERAL(_Char, "Default");

    auto sep = Separator_<_Char>();

    if (EDebugFlags::LogTasks & value) oss << sep << STRING_LITERAL(_Char, "LogTasks");
    if (EDebugFlags::LogBarriers & value) oss << sep << STRING_LITERAL(_Char, "LogBarriers");
    if (EDebugFlags::LogResourceUsage & value) oss << sep << STRING_LITERAL(_Char, "LogResourceUsage");
    if (EDebugFlags::VisTasks & value) oss << sep << STRING_LITERAL(_Char, "VisTasks");
    if (EDebugFlags::VisDrawTasks & value) oss << sep << STRING_LITERAL(_Char, "VisDrawTasks");
    if (EDebugFlags::VisResources & value) oss << sep << STRING_LITERAL(_Char, "VisResources");
    if (EDebugFlags::VisBarriers & value) oss << sep << STRING_LITERAL(_Char, "VisBarriers");
    if (EDebugFlags::VisBarrierLabels & value) oss << sep << STRING_LITERAL(_Char, "VisBarrierLabels");
    if (EDebugFlags::VisTaskDependencies & value) oss << sep << STRING_LITERAL(_Char, "VisTaskDependencies");
    if (EDebugFlags::FullBarrier & value) oss << sep << STRING_LITERAL(_Char, "FullBarrier");
    if (EDebugFlags::QueueSync & value) oss << sep << STRING_LITERAL(_Char, "QueueSync");

    return oss;
}
#endif
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EBlendFactor value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EBlendFactor>);
    switch (value) {
    case EBlendFactor::Zero: return oss << STRING_LITERAL(_Char, "Zero");
    case EBlendFactor::One: return oss << STRING_LITERAL(_Char, "One");
    case EBlendFactor::SrcColor: return oss << STRING_LITERAL(_Char, "SrcColor");
    case EBlendFactor::OneMinusSrcColor: return oss << STRING_LITERAL(_Char, "OneMinusSrcColor");
    case EBlendFactor::DstColor: return oss << STRING_LITERAL(_Char, "DstColor");
    case EBlendFactor::OneMinusDstColor: return oss << STRING_LITERAL(_Char, "OneMinusDstColor");
    case EBlendFactor::SrcAlpha: return oss << STRING_LITERAL(_Char, "SrcAlpha");
    case EBlendFactor::OneMinusSrcAlpha: return oss << STRING_LITERAL(_Char, "OneMinusSrcAlpha");
    case EBlendFactor::DstAlpha: return oss << STRING_LITERAL(_Char, "DstAlpha");
    case EBlendFactor::OneMinusDstAlpha: return oss << STRING_LITERAL(_Char, "OneMinusDstAlpha");
    case EBlendFactor::ConstColor: return oss << STRING_LITERAL(_Char, "ConstColor");
    case EBlendFactor::OneMinusConstColor: return oss << STRING_LITERAL(_Char, "OneMinusConstColor");
    case EBlendFactor::ConstAlpha: return oss << STRING_LITERAL(_Char, "ConstAlpha");
    case EBlendFactor::OneMinusConstAlpha: return oss << STRING_LITERAL(_Char, "OneMinusConstAlpha");
    case EBlendFactor::SrcAlphaSaturate: return oss << STRING_LITERAL(_Char, "SrcAlphaSaturate");
    case EBlendFactor::Src1Color: return oss << STRING_LITERAL(_Char, "Src1Color");
    case EBlendFactor::OneMinusSrc1Color: return oss << STRING_LITERAL(_Char, "OneMinusSrc1Color");
    case EBlendFactor::Src1Alpha: return oss << STRING_LITERAL(_Char, "Src1Alpha");
    case EBlendFactor::OneMinusSrc1Alpha: return oss << STRING_LITERAL(_Char, "OneMinusSrc1Alpha");
    case EBlendFactor::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EBlendOp value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EBlendOp>);
    switch (value) {
    case EBlendOp::Add: return oss << STRING_LITERAL(_Char, "Add");
    case EBlendOp::Sub: return oss << STRING_LITERAL(_Char, "Sub");
    case EBlendOp::RevSub: return oss << STRING_LITERAL(_Char, "RevSub");
    case EBlendOp::Min: return oss << STRING_LITERAL(_Char, "Min");
    case EBlendOp::Max: return oss << STRING_LITERAL(_Char, "Max");
    case EBlendOp::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, ELogicOp value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<ELogicOp>);
    switch (value) {
    case ELogicOp::None: return oss << STRING_LITERAL(_Char, "None");
    case ELogicOp::Clear: return oss << STRING_LITERAL(_Char, "Clear");
    case ELogicOp::Set: return oss << STRING_LITERAL(_Char, "Set");
    case ELogicOp::Copy: return oss << STRING_LITERAL(_Char, "Copy");
    case ELogicOp::CopyInverted: return oss << STRING_LITERAL(_Char, "CopyInverted");
    case ELogicOp::NoOp: return oss << STRING_LITERAL(_Char, "NoOp");
    case ELogicOp::Invert: return oss << STRING_LITERAL(_Char, "Invert");
    case ELogicOp::And: return oss << STRING_LITERAL(_Char, "And");
    case ELogicOp::NotAnd: return oss << STRING_LITERAL(_Char, "NotAnd");
    case ELogicOp::Or: return oss << STRING_LITERAL(_Char, "Or");
    case ELogicOp::NotOr: return oss << STRING_LITERAL(_Char, "NotOr");
    case ELogicOp::Xor: return oss << STRING_LITERAL(_Char, "Xor");
    case ELogicOp::Equiv: return oss << STRING_LITERAL(_Char, "Equiv");
    case ELogicOp::AndReverse: return oss << STRING_LITERAL(_Char, "AndReverse");
    case ELogicOp::AndInverted: return oss << STRING_LITERAL(_Char, "AndInverted");
    case ELogicOp::OrReverse: return oss << STRING_LITERAL(_Char, "OrReverse");
    case ELogicOp::OrInverted: return oss << STRING_LITERAL(_Char, "OrInverted");
    case ELogicOp::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EColorMask value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<EColorMask>);
    if (EColorMask::RGBA == value) return oss << STRING_LITERAL(_Char, "RGBA");
    if (EColorMask::All == value) return oss << STRING_LITERAL(_Char, "All");
    if (EColorMask::Default == value) return oss << STRING_LITERAL(_Char, "Default");
    if (EColorMask::Unknown == value) return oss << STRING_LITERAL(_Char, "Unknown");

    if (EColorMask::R & value) oss << STRING_LITERAL(_Char, "R");
    if (EColorMask::G & value) oss << STRING_LITERAL(_Char, "G");
    if (EColorMask::B & value) oss << STRING_LITERAL(_Char, "B");
    if (EColorMask::A & value) oss << STRING_LITERAL(_Char, "A");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, ECompareOp value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<ECompareOp>);
    switch (value) {
    case ECompareOp::Never: return oss << STRING_LITERAL(_Char, "Never");
    case ECompareOp::Less: return oss << STRING_LITERAL(_Char, "Less");
    case ECompareOp::Equal: return oss << STRING_LITERAL(_Char, "Equal");
    case ECompareOp::LessEqual: return oss << STRING_LITERAL(_Char, "LessEqual");
    case ECompareOp::Greater: return oss << STRING_LITERAL(_Char, "Greater");
    case ECompareOp::NotEqual: return oss << STRING_LITERAL(_Char, "NotEqual");
    case ECompareOp::GreaterEqual: return oss << STRING_LITERAL(_Char, "GreaterEqual");
    case ECompareOp::Always: return oss << STRING_LITERAL(_Char, "Always");
    case ECompareOp::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EStencilOp value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EStencilOp>);
    switch (value) {
    case EStencilOp::Keep: return oss << STRING_LITERAL(_Char, "Keep");
    case EStencilOp::Zero: return oss << STRING_LITERAL(_Char, "Zero");
    case EStencilOp::Replace: return oss << STRING_LITERAL(_Char, "Replace");
    case EStencilOp::Incr: return oss << STRING_LITERAL(_Char, "Incr");
    case EStencilOp::IncrWrap: return oss << STRING_LITERAL(_Char, "IncrWrap");
    case EStencilOp::Decr: return oss << STRING_LITERAL(_Char, "Decr");
    case EStencilOp::DecrWrap: return oss << STRING_LITERAL(_Char, "DecrWrap");
    case EStencilOp::Invert: return oss << STRING_LITERAL(_Char, "Invert");
    case EStencilOp::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EPolygonMode value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EPolygonMode>);
    switch (value) {
    case EPolygonMode::Point: return oss << STRING_LITERAL(_Char, "Point");
    case EPolygonMode::Line: return oss << STRING_LITERAL(_Char, "Line");
    case EPolygonMode::Fill: return oss << STRING_LITERAL(_Char, "Fill");
    case EPolygonMode::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EPrimitiveTopology value) {
    switch (value) {
    case EPrimitiveTopology::Point: return oss << STRING_LITERAL(_Char, "Point");
    case EPrimitiveTopology::LineList: return oss << STRING_LITERAL(_Char, "LineList");
    case EPrimitiveTopology::LineStrip: return oss << STRING_LITERAL(_Char, "LineStrip");
    case EPrimitiveTopology::LineListAdjacency: return oss << STRING_LITERAL(_Char, "LineListAdjacency");
    case EPrimitiveTopology::LineStripAdjacency: return oss << STRING_LITERAL(_Char, "LineStripAdjacency");
    case EPrimitiveTopology::TriangleList: return oss << STRING_LITERAL(_Char, "TriangleList");
    case EPrimitiveTopology::TriangleStrip: return oss << STRING_LITERAL(_Char, "TriangleStrip");
    case EPrimitiveTopology::TriangleFan: return oss << STRING_LITERAL(_Char, "TriangleFan");
    case EPrimitiveTopology::TriangleListAdjacency: return oss << STRING_LITERAL(_Char, "TriangleListAdjacency");
    case EPrimitiveTopology::TriangleStripAdjacency: return oss << STRING_LITERAL(_Char, "TriangleStripAdjacency");
    case EPrimitiveTopology::Patch: return oss << STRING_LITERAL(_Char, "Patch");
    case EPrimitiveTopology::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, ECullMode value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<ECullMode>);
    switch (value) {
    case ECullMode::None: return oss << STRING_LITERAL(_Char, "None");
    case ECullMode::Front: return oss << STRING_LITERAL(_Char, "Front");
    case ECullMode::Back: return oss << STRING_LITERAL(_Char, "Back");
    case ECullMode::FontAndBack: return oss << STRING_LITERAL(_Char, "FontAndBack");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EPipelineDynamicState value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<EPipelineDynamicState>);
    if (EPipelineDynamicState::All == value) return oss << STRING_LITERAL(_Char, "All");
    if (EPipelineDynamicState::Unknown == value) return oss << STRING_LITERAL(_Char, "Unknown");
    if (EPipelineDynamicState::Default == value) return oss << STRING_LITERAL(_Char, "Default");

    auto sep = Separator_<_Char>();

    if (EPipelineDynamicState::Viewport & value) oss << sep << STRING_LITERAL(_Char, "Viewport");
    if (EPipelineDynamicState::Scissor & value) oss << sep << STRING_LITERAL(_Char, "Scissor");
    if (EPipelineDynamicState::StencilCompareMask & value) oss << sep << STRING_LITERAL(_Char, "StencilCompareMask");
    if (EPipelineDynamicState::StencilWriteMask & value) oss << sep << STRING_LITERAL(_Char, "StencilWriteMask");
    if (EPipelineDynamicState::StencilReference & value) oss << sep << STRING_LITERAL(_Char, "StencilReference");
    if (EPipelineDynamicState::ShadingRatePalette & value) oss << sep << STRING_LITERAL(_Char, "ShadingRatePalette");
    if (EPipelineDynamicState::RasterizerMask & value) oss << sep << STRING_LITERAL(_Char, "RasterizerMask");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, ERayTracingGeometryFlags value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<ERayTracingGeometryFlags>);
    if (ERayTracingGeometryFlags::Unknown == value) return oss << STRING_LITERAL(_Char, "Unknown");

    auto sep = Separator_<_Char>();

    if (ERayTracingGeometryFlags::Opaque & value) return oss << sep << STRING_LITERAL(_Char, "Opaque");
    if (ERayTracingGeometryFlags::NoDuplicateAnyHitInvocation & value) return oss << sep << STRING_LITERAL(_Char, "NoDuplicateAnyHitInvocation");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, ERayTracingInstanceFlags value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<ERayTracingInstanceFlags>);
    if (ERayTracingInstanceFlags::Unknown == value) return oss << STRING_LITERAL(_Char, "Unknown");

    auto sep = Separator_<_Char>();

    if (ERayTracingInstanceFlags::TriangleCullDisable & value) return oss << sep << STRING_LITERAL(_Char, "TriangleCullDisable");
    if (ERayTracingInstanceFlags::TriangleFrontCCW & value) return oss << sep << STRING_LITERAL(_Char, "TriangleFrontCCW");
    if (ERayTracingInstanceFlags::ForceOpaque & value) return oss << sep << STRING_LITERAL(_Char, "ForceOpaque");
    if (ERayTracingInstanceFlags::ForceNonOpaque & value) return oss << sep << STRING_LITERAL(_Char, "ForceNonOpaque");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, ERayTracingBuildFlags value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<ERayTracingBuildFlags>);
    if (ERayTracingBuildFlags::Unknown == value) return oss << STRING_LITERAL(_Char, "Unknown");

    auto sep = Separator_<_Char>();

    if (ERayTracingBuildFlags::AllowUpdate & value) return oss << sep << STRING_LITERAL(_Char, "AllowUpdate");
    if (ERayTracingBuildFlags::AllowCompaction & value) return oss << sep << STRING_LITERAL(_Char, "AllowCompaction");
    if (ERayTracingBuildFlags::PreferFastTrace & value) return oss << sep << STRING_LITERAL(_Char, "PreferFastTrace");
    if (ERayTracingBuildFlags::PreferFastBuild & value) return oss << sep << STRING_LITERAL(_Char, "PreferFastBuild");
    if (ERayTracingBuildFlags::LowMemory & value) return oss << sep << STRING_LITERAL(_Char, "LowMemory");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EShaderType value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EShaderType>);
    switch (value) {
    case EShaderType::Vertex: return oss << STRING_LITERAL(_Char, "Vertex");
    case EShaderType::TessControl: return oss << STRING_LITERAL(_Char, "TessControl");
    case EShaderType::TessEvaluation: return oss << STRING_LITERAL(_Char, "TessEvaluation");
    case EShaderType::Geometry: return oss << STRING_LITERAL(_Char, "Geometry");
    case EShaderType::Fragment: return oss << STRING_LITERAL(_Char, "Fragment");
    case EShaderType::Compute: return oss << STRING_LITERAL(_Char, "Compute");
    case EShaderType::MeshTask: return oss << STRING_LITERAL(_Char, "MeshTask");
    case EShaderType::Mesh: return oss << STRING_LITERAL(_Char, "Mesh");
    case EShaderType::RayGen: return oss << STRING_LITERAL(_Char, "RayGen");
    case EShaderType::RayAnyHit: return oss << STRING_LITERAL(_Char, "RayAnyHit");
    case EShaderType::RayClosestHit: return oss << STRING_LITERAL(_Char, "RayClosestHit");
    case EShaderType::RayMiss: return oss << STRING_LITERAL(_Char, "RayMiss");
    case EShaderType::RayIntersection: return oss << STRING_LITERAL(_Char, "RayIntersection");
    case EShaderType::RayCallable: return oss << STRING_LITERAL(_Char, "RayCallable");
    case EShaderType::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EShaderStages value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<EShaderStages>);
    if (EShaderStages::All == value) return oss << STRING_LITERAL(_Char, "All");
    if (EShaderStages::AllGraphics == value) return oss << STRING_LITERAL(_Char, "AllGraphics");
    if (EShaderStages::AllRayTracing == value) return oss << STRING_LITERAL(_Char, "AllRayTracing");
    if (EShaderStages::Unknown == value) return oss << STRING_LITERAL(_Char, "Unknown");

    auto sep = Separator_<_Char>();

    if (EShaderStages::Vertex & value) oss << sep << STRING_LITERAL(_Char, "Vertex");
    if (EShaderStages::TessControl & value) oss << sep << STRING_LITERAL(_Char, "TessControl");
    if (EShaderStages::TessEvaluation & value) oss << sep << STRING_LITERAL(_Char, "TessEvaluation");
    if (EShaderStages::Geometry & value) oss << sep << STRING_LITERAL(_Char, "Geometry");
    if (EShaderStages::Fragment & value) oss << sep << STRING_LITERAL(_Char, "Fragment");
    if (EShaderStages::Compute & value) oss << sep << STRING_LITERAL(_Char, "Compute");
    if (EShaderStages::MeshTask & value) oss << sep << STRING_LITERAL(_Char, "MeshTask");
    if (EShaderStages::Mesh & value) oss << sep << STRING_LITERAL(_Char, "Mesh");
    if (EShaderStages::RayGen & value) oss << sep << STRING_LITERAL(_Char, "RayGen");
    if (EShaderStages::RayAnyHit & value) oss << sep << STRING_LITERAL(_Char, "RayAnyHit");
    if (EShaderStages::RayClosestHit & value) oss << sep << STRING_LITERAL(_Char, "RayClosestHit");
    if (EShaderStages::RayMiss & value) oss << sep << STRING_LITERAL(_Char, "RayMiss");
    if (EShaderStages::RayIntersection & value) oss << sep << STRING_LITERAL(_Char, "RayIntersection");
    if (EShaderStages::RayCallable & value) oss << sep << STRING_LITERAL(_Char, "RayCallable");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EShaderAccess value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EShaderAccess>);
    switch (value) {
    case EShaderAccess::ReadOnly: return oss << STRING_LITERAL(_Char, "ReadOnly");
    case EShaderAccess::WriteOnly: return oss << STRING_LITERAL(_Char, "WriteOnly");
    case EShaderAccess::WriteDiscard: return oss << STRING_LITERAL(_Char, "WriteDiscard");
    case EShaderAccess::ReadWrite: return oss << STRING_LITERAL(_Char, "ReadWrite");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EShaderLangFormat value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<EShaderLangFormat>);
    auto sep = Separator_<_Char>();

    if (EShaderLangFormat::Vulkan & value) oss << sep << STRING_LITERAL(_Char, "Vulkan");
    if (EShaderLangFormat::OpenGL & value) oss << sep << STRING_LITERAL(_Char, "OpenGL");
    if (EShaderLangFormat::OpenGLES & value) oss << sep << STRING_LITERAL(_Char, "OpenGLES");
    if (EShaderLangFormat::DirectX & value) oss << sep << STRING_LITERAL(_Char, "DirectX");
    if (EShaderLangFormat::Vulkan_100 & value) oss << sep << STRING_LITERAL(_Char, "Vulkan_100");
    if (EShaderLangFormat::Vulkan_110 & value) oss << sep << STRING_LITERAL(_Char, "Vulkan_110");
    if (EShaderLangFormat::Vulkan_120 & value) oss << sep << STRING_LITERAL(_Char, "Vulkan_120");
    if (EShaderLangFormat::OpenGL_450 & value) oss << sep << STRING_LITERAL(_Char, "OpenGL_450");
    if (EShaderLangFormat::OpenGL_460 & value) oss << sep << STRING_LITERAL(_Char, "OpenGL_460");
    if (EShaderLangFormat::OpenGLES_200 & value) oss << sep << STRING_LITERAL(_Char, "OpenGLES_200");
    if (EShaderLangFormat::OpenGLES_320 & value) oss << sep << STRING_LITERAL(_Char, "OpenGLES_320");
    if (EShaderLangFormat::DirectX_11 & value) oss << sep << STRING_LITERAL(_Char, "DirectX_11");
    if (EShaderLangFormat::DirectX_12 & value) oss << sep << STRING_LITERAL(_Char, "DirectX_12");
    if (EShaderLangFormat::Source & value) oss << sep << STRING_LITERAL(_Char, "Source");
    if (EShaderLangFormat::Binary & value) oss << sep << STRING_LITERAL(_Char, "Binary");
    if (EShaderLangFormat::Executable & value) oss << sep << STRING_LITERAL(_Char, "Executable");
    if (EShaderLangFormat::HighLevel & value) oss << sep << STRING_LITERAL(_Char, "HighLevel");
    if (EShaderLangFormat::SPIRV & value) oss << sep << STRING_LITERAL(_Char, "SPIRV");
    if (EShaderLangFormat::ShaderModule & value) oss << sep << STRING_LITERAL(_Char, "ShaderModule");
    if (EShaderLangFormat::EnableDebugTrace & value) oss << sep << STRING_LITERAL(_Char, "EnableDebugTrace");
    if (EShaderLangFormat::EnableProfiling & value) oss << sep << STRING_LITERAL(_Char, "EnableProfiling");
    if (EShaderLangFormat::EnableTimeMap & value) oss << sep << STRING_LITERAL(_Char, "EnableTimeMap");
    if (EShaderLangFormat::Unknown & value) oss << sep << STRING_LITERAL(_Char, "Unknown");
    if (EShaderLangFormat::GLSL_450 & value) oss << sep << STRING_LITERAL(_Char, "GLSL_450");
    if (EShaderLangFormat::GLSL_460 & value) oss << sep << STRING_LITERAL(_Char, "GLSL_460");
    if (EShaderLangFormat::HLSL_11 & value) oss << sep << STRING_LITERAL(_Char, "HLSL_11");
    if (EShaderLangFormat::HLSL_12 & value) oss << sep << STRING_LITERAL(_Char, "HLSL_12");
    if (EShaderLangFormat::VKSL_100 & value) oss << sep << STRING_LITERAL(_Char, "VKSL_100");
    if (EShaderLangFormat::VKSL_110 & value) oss << sep << STRING_LITERAL(_Char, "VKSL_110");
    if (EShaderLangFormat::VKSL_120 & value) oss << sep << STRING_LITERAL(_Char, "VKSL_120");
    if (EShaderLangFormat::SPIRV_100 & value) oss << sep << STRING_LITERAL(_Char, "SPIRV_100");
    if (EShaderLangFormat::SPIRV_110 & value) oss << sep << STRING_LITERAL(_Char, "SPIRV_110");
    if (EShaderLangFormat::SPIRV_120 & value) oss << sep << STRING_LITERAL(_Char, "SPIRV_120");
    if (EShaderLangFormat::VkShader_100 & value) oss << sep << STRING_LITERAL(_Char, "VkShader_100");
    if (EShaderLangFormat::VkShader_110 & value) oss << sep << STRING_LITERAL(_Char, "VkShader_110");
    if (EShaderLangFormat::VkShader_120 & value) oss << sep << STRING_LITERAL(_Char, "VkShader_120");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EShaderDebugMode value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EShaderDebugMode>);
    switch (value) {
    case EShaderDebugMode::None: return oss << STRING_LITERAL(_Char, "None");
#if USE_PPE_RHIDEBUG
    case EShaderDebugMode::Trace: return oss << STRING_LITERAL(_Char, "Trace");
    case EShaderDebugMode::Profiling: return oss << STRING_LITERAL(_Char, "Profiling");
    case EShaderDebugMode::Timemap: return oss << STRING_LITERAL(_Char, "Timemap");
#endif
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, ETextureFilter value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<ETextureFilter>);
    switch (value) {
    case ETextureFilter::Nearest: return oss << STRING_LITERAL(_Char, "Nearest");
    case ETextureFilter::Linear: return oss << STRING_LITERAL(_Char, "Linear");
    case ETextureFilter::Cubic: return oss << STRING_LITERAL(_Char, "Cubic");
    case ETextureFilter::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EMipmapFilter value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EMipmapFilter>);
    switch (value) {
    case EMipmapFilter::Nearest: return oss << STRING_LITERAL(_Char, "Nearest");
    case EMipmapFilter::Linear: return oss << STRING_LITERAL(_Char, "Linear");
    case EMipmapFilter::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EAddressMode value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EAddressMode>);
    switch (value) {
    case EAddressMode::Repeat: return oss << STRING_LITERAL(_Char, "Repeat");
    case EAddressMode::MirrorRepeat: return oss << STRING_LITERAL(_Char, "MirrorRepeat");
    case EAddressMode::ClampToEdge: return oss << STRING_LITERAL(_Char, "ClampToEdge");
    case EAddressMode::ClampToBorder: return oss << STRING_LITERAL(_Char, "ClampToBorder");
    case EAddressMode::MirrorClampToEdge: return oss << STRING_LITERAL(_Char, "MirrorClampToEdge");
    case EAddressMode::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EBorderColor value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EBorderColor>);
    switch (value) {
    case EBorderColor::FloatTransparentBlack: return oss << STRING_LITERAL(_Char, "FloatTransparentBlack");
    case EBorderColor::FloatOpaqueBlack: return oss << STRING_LITERAL(_Char, "FloatOpaqueBlack");
    case EBorderColor::FloatOpaqueWhite: return oss << STRING_LITERAL(_Char, "FloatOpaqueWhite");
    case EBorderColor::IntTransparentBlack: return oss << STRING_LITERAL(_Char, "IntTransparentBlack");
    case EBorderColor::IntOpaqueBlack: return oss << STRING_LITERAL(_Char, "IntOpaqueBlack");
    case EBorderColor::IntOpaqueWhite: return oss << STRING_LITERAL(_Char, "IntOpaqueWhite");
    case EBorderColor::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EIndexFormat value) {
    switch (value) {
    case EIndexFormat::UShort: return oss << STRING_LITERAL(_Char, "UShort");
    case EIndexFormat::UInt: return oss << STRING_LITERAL(_Char, "UInt");
    case EIndexFormat::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EVertexFormat value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<EVertexFormat>);
    switch (value) {
    case EVertexFormat::Byte: return oss << STRING_LITERAL(_Char, "Byte");
    case EVertexFormat::Byte2: return oss << STRING_LITERAL(_Char, "Byte2");
    case EVertexFormat::Byte3: return oss << STRING_LITERAL(_Char, "Byte3");
    case EVertexFormat::Byte4: return oss << STRING_LITERAL(_Char, "Byte4");
    case EVertexFormat::Byte_Norm: return oss << STRING_LITERAL(_Char, "Byte_Norm");
    case EVertexFormat::Byte2_Norm: return oss << STRING_LITERAL(_Char, "Byte2_Norm");
    case EVertexFormat::Byte3_Norm: return oss << STRING_LITERAL(_Char, "Byte3_Norm");
    case EVertexFormat::Byte4_Norm: return oss << STRING_LITERAL(_Char, "Byte4_Norm");
    case EVertexFormat::Byte_Scaled: return oss << STRING_LITERAL(_Char, "Byte_Scaled");
    case EVertexFormat::Byte2_Scaled: return oss << STRING_LITERAL(_Char, "Byte2_Scaled");
    case EVertexFormat::Byte3_Scaled: return oss << STRING_LITERAL(_Char, "Byte3_Scaled");
    case EVertexFormat::Byte4_Scaled: return oss << STRING_LITERAL(_Char, "Byte4_Scaled");
    case EVertexFormat::UByte: return oss << STRING_LITERAL(_Char, "UByte");
    case EVertexFormat::UByte2: return oss << STRING_LITERAL(_Char, "UByte2");
    case EVertexFormat::UByte3: return oss << STRING_LITERAL(_Char, "UByte3");
    case EVertexFormat::UByte4: return oss << STRING_LITERAL(_Char, "UByte4");
    case EVertexFormat::UByte_Norm: return oss << STRING_LITERAL(_Char, "UByte_Norm");
    case EVertexFormat::UByte2_Norm: return oss << STRING_LITERAL(_Char, "UByte2_Norm");
    case EVertexFormat::UByte3_Norm: return oss << STRING_LITERAL(_Char, "UByte3_Norm");
    case EVertexFormat::UByte4_Norm: return oss << STRING_LITERAL(_Char, "UByte4_Norm");
    case EVertexFormat::UByte_Scaled: return oss << STRING_LITERAL(_Char, "UByte_Scaled");
    case EVertexFormat::UByte2_Scaled: return oss << STRING_LITERAL(_Char, "UByte2_Scaled");
    case EVertexFormat::UByte3_Scaled: return oss << STRING_LITERAL(_Char, "UByte3_Scaled");
    case EVertexFormat::UByte4_Scaled: return oss << STRING_LITERAL(_Char, "UByte4_Scaled");
    case EVertexFormat::Short: return oss << STRING_LITERAL(_Char, "Short");
    case EVertexFormat::Short2: return oss << STRING_LITERAL(_Char, "Short2");
    case EVertexFormat::Short3: return oss << STRING_LITERAL(_Char, "Short3");
    case EVertexFormat::Short4: return oss << STRING_LITERAL(_Char, "Short4");
    case EVertexFormat::Short_Norm: return oss << STRING_LITERAL(_Char, "Short_Norm");
    case EVertexFormat::Short2_Norm: return oss << STRING_LITERAL(_Char, "Short2_Norm");
    case EVertexFormat::Short3_Norm: return oss << STRING_LITERAL(_Char, "Short3_Norm");
    case EVertexFormat::Short4_Norm: return oss << STRING_LITERAL(_Char, "Short4_Norm");
    case EVertexFormat::Short_Scaled: return oss << STRING_LITERAL(_Char, "Short_Scaled");
    case EVertexFormat::Short2_Scaled: return oss << STRING_LITERAL(_Char, "Short2_Scaled");
    case EVertexFormat::Short3_Scaled: return oss << STRING_LITERAL(_Char, "Short3_Scaled");
    case EVertexFormat::Short4_Scaled: return oss << STRING_LITERAL(_Char, "Short4_Scaled");
    case EVertexFormat::UShort: return oss << STRING_LITERAL(_Char, "UShort");
    case EVertexFormat::UShort2: return oss << STRING_LITERAL(_Char, "UShort2");
    case EVertexFormat::UShort3: return oss << STRING_LITERAL(_Char, "UShort3");
    case EVertexFormat::UShort4: return oss << STRING_LITERAL(_Char, "UShort4");
    case EVertexFormat::UShort_Norm: return oss << STRING_LITERAL(_Char, "UShort_Norm");
    case EVertexFormat::UShort2_Norm: return oss << STRING_LITERAL(_Char, "UShort2_Norm");
    case EVertexFormat::UShort3_Norm: return oss << STRING_LITERAL(_Char, "UShort3_Norm");
    case EVertexFormat::UShort4_Norm: return oss << STRING_LITERAL(_Char, "UShort4_Norm");
    case EVertexFormat::UShort_Scaled: return oss << STRING_LITERAL(_Char, "UShort_Scaled");
    case EVertexFormat::UShort2_Scaled: return oss << STRING_LITERAL(_Char, "UShort2_Scaled");
    case EVertexFormat::UShort3_Scaled: return oss << STRING_LITERAL(_Char, "UShort3_Scaled");
    case EVertexFormat::UShort4_Scaled: return oss << STRING_LITERAL(_Char, "UShort4_Scaled");
    case EVertexFormat::Int: return oss << STRING_LITERAL(_Char, "Int");
    case EVertexFormat::Int2: return oss << STRING_LITERAL(_Char, "Int2");
    case EVertexFormat::Int3: return oss << STRING_LITERAL(_Char, "Int3");
    case EVertexFormat::Int4: return oss << STRING_LITERAL(_Char, "Int4");
    case EVertexFormat::UInt: return oss << STRING_LITERAL(_Char, "UInt");
    case EVertexFormat::UInt2: return oss << STRING_LITERAL(_Char, "UInt2");
    case EVertexFormat::UInt3: return oss << STRING_LITERAL(_Char, "UInt3");
    case EVertexFormat::UInt4: return oss << STRING_LITERAL(_Char, "UInt4");
    case EVertexFormat::Long: return oss << STRING_LITERAL(_Char, "Long");
    case EVertexFormat::Long2: return oss << STRING_LITERAL(_Char, "Long2");
    case EVertexFormat::Long3: return oss << STRING_LITERAL(_Char, "Long3");
    case EVertexFormat::Long4: return oss << STRING_LITERAL(_Char, "Long4");
    case EVertexFormat::ULong: return oss << STRING_LITERAL(_Char, "ULong");
    case EVertexFormat::ULong2: return oss << STRING_LITERAL(_Char, "ULong2");
    case EVertexFormat::ULong3: return oss << STRING_LITERAL(_Char, "ULong3");
    case EVertexFormat::ULong4: return oss << STRING_LITERAL(_Char, "ULong4");
    case EVertexFormat::Half: return oss << STRING_LITERAL(_Char, "Half");
    case EVertexFormat::Half2: return oss << STRING_LITERAL(_Char, "Half2");
    case EVertexFormat::Half3: return oss << STRING_LITERAL(_Char, "Half3");
    case EVertexFormat::Half4: return oss << STRING_LITERAL(_Char, "Half4");
    case EVertexFormat::Float: return oss << STRING_LITERAL(_Char, "Float");
    case EVertexFormat::Float2: return oss << STRING_LITERAL(_Char, "Float2");
    case EVertexFormat::Float3: return oss << STRING_LITERAL(_Char, "Float3");
    case EVertexFormat::Float4: return oss << STRING_LITERAL(_Char, "Float4");
    case EVertexFormat::Double: return oss << STRING_LITERAL(_Char, "Double");
    case EVertexFormat::Double2: return oss << STRING_LITERAL(_Char, "Double2");
    case EVertexFormat::Double3: return oss << STRING_LITERAL(_Char, "Double3");
    case EVertexFormat::Double4: return oss << STRING_LITERAL(_Char, "Double4");
    case EVertexFormat::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EPixelValueType value) {
    STATIC_ASSERT(Meta::enum_is_flags_v<EPixelValueType>);
    switch (value) {
    case EPixelValueType::SFloat: return oss << STRING_LITERAL(_Char, "SFloat");
    case EPixelValueType::UFloat: return oss << STRING_LITERAL(_Char, "UFloat");
    case EPixelValueType::UNorm: return oss << STRING_LITERAL(_Char, "UNorm");
    case EPixelValueType::SNorm: return oss << STRING_LITERAL(_Char, "SNorm");
    case EPixelValueType::Int: return oss << STRING_LITERAL(_Char, "Int");
    case EPixelValueType::UInt: return oss << STRING_LITERAL(_Char, "UInt");
    case EPixelValueType::Depth: return oss << STRING_LITERAL(_Char, "Depth");
    case EPixelValueType::Stencil: return oss << STRING_LITERAL(_Char, "Stencil");
    case EPixelValueType::DepthStencil: return oss << STRING_LITERAL(_Char, "DepthStencil");
    case EPixelValueType::sRGB: return oss << STRING_LITERAL(_Char, "sRGB");
    case EPixelValueType::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, EPresentMode value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<EPresentMode>);
    switch (value) {
    case EPresentMode::Immediate: return oss << STRING_LITERAL(_Char, "Immediate");
    case EPresentMode::Fifo: return oss << STRING_LITERAL(_Char, "Fifo");
    case EPresentMode::RelaxedFifo: return oss << STRING_LITERAL(_Char, "RelaxedFifo");
    case EPresentMode::Mailbox: return oss << STRING_LITERAL(_Char, "Mailbox");
    case EPresentMode::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
template <typename _Char> TBasicTextWriter<_Char>& ToString_(TBasicTextWriter<_Char>& oss, ESurfaceTransform value) {
    STATIC_ASSERT(not Meta::enum_is_flags_v<ESurfaceTransform>);
    switch (value) {
    case ESurfaceTransform::Identity: return oss << STRING_LITERAL(_Char, "Identity");
    case ESurfaceTransform::TransformRotate90: return oss << STRING_LITERAL(_Char, "TransformRotate90");
    case ESurfaceTransform::TransformRotate180: return oss << STRING_LITERAL(_Char, "TransformRotate180");
    case ESurfaceTransform::TransformRotate270: return oss << STRING_LITERAL(_Char, "TransformRotate270");
    case ESurfaceTransform::HorizontalMirror: return oss << STRING_LITERAL(_Char, "HorizontalMirror");
    case ESurfaceTransform::HorizontalMirror_TransformRotate90: return oss << STRING_LITERAL(_Char, "HorizontalMirror_TransformRotate90");
    case ESurfaceTransform::HorizontalMirror_TransformRotate180: return oss << STRING_LITERAL(_Char, "HorizontalMirror_TransformRotate180");
    case ESurfaceTransform::HorizontalMirror_TransformRotate270: return oss << STRING_LITERAL(_Char, "HorizontalMirror_TransformRotate270");
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define PPE_RHI_ENUMTOSTRING_DEF(_Enum) \
    FTextWriter& operator <<(FTextWriter& oss, _Enum value) { return ToString_(oss, value); } \
    FWTextWriter& operator <<(FWTextWriter& oss, _Enum value) { return ToString_(oss, value); }
//----------------------------------------------------------------------------
PPE_RHI_ENUMTOSTRING_DEF(EQueueType);
PPE_RHI_ENUMTOSTRING_DEF(EQueueUsage);
PPE_RHI_ENUMTOSTRING_DEF(EMemoryType);
PPE_RHI_ENUMTOSTRING_DEF(EBufferUsage);
PPE_RHI_ENUMTOSTRING_DEF(EImageDim);
PPE_RHI_ENUMTOSTRING_DEF(EImageView);
PPE_RHI_ENUMTOSTRING_DEF(EImageFlags);
PPE_RHI_ENUMTOSTRING_DEF(EImageUsage);
PPE_RHI_ENUMTOSTRING_DEF(EImageAspect);
PPE_RHI_ENUMTOSTRING_DEF(EImageSampler);
PPE_RHI_ENUMTOSTRING_DEF(EAttachmentStoreOp);
PPE_RHI_ENUMTOSTRING_DEF(EShadingRatePalette);
PPE_RHI_ENUMTOSTRING_DEF(EPixelFormat);
PPE_RHI_ENUMTOSTRING_DEF(EColorSpace);
PPE_RHI_ENUMTOSTRING_DEF(EFragmentOutput);
#if USE_PPE_RHIDEBUG
PPE_RHI_ENUMTOSTRING_DEF(EDebugFlags);
#endif
PPE_RHI_ENUMTOSTRING_DEF(EBlendFactor);
PPE_RHI_ENUMTOSTRING_DEF(EBlendOp);
PPE_RHI_ENUMTOSTRING_DEF(ELogicOp);
PPE_RHI_ENUMTOSTRING_DEF(EColorMask);
PPE_RHI_ENUMTOSTRING_DEF(ECompareOp);
PPE_RHI_ENUMTOSTRING_DEF(EStencilOp);
PPE_RHI_ENUMTOSTRING_DEF(EPolygonMode);
PPE_RHI_ENUMTOSTRING_DEF(EPrimitiveTopology);
PPE_RHI_ENUMTOSTRING_DEF(ECullMode);
PPE_RHI_ENUMTOSTRING_DEF(EPipelineDynamicState);
PPE_RHI_ENUMTOSTRING_DEF(ERayTracingGeometryFlags);
PPE_RHI_ENUMTOSTRING_DEF(ERayTracingInstanceFlags);
PPE_RHI_ENUMTOSTRING_DEF(ERayTracingBuildFlags);
PPE_RHI_ENUMTOSTRING_DEF(EShaderType);
PPE_RHI_ENUMTOSTRING_DEF(EShaderStages);
PPE_RHI_ENUMTOSTRING_DEF(EShaderAccess);
PPE_RHI_ENUMTOSTRING_DEF(EShaderLangFormat);
PPE_RHI_ENUMTOSTRING_DEF(EShaderDebugMode);
PPE_RHI_ENUMTOSTRING_DEF(ETextureFilter);
PPE_RHI_ENUMTOSTRING_DEF(EMipmapFilter);
PPE_RHI_ENUMTOSTRING_DEF(EAddressMode);
PPE_RHI_ENUMTOSTRING_DEF(EBorderColor);
PPE_RHI_ENUMTOSTRING_DEF(EIndexFormat);
PPE_RHI_ENUMTOSTRING_DEF(EVertexFormat);
PPE_RHI_ENUMTOSTRING_DEF(EPixelValueType);
PPE_RHI_ENUMTOSTRING_DEF(EPresentMode);
PPE_RHI_ENUMTOSTRING_DEF(ESurfaceTransform);
//----------------------------------------------------------------------------
#undef PPE_RHI_ENUMTOSTRING_DEF
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
