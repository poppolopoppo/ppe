#include "stdafx.h"

#include "ShaderCompilerEncapsulator.h"

#include "DeviceEncapsulatorException.h"
#include "Shader/ShaderCompiled.h"
#include "Shader/ShaderProgram.h"
#include "Shader/ShaderSource.h"

#include "Core/Diagnostic/CrtDebug.h"
#include "Core/Diagnostic/DialogBox.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FileSystem.h"

#include "DirectX11/DX11ShaderCompiler.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ShaderCompilerEncapsulator::ShaderCompilerEncapsulator() : _api(DeviceAPI::Unknown) {}
//----------------------------------------------------------------------------
ShaderCompilerEncapsulator::~ShaderCompilerEncapsulator() {
    Assert(DeviceAPI::Unknown == _api);
    Assert(nullptr == _deviceAPIShaderCompiler);
}
//----------------------------------------------------------------------------
IDeviceAPIShaderCompiler* ShaderCompilerEncapsulator::Compiler() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(DeviceAPI::Unknown != _api);
    Assert(nullptr != _deviceAPIShaderCompiler);

    return remove_const(this);
}
//----------------------------------------------------------------------------
void ShaderCompilerEncapsulator::Create(DeviceAPI api) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(DeviceAPI::Unknown == _api);
    Assert(nullptr == _deviceAPIShaderCompiler);

    _api = api;

    switch (_api)
    {
    case Core::Graphics::DeviceAPI::DirectX11:
        _deviceAPIShaderCompiler.reset(new DX11ShaderCompiler());
        break;
    case Core::Graphics::DeviceAPI::OpenGL4:
        AssertNotImplemented();
        break;
    case Core::Graphics::DeviceAPI::Unknown:
        AssertNotReached();
        break;
    default:
        AssertNotImplemented();
        break;
    };
}
//----------------------------------------------------------------------------
void ShaderCompilerEncapsulator::Destroy() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(DeviceAPI::Unknown != _api);
    Assert(nullptr != _deviceAPIShaderCompiler);

    _api = DeviceAPI::Unknown;
    _deviceAPIShaderCompiler.reset(nullptr);
}
//----------------------------------------------------------------------------
ShaderCompiled* ShaderCompilerEncapsulator::CompileShaderSource(
    const ShaderSource *source,
    const VertexDeclaration* vertexDeclaration,
    ShaderProgramType programType,
    ShaderProfileType profileType,
    ShaderCompilerFlags flags,
    const char *entryPoint ) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(source);
    Assert(vertexDeclaration);
    Assert(entryPoint);

    return _deviceAPIShaderCompiler->CompileShaderSource(source, vertexDeclaration, programType, profileType, flags, entryPoint);
}
//----------------------------------------------------------------------------
void ShaderCompilerEncapsulator::PreprocessShaderSource(
    RAWSTORAGE(Shader, char)& output,
    const ShaderSource *source,
    const VertexDeclaration *vertexDeclaration ) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(source);
    Assert(vertexDeclaration);

    return _deviceAPIShaderCompiler->PreprocessShaderSource(output, source, vertexDeclaration);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ShaderCompilerFlagsToCStr(char *cstr, size_t capacity, ShaderCompilerFlags flags) {
    OCStrStream oss(cstr, capacity);
    if (Meta::HasFlag(flags, ShaderCompilerFlags::Debug))
        oss << "Debug,";
    if (Meta::HasFlag(flags, ShaderCompilerFlags::Optimize))
        oss << "Optimize,";
    if (Meta::HasFlag(flags, ShaderCompilerFlags::NoOptimize))
        oss << "NoOptimize,";
    if (Meta::HasFlag(flags, ShaderCompilerFlags::Pedantic))
        oss << "Pedantic,";
    if (Meta::HasFlag(flags, ShaderCompilerFlags::WError))
        oss << "WError,";
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static bool TryCompileShaderSource_(
    PShaderCompiled& compiled,
    PShaderSource& source,
    WString& whatIfFailed,
    IDeviceAPIShaderCompiler *compiler,
    const Filename& filename,
    const VertexDeclaration *vertexDeclaration,
    ShaderProgramType programType,
    ShaderProfileType profileType,
    ShaderCompilerFlags flags,
    const char *entryPoint,
    const MemoryView<const Pair<String, String>>& defines ) {

    try {
        source = ShaderSource::LoadFromFileIFP(filename, defines);
        if (nullptr == source)
            throw ShaderCompilerException("failed to read source file", compiler, nullptr);

        compiled = compiler->CompileShaderSource(
            source.get(),
            vertexDeclaration,
            programType,
            profileType,
            flags,
            entryPoint );
    }
    catch (const ShaderCompilerException& e) {
        whatIfFailed = ToWString(e.what());
        return false;
    }
    return true;
}
//----------------------------------------------------------------------------
PShaderCompiled CompileShaderSource(
    const ShaderCompilerEncapsulator* encapsulator,
    const Filename& filename,
    const VertexDeclaration *vertexDeclaration,
    ShaderProgramType programType,
    ShaderProfileType profileType,
    ShaderCompilerFlags flags,
    const char *entryPoint,
    const MemoryView<const Pair<String, String>>& defines ) {
    Assert(encapsulator);
    Assert(false == filename.empty());

#ifdef USE_DEBUG_LOGGER
    LOG(Info, L"[Shader] Compiling {0} program with profile {1} from '{2}':{3}()",
        ShaderProgramTypeToCStr(programType),
        ShaderProfileTypeToCStr(profileType),
        filename, entryPoint );

    for (const Pair<String, String>& define : defines)
        LOG(Info, L"[Shader] #define {0} {1}", define.first, define.second);
#endif

    IDeviceAPIShaderCompiler* const deviceAPIShaderCompiler = encapsulator->Compiler();

    PShaderSource source;
    PShaderCompiled compiled;
    WString errorMessage;
    while (!TryCompileShaderSource_(
                compiled, source, errorMessage,
                deviceAPIShaderCompiler,
                filename,
                vertexDeclaration,
                programType,
                profileType,
                flags,
                entryPoint,
                defines )) {

        const Dialog::Result dialog = Dialog::AbortRetryIgnore(
            MakeStringSlice(errorMessage),
            L"Shader compilation error",
            Dialog::Icon::Exclamation );

        switch (dialog)
        {
        case Dialog::Result::Ignore:
            {
                RAWSTORAGE(Shader, char) preprocess;
                deviceAPIShaderCompiler->PreprocessShaderSource(preprocess, source.get(), vertexDeclaration);
                BREAKPOINT();
            }
            //break;
        case Dialog::Result::Retry:
            errorMessage.clear();
            if (source)
                RemoveRef_AssertReachZero(source);
            if (compiled)
                RemoveRef_AssertReachZero(compiled);
            continue;

        default:
            throw ShaderCompilerException("abort compilation", deviceAPIShaderCompiler, source.get());
        }
    }

    Assert(compiled);
    return compiled;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
