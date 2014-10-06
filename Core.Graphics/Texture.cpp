#include "stdafx.h"

#include "Texture.h"

#include "DeviceResourceBuffer.h"
#include "SurfaceFormat.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Texture::Texture(const SurfaceFormat *format, BufferMode mode, BufferUsage usage)
:   _format(format), _usageAndMode(0) {
    Assert(format);
    Assert(format->SupportTexture());

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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantTexture::DeviceAPIDependantTexture(IDeviceAPIEncapsulator *device, Texture *owner)
:   DeviceAPIDependantEntity(device)
,   _owner(owner) {
    Assert(owner);
}
//----------------------------------------------------------------------------
DeviceAPIDependantTexture::~DeviceAPIDependantTexture() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
