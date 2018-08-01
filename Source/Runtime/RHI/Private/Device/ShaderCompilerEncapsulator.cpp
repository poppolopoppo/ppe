#include "stdafx.h"

#include "ShaderCompilerEncapsulator.h"

#include "DeviceEncapsulatorException.h"
#include "Shader/ShaderCompiled.h"
#include "Shader/ShaderProgram.h"
#include "Shader/ShaderSource.h"

#include "Diagnostic/CrtDebug.h"
#include "Diagnostic/DialogBox.h"
#include "Diagnostic/Logger.h"
#include "IO/FileSystem.h"
#include "Misc/TargetPlatform.h"

#include "DirectX11/DX11ShaderCompiler.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FShaderCompilerEncapsulator::FShaderCompilerEncapsulator() : _api(EDeviceAPI::Unknown) {}
//----------------------------------------------------------------------------
FShaderCompilerEncapsulator::~FShaderCompilerEncapsulator() {
    Assert(EDeviceAPI::Unknown == _api);
    Assert(nullptr == _deviceAPIShaderCompiler);
}
//----------------------------------------------------------------------------
IDeviceAPIShaderCompiler* FShaderCompilerEncapsulator::Compiler() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(EDeviceAPI::Unknown != _api);
    Assert(nullptr != _deviceAPIShaderCompiler);

    return remove_const(this);
}
//----------------------------------------------------------------------------
void FShaderCompilerEncapsulator::Create(EDeviceAPI api) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(EDeviceAPI::Unknown == _api);
    Assert(nullptr == _deviceAPIShaderCompiler);

    _api = api;

    switch (_api)
    {
    case Core::Graphics::EDeviceAPI::DirectX11:
        _deviceAPIShaderCompiler.reset(new FDX11ShaderCompiler());
        break;
    case Core::Graphics::EDeviceAPI::OpenGL4:
        AssertNotImplemented();
        break;
    case Core::Graphics::EDeviceAPI::Unknown:
        AssertNotReached();
        break;
    default:
        AssertNotImplemented();
        break;
    };
}
//----------------------------------------------------------------------------
void FShaderCompilerEncapsulator::Destroy() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(EDeviceAPI::Unknown != _api);
    Assert(nullptr != _deviceAPIShaderCompiler);

    _api = EDeviceAPI::Unknown;
    _deviceAPIShaderCompiler.reset(nullptr);
}
//----------------------------------------------------------------------------
FShaderCompiled* FShaderCompilerEncapsulator::CompileShaderSource(
    const FShaderSource *source,
    const FVertexDeclaration* vertexDeclaration,
    EShaderProgramType programType,
    EShaderProfileType profileType,
    EShaderCompilerFlags flags,
    const char *entryPoint ) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(source);
    Assert(vertexDeclaration);
    Assert(entryPoint);

    return _deviceAPIShaderCompiler->CompileShaderSource(source, vertexDeclaration, programType, profileType, flags, entryPoint);
}
//----------------------------------------------------------------------------
void FShaderCompilerEncapsulator::PreprocessShaderSource(
    RAWSTORAGE(Shader, char)& output,
    const FShaderSource *source,
    const FVertexDeclaration *vertexDeclaration ) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(source);
    Assert(vertexDeclaration);

    return _deviceAPIShaderCompiler->PreprocessShaderSource(output, source, vertexDeclaration);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ShaderCompilerFlagsToCStr(char *cstr, size_t capacity, EShaderCompilerFlags flags) {
    FOCStrStream oss(cstr, capacity);
    if (flags ^ EShaderCompilerFlags::Debug)
        oss << "Debug,";
    if (flags ^ EShaderCompilerFlags::Optimize)
        oss << "Optimize,";
    if (flags ^ EShaderCompilerFlags::NoOptimize)
        oss << "NoOptimize,";
    if (flags ^ EShaderCompilerFlags::Pedantic)
        oss << "Pedantic,";
    if (flags ^ EShaderCompilerFlags::WError)
        oss << "WError,";
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static bool TryCompileShaderSource_(
    PShaderCompiled& compiled,
    PShaderSource& source,
    FWString& whatIfFailed,
    IDeviceAPIShaderCompiler *compiler,
    const FFilename& filename,
    const FVertexDeclaration *vertexDeclaration,
    EShaderProgramType programType,
    EShaderProfileType profileType,
    EShaderCompilerFlags flags,
    const char *entryPoint,
    const TMemoryView<const TPair<FString, FString>>& defines ) {

    try {
        source = FShaderSource::LoadFromFileIFP(filename, defines);
        if (nullptr == source)
            PPE_THROW_IT(FShaderCompilerException("failed to read source file", compiler, nullptr));

        compiled = compiler->CompileShaderSource(
            source.get(),
            vertexDeclaration,
            programType,
            profileType,
            flags,
            entryPoint );
    }
    catch (const FShaderCompilerException& e) {
        whatIfFailed = ToWString(e.what());
        return false;
    }
    return true;
}
//----------------------------------------------------------------------------
PShaderCompiled CompileShaderSource(
    const FShaderCompilerEncapsulator* encapsulator,
    const FFilename& filename,
    const FVertexDeclaration *vertexDeclaration,
    EShaderProgramType programType,
    EShaderProfileType profileType,
    EShaderCompilerFlags flags,
    const char *entryPoint,
    const TMemoryView<const TPair<FString, FString>>& defines ) {
    Assert(encapsulator);
    Assert(false == filename.empty());

#ifdef USE_DEBUG_LOGGER
    LOG(Info, L"[Shader] Compiling {0} program with profile {1} from '{2}':{3}()",
        ShaderProgramTypeToCStr(programType),
        ShaderProfileTypeToCStr(profileType),
        filename, entryPoint );

    for (const TPair<FString, FString>& define : defines)
        LOG(Info, L"[Shader] #define {0} {1}", define.first, define.second);
#endif

    IDeviceAPIShaderCompiler* const deviceAPIShaderCompiler = encapsulator->Compiler();

    PShaderSource source;
    PShaderCompiled compiled;
    FWString errorMessage;
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

        const Dialog::EResult dialog = Dialog::AbortRetryIgnore(
            MakeStringView(errorMessage),
            L"Shader compilation error",
            Dialog::Icon::Exclamation );

        switch (dialog)
        {
        case Dialog::EResult::Ignore:
            {
                RAWSTORAGE(Shader, char) preprocess;
                deviceAPIShaderCompiler->PreprocessShaderSource(preprocess, source.get(), vertexDeclaration);
#ifndef FINAL_RELEASE
                FPlatform::DebugBreak();
#endif
            }
            //break;
        case Dialog::EResult::Retry:
            errorMessage.clear();
            if (source)
                RemoveRef_AssertReachZero(source);
            if (compiled)
                RemoveRef_AssertReachZero(compiled);
            continue;

        default:
            PPE_THROW_IT(FShaderCompilerException("abort compilation", deviceAPIShaderCompiler, source.get()));
        }
    }

    Assert(compiled);
    return compiled;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
