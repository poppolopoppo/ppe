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
enum class EShaderCompilerFlags {
    None        = 0,
    Debug       = 1 << 0,
    Optimize    = 1 << 1,
    NoOptimize  = 1 << 2,
    Pedantic    = 1 << 3,
    WError      = 1 << 4,
    IEEEStrict  = 1 << 5,
    StripDebug  = 1 << 6,

    Default = Optimize|Pedantic|WError,
    DefaultForDebug = Debug|NoOptimize|Pedantic|WError,
    DefaultForFinal = Default|StripDebug,
};
ENUM_FLAGS(EShaderCompilerFlags);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IDeviceAPIShaderCompiler {
public:
    virtual ~IDeviceAPIShaderCompiler() {}

    virtual FShaderCompiled* CompileShaderSource(
        const FShaderSource* source,
        const FVertexDeclaration* vertexDeclaration,
        EShaderProgramType programType,
        EShaderProfileType profileType,
        EShaderCompilerFlags flags,
        const char* entryPoint ) = 0;

    virtual void PreprocessShaderSource(
        RAWSTORAGE(Shader, char)& output,
        const FShaderSource* source,
        const FVertexDeclaration* vertexDeclaration) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
