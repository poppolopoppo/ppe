#include "stdafx.h"

#include "DeviceResourceType.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView ResourceTypeToCStr(EDeviceResourceType type) {
    switch (type)
    {
    case Core::Graphics::EDeviceResourceType::Constants:
        return MakeStringView("Constants");
    case Core::Graphics::EDeviceResourceType::Indices:
        return MakeStringView("Indices");
    case Core::Graphics::EDeviceResourceType::FBlendState:
        return MakeStringView("FBlendState");
    case Core::Graphics::EDeviceResourceType::FDepthStencilState:
        return MakeStringView("FDepthStencilState");
    case Core::Graphics::EDeviceResourceType::FRasterizerState:
        return MakeStringView("FRasterizerState");
    case Core::Graphics::EDeviceResourceType::FRenderTarget:
        return MakeStringView("FRenderTarget");
    case Core::Graphics::EDeviceResourceType::FSamplerState:
        return MakeStringView("FSamplerState");
    case Core::Graphics::EDeviceResourceType::FShaderEffect:
        return MakeStringView("FShaderEffect");
    case Core::Graphics::EDeviceResourceType::FShaderProgram:
        return MakeStringView("FShaderProgram");
    case Core::Graphics::EDeviceResourceType::FTexture2D:
        return MakeStringView("FTexture2D");
    case Core::Graphics::EDeviceResourceType::FTextureCube:
        return MakeStringView("FTextureCube");
    case Core::Graphics::EDeviceResourceType::FVertexDeclaration:
        return MakeStringView("FVertexDeclaration");
    case Core::Graphics::EDeviceResourceType::Vertices:
        return MakeStringView("Vertices");
    default:
        AssertNotImplemented();
    }
    return FStringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
