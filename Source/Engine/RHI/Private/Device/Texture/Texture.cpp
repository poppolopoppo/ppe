// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Texture.h"

#include "Device/IDeviceAPIEncapsulator.h"
#include "Device/DeviceResourceBuffer.h"
#include "SurfaceFormat.h"

#include "Container/Hash.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTexture::FTexture(EDeviceResourceType textureType, const FSurfaceFormat *format, EBufferMode mode, EBufferUsage usage, bool sharable)
:   FDeviceResourceSharable(textureType, sharable)
,   _usageAndMode(0)
,   _format(format) {
    Assert(format);
    Assert(format->SupportTexture());
    Assert( EDeviceResourceType::FTexture2D == textureType ||
            EDeviceResourceType::FTextureCube == textureType );

#ifdef WITH_PPE_ASSERT_RELEASE
    switch (usage)
    {
    case EBufferUsage::Default:
        AssertRelease(  EBufferMode::None == mode ||
                        EBufferMode::Write == mode);
        break;
    case EBufferUsage::Dynamic:
        AssertRelease(  EBufferMode::Write == mode);
        break;
    case EBufferUsage::Immutable:
        AssertRelease(  EBufferMode::None == mode);
        break;
    case EBufferUsage::Staging:
        AssertRelease(  EBufferMode::None != mode);
        break;
    default:
        AssertNotImplemented();
        break;
    }
#endif

    bitmode_type::InplaceSet(_usageAndMode, u32(mode));
    bitusage_type::InplaceSet(_usageAndMode, u32(usage));
}
//----------------------------------------------------------------------------
FTexture::~FTexture() {}
//----------------------------------------------------------------------------
bool FTexture::Available() const {
    return nullptr != TextureEntity();
}
//----------------------------------------------------------------------------
FDeviceAPIDependantEntity *FTexture::TerminalEntity() const {
    return TextureEntity();
}
//----------------------------------------------------------------------------
size_t FTexture::HashValue_() const {
    return hash_tuple(_usageAndMode, _format);
}
//----------------------------------------------------------------------------
bool FTexture::Match_(const FDeviceAPIDependantTexture& texture) const {
    return  texture.Mode() == Mode() &&
            texture.Usage() == Usage() &&
            texture.Format() == Format();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceAPIDependantTexture::FDeviceAPIDependantTexture(IDeviceAPIEncapsulator *device, const FTexture *resource)
:   TTypedDeviceAPIDependantEntity<FTexture>(device->APIEncapsulator(), resource)
,   _format(resource->Format())
,   _mode(resource->Mode())
,   _usage(resource->Usage()) {
    Assert(resource);
    Assert(_format);
}
//----------------------------------------------------------------------------
FDeviceAPIDependantTexture::~FDeviceAPIDependantTexture() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
