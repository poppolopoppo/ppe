#pragma once

#include "Core.Graphics/Graphics.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EDeviceResourceType {
    Constants = 0,
    Indices,
    FBlendState,
    FDepthStencilState,
    FRasterizerState,
    FRenderTarget,
    FSamplerState,
    FShaderEffect,
    FShaderProgram,
    FTexture2D,
    FTextureCube,
    FVertexDeclaration,
    Vertices,

    _Count
};
//----------------------------------------------------------------------------
FStringView ResourceTypeToCStr(EDeviceResourceType type);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
