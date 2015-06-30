#include "stdafx.h"

#include "Texture.h"

#include "Device/IDeviceAPIEncapsulator.h"
#include "Device/DeviceResourceBuffer.h"
#include "SurfaceFormat.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Texture::Texture(DeviceResourceType textureType, const SurfaceFormat *format, BufferMode mode, BufferUsage usage)
:   DeviceResource(textureType)
,   _format(format)
,   _usageAndMode(0) {
    Assert(format);
    Assert(format->SupportTexture());
    Assert( DeviceResourceType::Texture2D == textureType ||
            DeviceResourceType::TextureCube == textureType );

#ifdef WITH_CORE_ASSERT_RELEASE
    switch (usage)
    {
    case BufferUsage::Default:
        AssertRelease(  BufferMode::None == mode ||
                        BufferMode::Write == mode);
        break;
    case BufferUsage::Dynamic:
        AssertRelease(  BufferMode::Write == mode);
        break;
    case BufferUsage::Immutable:
        AssertRelease(  BufferMode::None == mode);
        break;
    case BufferUsage::Staging:
        AssertRelease(  BufferMode::None != mode);
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
Texture::~Texture() {}
//----------------------------------------------------------------------------
bool Texture::Available() const {
    return nullptr != TextureEntity();
}
//----------------------------------------------------------------------------
DeviceAPIDependantEntity *Texture::TerminalEntity() const {
    return TextureEntity();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantTexture::DeviceAPIDependantTexture(IDeviceAPIEncapsulator *device, const Texture *resource)
:   TypedDeviceAPIDependantEntity<Texture>(device->APIEncapsulator(), resource)
,   _format(resource->Format())
,   _mode(resource->Mode()) 
,   _usage(resource->Usage()) {
    Assert(resource);
    Assert(_format);
}
//----------------------------------------------------------------------------
DeviceAPIDependantTexture::~DeviceAPIDependantTexture() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
