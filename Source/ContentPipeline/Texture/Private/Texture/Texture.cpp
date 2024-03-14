// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Texture/Texture.h"

#include "Texture/Texture2D.h"
#include "Texture/Texture2DArray.h"
#include "Texture/Texture3D.h"
#include "Texture/TextureCube.h"
#include "Texture/TextureCubeArray.h"
// #include "TextureModule.h"

#include "RHIApi.h"

// #include "RTTI/Macros-impl.h"

#include "Color/Color.h"
#include "Diagnostic/Logger.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// RTTI_CLASS_BEGIN(Texture, FTexture, Abstract)
// RTTI_PROPERTY_PRIVATE_FIELD(_bulkData)
// RTTI_PROPERTY_PRIVATE_FIELD(_textureGroup)
// RTTI_PROPERTY_PRIVATE_FIELD(_gammaSpace)
// RTTI_PROPERTY_PRIVATE_FIELD(_imageView)
// RTTI_PROPERTY_PRIVATE_FIELD(_pixelFormat)
// RTTI_PROPERTY_PRIVATE_FIELD(_mipmapFilter)
// RTTI_PROPERTY_PRIVATE_FIELD(_magFilter)
// RTTI_PROPERTY_PRIVATE_FIELD(_minFilter)
// RTTI_PROPERTY_PRIVATE_FIELD(_allowAnisotropy)
// RTTI_CLASS_END()
//----------------------------------------------------------------------------
FTexture::FTexture() NOEXCEPT = default;
//----------------------------------------------------------------------------
FTexture::FTexture(const FTextureProperties& properties, FBulkData&& data) NOEXCEPT
:   _data(std::move(data))
,   _properties(properties)
{}
//----------------------------------------------------------------------------
RHI::FImageID FTexture::CreateTextureRHI(RHI::IFrameGraph& fg) const {
    using namespace RHI;

    FImageDesc desc = FImageDesc{}
        .SetDimension(Dimensions())
        .SetFormat(Format())
        .SetMaxMipmaps(NumMips())
        .SetArrayLayers(ArraySize())
        .SetView(ImageView())
        .SetUsage(EImageUsage::Sampled | EImageUsage::TransferDst2);

#if USE_PPE_RHIDEBUG
    FString debugName = "@anon";
    if (Data().SourceFile())
        debugName = Data().SourceFile()->ToString();
#endif
    TAutoResource<FImageID> image(fg, fg.CreateImage(desc, Default ARGS_IF_RHIDEBUG(*debugName) ));
    PPE_LOG_CHECK(Texture, image.Valid());

    RHI::FCommandBufferBatch cmd{ fg.Begin(RHI::FCommandBufferDesc{}
        .SetName("ContentPipeline/CreateTextureRHI")) };
    PPE_LOG_CHECK(Texture, cmd.Valid());

    FSharedBuffer textureData = Data().LockRead();
    DEFERRED{ Data().UnlockRead(std::move(textureData)); };

    const RHI::FPixelFormatInfo pixelInfo = EPixelFormat_Infos(desc.Format);

    forrange(slice, 0, *desc.ArrayLayers) {
        FRawMemoryConst sliceData = pixelInfo
            .SliceRange(RHI::EImageAspect::Color, desc.Dimensions, *desc.MaxLevel, slice)
            .MakeView(textureData.MakeView());

        uint3 mipDimensions = desc.Dimensions;
        forrange(mip, 0, *desc.MaxLevel) {
            const FRawMemoryConst mipData = sliceData.Eat(pixelInfo.SizeInBytes(EImageAspect::Color, mipDimensions));

            const RHI::PFrameTask tUpdate = cmd->Task(RHI::FUpdateImage{}
                .SetImage(image, int3(0), RHI::FImageLayer(slice), RHI::FMipmapLevel(mip))
                .SetData(mipData, mipDimensions));
            PPE_LOG_CHECK(Texture, tUpdate.valid());

            mipDimensions = RHI::FPixelFormatInfo::NextMipDimensions(mipDimensions);
        }

        Assert_NoAssume(sliceData.empty());
    }

    PPE_LOG_CHECK(Texture, fg.Execute(cmd));
    return image.Release();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// RTTI_CLASS_BEGIN(Texture, FTexture2D, Public)
// RTTI_PROPERTY_PRIVATE_FIELD(_dimensions)
// RTTI_PROPERTY_PRIVATE_FIELD(_addressModeU)
// RTTI_PROPERTY_PRIVATE_FIELD(_addressModeV)
// RTTI_CLASS_END()
//----------------------------------------------------------------------------
FTexture2D::FTexture2D() NOEXCEPT
:   _dimensions(0)
,   _addressModeU(ETextureAddressMode::Unknown)
,   _addressModeV(ETextureAddressMode::Unknown)
{}
//----------------------------------------------------------------------------
FTexture2D::FTexture2D(
    const uint2& dimensions,
    const FTextureProperties& properties, FBulkData&& data,
    ETextureAddressMode addressModeU/* = Default */,
    ETextureAddressMode addressModeV/* = Default */) NOEXCEPT
:   FTexture(properties, std::move(data))
,   _dimensions(dimensions)
,   _addressModeU(addressModeU)
,   _addressModeV(addressModeV)
{}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// RTTI_CLASS_BEGIN(Texture, FTexture2DArray, Public)
// RTTI_PROPERTY_PRIVATE_FIELD(_dimensions)
// RTTI_PROPERTY_PRIVATE_FIELD(_numSlices)
// RTTI_PROPERTY_PRIVATE_FIELD(_addressModeU)
// RTTI_PROPERTY_PRIVATE_FIELD(_addressModeV)
// RTTI_CLASS_END()
//----------------------------------------------------------------------------
FTexture2DArray::FTexture2DArray() NOEXCEPT
:   _dimensions(0)
,   _numSlices(0)
,   _addressModeU(ETextureAddressMode::Unknown)
,   _addressModeV(ETextureAddressMode::Unknown)
{}
//----------------------------------------------------------------------------
FTexture2DArray::FTexture2DArray(
    const uint2& dimensions,
    u32 numSlices,
    const FTextureProperties& properties, FBulkData&& data,
    ETextureAddressMode addressModeU/* = Default */,
    ETextureAddressMode addressModeV/* = Default */) NOEXCEPT
:   FTexture(properties, std::move(data))
,   _dimensions(dimensions)
,   _numSlices(numSlices)
,   _addressModeU(addressModeU)
,   _addressModeV(addressModeV)
{}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// RTTI_CLASS_BEGIN(Texture, FTexture3D, Public)
// RTTI_PROPERTY_PRIVATE_FIELD(_dimensions)
// RTTI_PROPERTY_PRIVATE_FIELD(_addressModeU)
// RTTI_PROPERTY_PRIVATE_FIELD(_addressModeV)
// RTTI_PROPERTY_PRIVATE_FIELD(_addressModeW)
// RTTI_CLASS_END()
//----------------------------------------------------------------------------
FTexture3D::FTexture3D() NOEXCEPT
:   _dimensions(0)
,   _addressModeU(ETextureAddressMode::Unknown)
,   _addressModeV(ETextureAddressMode::Unknown)
,   _addressModeW(ETextureAddressMode::Unknown)
{}
//----------------------------------------------------------------------------
FTexture3D::FTexture3D(
    const uint3& dimensions,
    const FTextureProperties& properties, FBulkData&& data,
    ETextureAddressMode addressModeU/* = Default */,
    ETextureAddressMode addressModeV/* = Default */,
    ETextureAddressMode addressModeW/* = Default */) NOEXCEPT
:   FTexture(properties, std::move(data))
,   _dimensions(dimensions)
,   _addressModeU(addressModeU)
,   _addressModeV(addressModeV)
,   _addressModeW(addressModeW)
{}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// RTTI_CLASS_BEGIN(Texture, FTextureCube, Public)
// RTTI_PROPERTY_PRIVATE_FIELD(_dimensions)
// RTTI_PROPERTY_PRIVATE_FIELD(_addressModeU)
// RTTI_PROPERTY_PRIVATE_FIELD(_addressModeV)
// RTTI_CLASS_END()
//----------------------------------------------------------------------------
FTextureCube::FTextureCube() NOEXCEPT
:   _dimensions(0)
,   _addressModeU(ETextureAddressMode::Unknown)
,   _addressModeV(ETextureAddressMode::Unknown)
{}
//----------------------------------------------------------------------------
FTextureCube::FTextureCube(
    const uint2& dimensions,
    const FTextureProperties& properties, FBulkData&& data,
    ETextureAddressMode addressModeU/* = Default */,
    ETextureAddressMode addressModeV/* = Default */) NOEXCEPT
:   FTexture(properties, std::move(data))
,   _dimensions(dimensions)
,   _addressModeU(addressModeU)
,   _addressModeV(addressModeV)
{}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// RTTI_CLASS_BEGIN(Texture, FTextureCubeArray, Public)
// RTTI_PROPERTY_PRIVATE_FIELD(_dimensions)
// RTTI_PROPERTY_PRIVATE_FIELD(_numCubes)
// RTTI_PROPERTY_PRIVATE_FIELD(_addressModeU)
// RTTI_PROPERTY_PRIVATE_FIELD(_addressModeV)
// RTTI_CLASS_END()
//----------------------------------------------------------------------------
FTextureCubeArray::FTextureCubeArray() NOEXCEPT
:   _dimensions(0)
,   _numCubes(0)
,   _addressModeU(ETextureAddressMode::Unknown)
,   _addressModeV(ETextureAddressMode::Unknown)
{}
//----------------------------------------------------------------------------
FTextureCubeArray::FTextureCubeArray(
    const uint2& dimensions,
    u32 numCubes,
    const FTextureProperties& properties, FBulkData&& data,
    ETextureAddressMode addressModeU/* = Default */,
    ETextureAddressMode addressModeV/* = Default */) NOEXCEPT
:   FTexture(properties, std::move(data))
,   _dimensions(dimensions)
,   _numCubes(numCubes)
,   _addressModeU(addressModeU)
,   _addressModeV(addressModeV)
{}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
