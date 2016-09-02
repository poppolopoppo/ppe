#include "stdafx.h"

#include "DeviceResourceType.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringView ResourceTypeToCStr(DeviceResourceType type) {
    switch (type)
    {
    case Core::Graphics::DeviceResourceType::Constants:
        return MakeStringView("Constants");
    case Core::Graphics::DeviceResourceType::Indices:
        return MakeStringView("Indices");
    case Core::Graphics::DeviceResourceType::BlendState:
        return MakeStringView("BlendState");
    case Core::Graphics::DeviceResourceType::DepthStencilState:
        return MakeStringView("DepthStencilState");
    case Core::Graphics::DeviceResourceType::RasterizerState:
        return MakeStringView("RasterizerState");
    case Core::Graphics::DeviceResourceType::RenderTarget:
        return MakeStringView("RenderTarget");
    case Core::Graphics::DeviceResourceType::SamplerState:
        return MakeStringView("SamplerState");
    case Core::Graphics::DeviceResourceType::ShaderEffect:
        return MakeStringView("ShaderEffect");
    case Core::Graphics::DeviceResourceType::ShaderProgram:
        return MakeStringView("ShaderProgram");
    case Core::Graphics::DeviceResourceType::Texture2D:
        return MakeStringView("Texture2D");
    case Core::Graphics::DeviceResourceType::TextureCube:
        return MakeStringView("TextureCube");
    case Core::Graphics::DeviceResourceType::VertexDeclaration:
        return MakeStringView("VertexDeclaration");
    case Core::Graphics::DeviceResourceType::Vertices:
        return MakeStringView("Vertices");
    default:
        AssertNotImplemented();
    }
    return StringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
