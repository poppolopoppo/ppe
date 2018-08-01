#pragma once

#include "Graphics.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EShaderProfileType {
    ShaderModel5 = 0,
    ShaderModel4_1,
    ShaderModel4,
    ShaderModel3,
};
//----------------------------------------------------------------------------
FStringView ShaderProfileTypeToCStr(EShaderProfileType profile);
//----------------------------------------------------------------------------
enum class EShaderProgramType {
    Vertex = 0,
    Hull,
    Domain,
    Pixel,
    Geometry,
    Compute,

    __Count,
};
//----------------------------------------------------------------------------
TMemoryView<const EShaderProgramType> EachShaderProgramType();
FStringView ShaderProgramTypeToCStr(EShaderProgramType program);
FStringView ShaderProgramTypeToEntryPoint(EShaderProgramType program);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
