#pragma once

#include "Core.Graphics/Graphics.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ShaderProfileType {
    ShaderModel5 = 0,
    ShaderModel4_1,
    ShaderModel4,
    ShaderModel3,
};
//----------------------------------------------------------------------------
StringSlice ShaderProfileTypeToCStr(ShaderProfileType profile);
//----------------------------------------------------------------------------
enum class ShaderProgramType {
    Vertex = 0,
    Hull,
    Domain,
    Pixel,
    Geometry,
    Compute,

    __Count,
};
//----------------------------------------------------------------------------
MemoryView<const ShaderProgramType> EachShaderProgramType();
StringSlice ShaderProgramTypeToCStr(ShaderProgramType program);
StringSlice ShaderProgramTypeToEntryPoint(ShaderProgramType program);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
