#pragma once

#include "Core.Graphics/Graphics.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class DeviceResourceType {
    Constants = 0,
    Indices,
    BlendState,
    DepthStencilState,
    RasterizerState,
    RenderTarget,
    SamplerState,
    ShaderEffect,
    ShaderProgram,
    Texture2D,
    TextureCube,
    VertexDeclaration,
    Vertices,

    _Count
};
//----------------------------------------------------------------------------
StringSlice ResourceTypeToCStr(DeviceResourceType type);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
