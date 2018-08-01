#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/IDeviceAPIShaderCompiler.h"

#include "Core/IO/VirtualFileSystem_fwd.h"
#include "Core/Memory/UniquePtr.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FShaderCompilerEncapsulator
:   private Meta::FThreadResource
,   private IDeviceAPIShaderCompiler {
public:
    FShaderCompilerEncapsulator();
    virtual ~FShaderCompilerEncapsulator();

    EDeviceAPI API() const { return _api; }
    IDeviceAPIShaderCompiler* Compiler() const;

    void Create(EDeviceAPI api);
    void Destroy();

private: // IDeviceAPIShaderCompiler

    virtual FShaderCompiled *CompileShaderSource(
        const FShaderSource *source,
        const FVertexDeclaration *vertexDeclaration,
        EShaderProgramType programType,
        EShaderProfileType profileType,
        EShaderCompilerFlags flags,
        const char *entryPoint ) override;

    virtual void PreprocessShaderSource(
        RAWSTORAGE(Shader, char)& output,
        const FShaderSource *source,
        const FVertexDeclaration *vertexDeclaration) override;

private:
    EDeviceAPI _api;
    TUniquePtr< IDeviceAPIShaderCompiler > _deviceAPIShaderCompiler;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ShaderCompilerFlagsToCStr(char *cstr, size_t capacity, EShaderCompilerFlags flags);
//----------------------------------------------------------------------------
PShaderCompiled CompileShaderSource(
    const FShaderCompilerEncapsulator* encapsulator,
    const FFilename& filename,
    const FVertexDeclaration *vertexDeclaration,
    EShaderProgramType programType,
    EShaderProfileType profileType,
    EShaderCompilerFlags flags,
    const char *entryPoint,
    const TMemoryView<const TPair<FString, FString>>& defines );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
