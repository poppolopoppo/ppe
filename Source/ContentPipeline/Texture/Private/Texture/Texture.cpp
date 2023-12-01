// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Texture/Texture.h"

#include "Texture/Texture2D.h"
#include "Texture/Texture2DArray.h"
#include "Texture/Texture3D.h"
#include "Texture/TextureCube.h"
#include "Texture/TextureCubeArray.h"
// #include "TextureModule.h"

#include "RHI/ResourceEnums.h"

// #include "RTTI/Macros-impl.h"

#include "Color/Color.h"

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
FTexture::FTexture() NOEXCEPT
:   _gammaSpace(EGammaSpace::sRGB)
,   _imageView(ETextureImageView::Unknown)
,   _pixelFormat(ETexturePixelFormat::Unknown)
,   _mipmapFilter(ETextureMipmapFilter::Unknown)
,   _magFilter(ETextureSampleFilter::Unknown)
,   _minFilter(ETextureSampleFilter::Unknown)
,   _allowAnisotropy(false)
{}
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
