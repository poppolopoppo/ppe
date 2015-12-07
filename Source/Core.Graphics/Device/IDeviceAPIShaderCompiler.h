#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPI_fwd.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/RawStorage.h"

namespace Core {
namespace Graphics {
FWD_REFPTR(ShaderSource);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ShaderCompilerFlags {
    None        = 0,
    Debug       = 1 << 0,
    Optimize    = 1 << 1,
    NoOptimize  = 1 << 2,
    Pedantic    = 1 << 3,
    WError      = 1 << 4,

    Default = Optimize|Pedantic|WError,
    DefaultForDebug = Debug|NoOptimize|Pedantic|WError,
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IDeviceAPIShaderCompiler {
public:
    virtual ~IDeviceAPIShaderCompiler() {}

    virtual ShaderCompiled* CompileShaderSource(
        const ShaderSource* source,
        const VertexDeclaration* vertexDeclaration,
        ShaderProgramType programType,
        ShaderProfileType profileType,
        ShaderCompilerFlags flags,
        const char* entryPoint ) = 0;

    virtual void PreprocessShaderSource(
        RAWSTORAGE(Shader, char)& output,
        const ShaderSource* source,
        const VertexDeclaration* vertexDeclaration) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
