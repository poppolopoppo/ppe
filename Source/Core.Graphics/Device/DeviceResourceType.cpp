#include "stdafx.h"

#include "DeviceResourceType.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringSlice ResourceTypeToCStr(DeviceResourceType type) {
    switch (type)
    {
    case Core::Graphics::DeviceResourceType::Constants:
        return MakeStringSlice("Constants");
    case Core::Graphics::DeviceResourceType::Indices:
        return MakeStringSlice("Indices");
    case Core::Graphics::DeviceResourceType::BlendState:
        return MakeStringSlice("BlendState");
    case Core::Graphics::DeviceResourceType::DepthStencilState:
        return MakeStringSlice("DepthStencilState");
    case Core::Graphics::DeviceResourceType::RasterizerState:
        return MakeStringSlice("RasterizerState");
    case Core::Graphics::DeviceResourceType::RenderTarget:
        return MakeStringSlice("RenderTarget");
    case Core::Graphics::DeviceResourceType::SamplerState:
        return MakeStringSlice("SamplerState");
    case Core::Graphics::DeviceResourceType::ShaderEffect:
        return MakeStringSlice("ShaderEffect");
    case Core::Graphics::DeviceResourceType::ShaderProgram:
        return MakeStringSlice("ShaderProgram");
    case Core::Graphics::DeviceResourceType::Texture2D:
        return MakeStringSlice("Texture2D");
    case Core::Graphics::DeviceResourceType::TextureCube:
        return MakeStringSlice("TextureCube");
    case Core::Graphics::DeviceResourceType::VertexDeclaration:
        return MakeStringSlice("VertexDeclaration");
    case Core::Graphics::DeviceResourceType::Vertices:
        return MakeStringSlice("Vertices");
    default:
        AssertNotImplemented();
    }
    return StringSlice();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
