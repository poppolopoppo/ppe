#include "stdafx.h"

#include "ShaderProgram.h"

#include "ConstantBufferLayout.h"
#include "Device/BindName.h"
#include "Device/DeviceEncapsulator.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/Geometry/VertexDeclaration.h"
#include "ShaderSource.h"

#include "Core/Container/RawStorage.h"
#include "Core/Diagnostic/CrtDebug.h"
#include "Core/Diagnostic/DialogBox.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/Stream.h"
#include "Core/IO/VirtualFileSystem.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ShaderProgram::ShaderProgram(ShaderProfileType profile, ShaderProgramType type)
:   _data(0) {
    bitprofile_type::InplaceSet(_data, static_cast<size_t>(profile));
    bitprogram_type::InplaceSet(_data, static_cast<size_t>(type));
}
//----------------------------------------------------------------------------
ShaderProgram::~ShaderProgram() {
    Assert(!_deviceAPIDependantProgram);
}
//----------------------------------------------------------------------------
void ShaderProgram::Create(
    IDeviceAPIShaderCompilerEncapsulator *compiler,
    const char *entryPoint,
    ShaderCompilerFlags flags,
    const ShaderSource *source,
    const VertexDeclaration *vertexDeclaration) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(compiler);
    Assert(!_deviceAPIDependantProgram);

    _deviceAPIDependantProgram = compiler->CreateShaderProgram(this, entryPoint, flags, source, vertexDeclaration);

    Assert(_deviceAPIDependantProgram);
}
//----------------------------------------------------------------------------
void ShaderProgram::Destroy(IDeviceAPIShaderCompilerEncapsulator *compiler) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(compiler);
    Assert(_deviceAPIDependantProgram);

    compiler->DestroyShaderProgram(this, _deviceAPIDependantProgram);

    Assert(!_deviceAPIDependantProgram);
}
//----------------------------------------------------------------------------
void ShaderProgram::Preprocess(
    IDeviceAPIShaderCompilerEncapsulator *compiler,
    RAWSTORAGE(Shader, char)& output,
    const ShaderSource *source,
    const VertexDeclaration *vertexDeclaration) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(compiler);

    compiler->PreprocessShaderProgram(output, this, source, vertexDeclaration);
}
//----------------------------------------------------------------------------
void ShaderProgram::Reflect(
    IDeviceAPIShaderCompilerEncapsulator *compiler,
    ASSOCIATIVE_VECTOR(Shader, BindName, PCConstantBufferLayout)& constants,
    VECTOR(Shader, BindName)& textures ) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(compiler);
    Assert(_deviceAPIDependantProgram);

    compiler->ReflectShaderProgram(constants, textures, this);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantShaderProgram::DeviceAPIDependantShaderProgram(
    IDeviceAPIShaderCompilerEncapsulator *compiler,
    Graphics::ShaderProgram *owner,
    const char *entryPoint,
    ShaderCompilerFlags flags,
    const ShaderSource *source,
    const VertexDeclaration *vertexDeclaration)
