#include "stdafx.h"

#include "DX11SurfaceFormat.h"

#include "Device/Texture/SurfaceFormat.h"

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DXGI_FORMAT SurfaceFormatTypeToDXGIFormat(SurfaceFormatType value) {
    switch (value)
    {
    case Core::Graphics::SurfaceFormatType::UNKNOWN:
        return DXGI_FORMAT_UNKNOWN;
    case Core::Graphics::SurfaceFormatType::A8:
        return DXGI_FORMAT_A8_UNORM;
    case Core::Graphics::SurfaceFormatType::D16:
        return DXGI_FORMAT_D16_UNORM;
    case Core::Graphics::SurfaceFormatType::D24S8:
        return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case Core::Graphics::SurfaceFormatType::D32:
        return DXGI_FORMAT_D32_FLOAT;
    case Core::Graphics::SurfaceFormatType::DXN0:
        return DXGI_FORMAT_BC5_UNORM;
    case Core::Graphics::SurfaceFormatType::DXT1:
        return DXGI_FORMAT_BC1_UNORM;
    case Core::Graphics::SurfaceFormatType::DXT1_SRGB:
        return DXGI_FORMAT_BC1_UNORM_SRGB;
    case Core::Graphics::SurfaceFormatType::DXT3:
        return DXGI_FORMAT_BC2_UNORM;
    case Core::Graphics::SurfaceFormatType::DXT3_SRGB:
        return DXGI_FORMAT_BC2_UNORM_SRGB;
    case Core::Graphics::SurfaceFormatType::DXT5:
        return DXGI_FORMAT_BC3_UNORM;
    case Core::Graphics::SurfaceFormatType::DXT5_SRGB:
        return DXGI_FORMAT_BC3_UNORM_SRGB;
    case Core::Graphics::SurfaceFormatType::R5G5B5A1:
        return DXGI_FORMAT_B5G5R5A1_UNORM;
    case Core::Graphics::SurfaceFormatType::R5G6B5:
        return DXGI_FORMAT_B5G6R5_UNORM;
    case Core::Graphics::SurfaceFormatType::R8:
        return DXGI_FORMAT_R8_UNORM;
    case Core::Graphics::SurfaceFormatType::R8G8:
        return DXGI_FORMAT_R8G8_UNORM;
    case Core::Graphics::SurfaceFormatType::R8G8B8A8:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case Core::Graphics::SurfaceFormatType::R8G8B8A8_SRGB:
        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case Core::Graphics::SurfaceFormatType::B8G8R8A8:
        return DXGI_FORMAT_B8G8R8A8_UNORM;
    case Core::Graphics::SurfaceFormatType::B8G8R8A8_SRGB:
        return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    case Core::Graphics::SurfaceFormatType::R10G10B10A2:
        return DXGI_FORMAT_R10G10B10A2_UNORM;
    case Core::Graphics::SurfaceFormatType::R11G11B10:
        return DXGI_FORMAT_R11G11B10_FLOAT;
    case Core::Graphics::SurfaceFormatType::R16:
        return DXGI_FORMAT_R16_UNORM;
    case Core::Graphics::SurfaceFormatType::R16G16:
        return DXGI_FORMAT_R16G16_UNORM;
    case Core::Graphics::SurfaceFormatType::R16G16B16A16:
        return DXGI_FORMAT_R16G16B16A16_UNORM;
    case Core::Graphics::SurfaceFormatType::R16G16B16A16_F:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case Core::Graphics::SurfaceFormatType::R16G16_F:
        return DXGI_FORMAT_R16G16_FLOAT;
    case Core::Graphics::SurfaceFormatType::R16_F:
        return DXGI_FORMAT_R16_FLOAT;
    case Core::Graphics::SurfaceFormatType::R32:
        return DXGI_FORMAT_R32_UINT;
    case Core::Graphics::SurfaceFormatType::R32G32:
        return DXGI_FORMAT_R32G32_UINT;
    case Core::Graphics::SurfaceFormatType::R32G32B32A32:
        return DXGI_FORMAT_R32G32B32A32_UINT;
    case Core::Graphics::SurfaceFormatType::R32G32B32A32_F:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case Core::Graphics::SurfaceFormatType::R32G32_F:
        return DXGI_FORMAT_R32G32_FLOAT;
    case Core::Graphics::SurfaceFormatType::R32_F:
        return DXGI_FORMAT_R32_FLOAT;
    default:
        AssertNotImplemented();
    }
    return DXGI_FORMAT_UNKNOWN;
}
//----------------------------------------------------------------------------
SurfaceFormatType DXGIFormatToSurfaceFormatType(DXGI_FORMAT value) {
    switch (value)
    {
    case DXGI_FORMAT_UNKNOWN: return Core::Graphics::SurfaceFormatType::UNKNOWN;
    case DXGI_FORMAT_A8_UNORM: return Core::Graphics::SurfaceFormatType::A8;
    case DXGI_FORMAT_D16_UNORM: return Core::Graphics::SurfaceFormatType::D16;
    case DXGI_FORMAT_D24_UNORM_S8_UINT: return Core::Graphics::SurfaceFormatType::D24S8;
    case DXGI_FORMAT_D32_FLOAT: return Core::Graphics::SurfaceFormatType::D32;
    case DXGI_FORMAT_BC5_UNORM: return Core::Graphics::SurfaceFormatType::DXN0;
    case DXGI_FORMAT_BC1_UNORM: return Core::Graphics::SurfaceFormatType::DXT1;
    case DXGI_FORMAT_BC1_UNORM_SRGB: return Core::Graphics::SurfaceFormatType::DXT1_SRGB;
    case DXGI_FORMAT_BC2_UNORM: return Core::Graphics::SurfaceFormatType::DXT3;
    case DXGI_FORMAT_BC2_UNORM_SRGB: return Core::Graphics::SurfaceFormatType::DXT3_SRGB;
    case DXGI_FORMAT_BC3_UNORM: return Core::Graphics::SurfaceFormatType::DXT5;
    case DXGI_FORMAT_BC3_UNORM_SRGB: return Core::Graphics::SurfaceFormatType::DXT5_SRGB;
    case DXGI_FORMAT_B5G5R5A1_UNORM: return Core::Graphics::SurfaceFormatType::R5G5B5A1;
    case DXGI_FORMAT_B5G6R5_UNORM: return Core::Graphics::SurfaceFormatType::R5G6B5;
    case DXGI_FORMAT_R8_UNORM: return Core::Graphics::SurfaceFormatType::R8;
    case DXGI_FORMAT_R8G8_UNORM: return Core::Graphics::SurfaceFormatType::R8G8;
    case DXGI_FORMAT_R8G8B8A8_UNORM: return Core::Graphics::SurfaceFormatType::R8G8B8A8;
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return Core::Graphics::SurfaceFormatType::R8G8B8A8_SRGB;
    case DXGI_FORMAT_B8G8R8A8_UNORM: return Core::Graphics::SurfaceFormatType::B8G8R8A8;
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return Core::Graphics::SurfaceFormatType::B8G8R8A8_SRGB;
    case DXGI_FORMAT_R10G10B10A2_UNORM: return Core::Graphics::SurfaceFormatType::R10G10B10A2;
    case DXGI_FORMAT_R11G11B10_FLOAT: return Core::Graphics::SurfaceFormatType::R11G11B10;
    case DXGI_FORMAT_R16_UNORM: return Core::Graphics::SurfaceFormatType::R16;
    case DXGI_FORMAT_R16G16_UNORM: return Core::Graphics::SurfaceFormatType::R16G16;
    case DXGI_FORMAT_R16G16B16A16_UNORM: return Core::Graphics::SurfaceFormatType::R16G16B16A16;
    case DXGI_FORMAT_R16G16B16A16_FLOAT: return Core::Graphics::SurfaceFormatType::R16G16B16A16_F;
    case DXGI_FORMAT_R16G16_FLOAT: return Core::Graphics::SurfaceFormatType::R16G16_F;
    case DXGI_FORMAT_R16_FLOAT: return Core::Graphics::SurfaceFormatType::R16_F;
    case DXGI_FORMAT_R32_UINT: return Core::Graphics::SurfaceFormatType::R32;
    case DXGI_FORMAT_R32G32_UINT: return Core::Graphics::SurfaceFormatType::R32G32;
    case DXGI_FORMAT_R32G32B32A32_UINT: return Core::Graphics::SurfaceFormatType::R32G32B32A32;
    case DXGI_FORMAT_R32G32B32A32_FLOAT: return Core::Graphics::SurfaceFormatType::R32G32B32A32_F;
    case DXGI_FORMAT_R32G32_FLOAT: return Core::Graphics::SurfaceFormatType::R32G32_F;
    case DXGI_FORMAT_R32_FLOAT: return Core::Graphics::SurfaceFormatType::R32_F;
    default:
        AssertNotImplemented();
    }
    return SurfaceFormatType::UNKNOWN;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
