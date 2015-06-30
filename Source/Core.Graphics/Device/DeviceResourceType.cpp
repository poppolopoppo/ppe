#include "stdafx.h"

#include "DeviceResourceType.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const char *ResourceTypeToCStr(DeviceResourceType type) {
    switch (type)
    {
    case Core::Graphics::DeviceResourceType::Constants:
        return "Constants";
    case Core::Graphics::DeviceResourceType::Indices:
        return "Indices";
    case Core::Graphics::DeviceResourceType::BlendState:
        return "BlendState";
    case Core::Graphics::DeviceResourceType::DepthStencilState:
        return "DepthStencilState";
    case Core::Graphics::DeviceResourceType::RasterizerState:
        return "RasterizerState";
    case Core::Graphics::DeviceResourceType::RenderTarget:
        return "RenderTarget";
    case Core::Graphics::DeviceResourceType::SamplerState:
        return "SamplerState";
    case Core::Graphics::DeviceResourceType::ShaderEffect:
        return "ShaderEffect";
    case Core::Graphics::DeviceResourceType::ShaderProgram:
        return "ShaderProgram";
    case Core::Graphics::DeviceResourceType::Texture2D:
        return "Texture2D";
    case Core::Graphics::DeviceResourceType::TextureCube:
        return "TextureCube";
    case Core::Graphics::DeviceResourceType::VertexDeclaration:
        return "VertexDeclaration";
    case Core::Graphics::DeviceResourceType::Vertices:
        return "Vertices";
    }

    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