:   DeviceAPIDependantEntity(compiler->Encapsulator()->Device())
,   _owner(owner) {}
//----------------------------------------------------------------------------
DeviceAPIDependantShaderProgram::~DeviceAPIDependantShaderProgram() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static bool TryCompileShaderProgram_(
    PShaderSource& source,
    WString *wwhatIfFailed,
    IDeviceAPIShaderCompilerEncapsulator *compiler,
    ShaderProgram *program,
    const char *entryPoint,
    ShaderCompilerFlags flags,
    const Filename& filename,
    const VertexDeclaration *vertexDeclaration,
    const MemoryView<const Pair<String, String>>& defines ) {
    try {
        source = ShaderSource::LoadFromFile(filename, defines);
        program->Create(compiler, entryPoint, flags, source.get(), vertexDeclaration);
    }
    catch (const ShaderCompilerEncapsulatorException& e) {
        *wwhatIfFailed = ToWString(e.what());

        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
void CompileShaderProgram(
    IDeviceAPIShaderCompilerEncapsulator *compiler,
    ShaderProgram *program,
    const char *entryPoint,
    ShaderCompilerFlags flags,
    const Filename& filename,
    const VertexDeclaration *vertexDeclaration,
    const MemoryView<const Pair<String, String>>& defines ) {
    Assert(compiler);
    Assert(program);

#ifdef USE_LOGGER
    LOG(Information, L"[Shader] Compiling {0} program with profile {1} from '{2}':{3}()",
        ShaderProgramTypeToCStr(program->ProgramType()),
        ShaderProfileTypeToCStr(program->ProfileType()),
        filename, entryPoint );

    for (const Pair<String, String>& define : defines)
        LOG(Information, L"[Shader] #define {0} {1}", define.first, define.second);
#endif

    PShaderSource source;
    WString errorMessage;
    while (!TryCompileShaderProgram_(   source, &errorMessage,
                                        compiler, program, entryPoint, flags, filename, vertexDeclaration, defines )) {
        const DialogBox::Result dialog = DialogBox::AbortRetryIgnore(
            errorMessage.c_str(),
            L"Shader compilation error",
            DialogBox::Icon::Exclamation);

        switch (dialog)
        {
        case DialogBox::Result::Ignore:
            {
                RAWSTORAGE(Shader, char) preprocess;
                program->Preprocess(compiler, preprocess, source, vertexDeclaration);
                BREAKPOINT();
            }
        case DialogBox::Result::Retry:
            errorMessage.clear();
            RemoveRef_AssertReachZero(source);
            continue;

        default:
            throw ShaderCompilerEncapsulatorException("abort compilation", compiler, program, source);
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ShaderCompilerFlagsToCStr(char *cstr, size_t capacity, ShaderCompilerFlags flags) {
    BasicOCStrStream<char> oss(cstr, capacity);
    if (size_t(flags) & size_t(ShaderCompilerFlags::Debug))
        oss << "Debug";
    if (size_t(flags) & size_t(ShaderCompilerFlags::Optimize))
        oss << "Optimize";
    if (size_t(flags) & size_t(ShaderCompilerFlags::NoOptimize))
        oss << "NoOptimize";
    if (size_t(flags) & size_t(ShaderCompilerFlags::Pedantic))
        oss << "Pedantic";
    if (size_t(flags) & size_t(ShaderCompilerFlags::WError))
        oss << "WError";
}
//----------------------------------------------------------------------------
const char *ShaderProfileTypeToCStr(ShaderProfileType profile) {
    switch (profile)
    {
    case Core::Graphics::ShaderProfileType::ShaderModel5:
        return "ShaderModel5";
    case Core::Graphics::ShaderProfileType::ShaderModel4_1:
        return "ShaderModel4_1";
    case Core::Graphics::ShaderProfileType::ShaderModel4:
        return "ShaderModel4";
    case Core::Graphics::ShaderProfileType::ShaderModel3:
        return "ShaderModel3";
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
namespace {
    static const ShaderProgramType gShaderProgramTypes[size_t(ShaderProgramType::__Count)] = {
        ShaderProgramType::Vertex,
        ShaderProgramType::Hull,
        ShaderProgramType::Domain,
        ShaderProgramType::Pixel,
        ShaderProgramType::Geometry,
        ShaderProgramType::Compute,
    };
}
MemoryView<const ShaderProgramType> EachShaderProgramType() {
    return MakeView(gShaderProgramTypes);
}
//----------------------------------------------------------------------------
const char *ShaderProgramTypeToCStr(ShaderProgramType program) {
    switch (program)
    {
    case Core::Graphics::ShaderProgramType::Vertex:
        return "Vertex";
    case Core::Graphics::ShaderProgramType::Hull:
        return "Hull";
    case Core::Graphics::ShaderProgramType::Domain:
        return "Domain";
    case Core::Graphics::ShaderProgramType::Pixel:
        return "Pixel";
    case Core::Graphics::ShaderProgramType::Geometry:
        return "Geometry";
    case Core::Graphics::ShaderProgramType::Compute:
        return "Compute";
    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
const char *ShaderProgramTypeToEntryPoint(ShaderProgramType program) {
    switch (program)
    {
    case Core::Graphics::ShaderProgramType::Vertex:
        return "vmain";
    case Core::Graphics::ShaderProgramType::Hull:
        return "hmain";
    case Core::Graphics::ShaderProgramType::Domain:
        return "dmain";
    case Core::Graphics::ShaderProgramType::Pixel:
        return "pmain";
    case Core::Graphics::ShaderProgramType::Geometry:
        return "gmain";
    case Core::Graphics::ShaderProgramType::Compute:
        return "cmain";
    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
