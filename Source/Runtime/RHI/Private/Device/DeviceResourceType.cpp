#include "stdafx.h"

#include "DeviceResourceType.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView ResourceTypeToCStr(EDeviceResourceType type) {
    switch (type)
    {
    case PPE::Graphics::EDeviceResourceType::Constants:
        return MakeStringView("Constants");
    case PPE::Graphics::EDeviceResourceType::Indices:
        return MakeStringView("Indices");
    case PPE::Graphics::EDeviceResourceType::FBlendState:
        return MakeStringView("FBlendState");
    case PPE::Graphics::EDeviceResourceType::FDepthStencilState:
        return MakeStringView("FDepthStencilState");
    case PPE::Graphics::EDeviceResourceType::FRasterizerState:
        return MakeStringView("FRasterizerState");
    case PPE::Graphics::EDeviceResourceType::FRenderTarget:
        return MakeStringView("FRenderTarget");
    case PPE::Graphics::EDeviceResourceType::FSamplerState:
        return MakeStringView("FSamplerState");
    case PPE::Graphics::EDeviceResourceType::FShaderEffect:
        return MakeStringView("FShaderEffect");
    case PPE::Graphics::EDeviceResourceType::FShaderProgram:
        return MakeStringView("FShaderProgram");
    case PPE::Graphics::EDeviceResourceType::FTexture2D:
        return MakeStringView("FTexture2D");
    case PPE::Graphics::EDeviceResourceType::FTextureCube:
        return MakeStringView("FTextureCube");
    case PPE::Graphics::EDeviceResourceType::FVertexDeclaration:
        return MakeStringView("FVertexDeclaration");
    case PPE::Graphics::EDeviceResourceType::Vertices:
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
} //!namespace PPE
