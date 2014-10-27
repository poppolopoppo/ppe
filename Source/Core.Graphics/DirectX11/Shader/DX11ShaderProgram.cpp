#include "stdafx.h"

#include "DX11ShaderProgram.h"

#include "DirectX11/DX11CompilerIncludes.h"
#include "DirectX11/DX11DeviceEncapsulator.h"

#include "Device/DeviceAPIEncapsulator.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/Geometry/VertexDeclaration.h"
#include "Device/Shader/ShaderSource.h"
#include "Device/Shader/VertexSubstitutions.h"

#include "Device/BindName.h"
#include "Device/Shader/ConstantBufferLayout.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Allocator/ThreadLocalHeap.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FS/Dirpath.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/VFS/VirtualFileSystemStream.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Memory/UniqueView.h"

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class ShaderIncludeHandler_ : public ::ID3DInclude {
public:
    ShaderIncludeHandler_(  const Dirpath& systemDir,
                            const Graphics::ShaderSource *source,
                            const Graphics::VertexDeclaration *vertexDeclaration);
    ~ShaderIncludeHandler_();

    ShaderIncludeHandler_(const ShaderIncludeHandler_& ) = delete;
    ShaderIncludeHandler_& operator =(const ShaderIncludeHandler_& ) = delete;

    HRESULT __stdcall Open(::D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) override;
    HRESULT __stdcall Close(LPCVOID pData) override;

private:
    void Open_(const Filename& filename,  LPCVOID *ppData, UINT *pBytes);
    void GenerateAutomaticSubstitutions_(LPCVOID *ppData, UINT *pBytes);

    static void *Allocate_(size_t sizeInBytes) { return GetThreadLocalHeap().malloc(sizeInBytes, MEMORY_DOMAIN_TRACKING_DATA(Shader)); }
    static void Deallocate_(void *ptr) { GetThreadLocalHeap().free(ptr); }

    const Dirpath _systemDir;
    const Graphics::ShaderSource *_source;
    const Graphics::VertexDeclaration *_vertexDeclaration;
};
//----------------------------------------------------------------------------
ShaderIncludeHandler_::ShaderIncludeHandler_(
    const Dirpath& systemDir,
    const Graphics::ShaderSource *source,
    const Graphics::VertexDeclaration *vertexDeclaration )
:   _systemDir(systemDir)
,   _source(source)
,   _vertexDeclaration(vertexDeclaration) {
    Assert(VirtualFileSystem::Instance().DirectoryExists(_systemDir));
    Assert(source);
    Assert(vertexDeclaration);
}
//----------------------------------------------------------------------------
ShaderIncludeHandler_::~ShaderIncludeHandler_() {}
//----------------------------------------------------------------------------
HRESULT __stdcall ShaderIncludeHandler_::Open(::D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) {
    *ppData = nullptr;
    *pBytes = 0;

    if (0 == CompareI(pFileName, Graphics::ShaderSource::AppIn_SubstitutionName())) {
        GenerateAutomaticSubstitutions_(ppData, pBytes);
        return S_OK;
    }

    FileSystem::char_type buffer[2048];
    const FileSystem::char_type sep = FileSystem::char_type(FileSystem::Separator);

    switch (IncludeType)
    {
    case ::D3D_INCLUDE_SYSTEM:
        Format(buffer, L"{0}{1}{2}", _systemDir, sep, pFileName);
        break;
    case ::D3D_INCLUDE_LOCAL:
        Format(buffer, L"{0}{1}{2}", _source->Filename().Dirpath(), sep, pFileName);
        break;
    default:
        AssertNotImplemented();
        return E_FAIL;
    }

    const Filename includeFilename(buffer);
    if (!VirtualFileSystem::Instance().FileExists(includeFilename))
        return E_FAIL;

    Open_(includeFilename, ppData, pBytes);
    return S_OK;
}
//----------------------------------------------------------------------------
HRESULT __stdcall ShaderIncludeHandler_::Close(LPCVOID pData) {
    if (pData)
        Deallocate_(const_cast<void *>(pData));

    return S_OK;
}
//----------------------------------------------------------------------------
void ShaderIncludeHandler_::Open_(const Filename& filename, LPCVOID *ppData, UINT *pBytes) {
    const auto file = VirtualFileSystem::Instance().OpenReadable(filename, AccessPolicy::Mode(AccessPolicy::Ate|AccessPolicy::Binary) );
    AssertRelease(file);

    const size_t sizeInBytes = checked_cast<size_t>(file->TellI()); // Ate
    void *const storage = Allocate_(sizeInBytes);

    file->SeekI(0);
    file->Read(storage, sizeInBytes);

    *ppData = storage;
    *pBytes = checked_cast<UINT>(sizeInBytes);
}
//----------------------------------------------------------------------------
void ShaderIncludeHandler_::GenerateAutomaticSubstitutions_(LPCVOID *ppData, UINT *pBytes) {
    VECTOR_THREAD_LOCAL(Shader, Pair<String COMMA String>) substitutions;
    _source->FillSubstitutions(substitutions, _vertexDeclaration);

    ThreadLocalOStringStream oss;

    oss << "#ifndef __AppIn_SubstitutionName_INCLUDED" << std::endl
        << "#define __AppIn_SubstitutionName_INCLUDED" << std::endl
        << std::endl
        << "/* automatically generated by shader compiler */" << std::endl
        << std::endl;

    for (const Pair<String COMMA String>& s : substitutions)
        oss << "#define " << s.first << " " << s.second << std::endl;

    oss << std::endl
        << "/* end of automatically generated content */" << std::endl
        << std::endl
        << "#endif //!__AppIn_SubstitutionName_INCLUDED" << std::endl
        << std::endl;

    const size_t sizeInBytes = oss.str().size();
    void *const storage = Allocate_(sizeInBytes);

    ::memcpy(storage, oss.str().c_str(), sizeInBytes);

    *ppData = storage;
    *pBytes = checked_cast<UINT>(sizeInBytes);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ShaderProgram::ShaderProgram(   IDeviceAPIShaderCompilerEncapsulator *compiler,
                                Graphics::ShaderProgram *owner,
                                const char *entryPoint,
                                ShaderCompilerFlags flags,
                                const Graphics::ShaderSource *source,
                                const Graphics::VertexDeclaration *vertexDeclaration)
:   DeviceAPIDependantShaderProgram(compiler, owner, entryPoint, flags, source, vertexDeclaration) {
    const MemoryView<const char> sourceCode = source->SourceCode();

    ShaderIncludeHandler_ dx11Include(ShaderSource::SystemDirpath(), source, vertexDeclaration);
    const ::D3D_SHADER_MACRO dx11Defines[] = {
        {"DIRECTX11", "1"},
        {nullptr, nullptr},
    };

    const char *dx11Target = ShaderProfileTypeToD3D11Target(owner->ProgramType(), owner->ProfileType());
    const UINT dx11Compile = ShaderCompilerFlagsToD3D11CompileFlags(flags);

    ComPtr<::ID3DBlob> errors = nullptr;
    ComPtr<::ID3DBlob> compiled = nullptr;

    if ( FAILED(::D3DCompile(
                    sourceCode.Pointer(),
                    sourceCode.SizeInBytes(),
                    source->SourceName().c_str(),
                    dx11Defines,
                    &dx11Include,
                    entryPoint,
                    dx11Target,
                    dx11Compile,
                    0,
                    compiled.GetAddressOf(),
                    errors.GetAddressOf()
        )) ) {
        AssertRelease(errors);

        const char *msg = reinterpret_cast<const char *>(errors->GetBufferPointer());
        throw ShaderCompilerEncapsulatorException(msg, compiler, owner, source);
    }
    Assert(!errors);

    if (size_t(flags) & size_t(ShaderCompilerFlags::NoOptimize) ) {
        // shader is not stripped when NoOptimize is set (debug)
        _entity = compiled;
    }
    else {
        ComPtr<::ID3DBlob> striped = nullptr;
        if ( FAILED(::D3DStripShader(
                        compiled->GetBufferPointer(),
                        compiled->GetBufferSize(),
                        //::D3DCOMPILER_STRIP_REFLECTION_DATA | // disables further ::D3DReflect() calls obviously
                        ::D3DCOMPILER_STRIP_DEBUG_INFO |
                        ::D3DCOMPILER_STRIP_PRIVATE_DATA |
                        ::D3DCOMPILER_STRIP_TEST_BLOBS,
                        striped.GetAddressOf()
            )) ) {
            throw ShaderCompilerEncapsulatorException("failed to strip shader program", compiler, owner, source);
        }

        _entity = striped;
    }
    Assert(_entity);
}
//----------------------------------------------------------------------------
ShaderProgram::~ShaderProgram() {
    ReleaseComRef(_entity);
}
//----------------------------------------------------------------------------
void ShaderProgram::Preprocess(
    IDeviceAPIShaderCompilerEncapsulator *compiler,
    RAWSTORAGE(Shader, char)& output,
    const Graphics::ShaderProgram *owner,
    const Graphics::ShaderSource *source,
    const Graphics::VertexDeclaration *vertexDeclaration) {
    const MemoryView<const char> sourceCode = source->SourceCode();

    ShaderIncludeHandler_ dx11Include(ShaderSource::SystemDirpath(), source, vertexDeclaration);
    const ::D3D_SHADER_MACRO dx11Defines[] = {
        {"DIRECTX11", "1"},
        {nullptr, nullptr},
    };

    ComPtr<::ID3DBlob> errors = nullptr;
    ComPtr<::ID3DBlob> preprocessed = nullptr;

    if ( FAILED(::D3DPreprocess(
                    sourceCode.Pointer(),
                    sourceCode.SizeInBytes(),
                    source->SourceName().c_str(),
                    dx11Defines,
                    &dx11Include,
                    preprocessed.GetAddressOf(),
                    errors.GetAddressOf()
        )) ) {
        AssertRelease(errors);

        const char *msg = reinterpret_cast<const char *>(errors->GetBufferPointer());
        throw ShaderCompilerEncapsulatorException(msg, compiler, owner, source);
    }
    Assert(!errors);
    Assert(preprocessed);

    output.Resize_DiscardData(preprocessed->GetBufferSize());
    memcpy(output.Pointer(), preprocessed->GetBufferPointer(), output.SizeInBytes());
}
//----------------------------------------------------------------------------
void ShaderProgram::Reflect(
    IDeviceAPIShaderCompilerEncapsulator *compiler,
    ASSOCIATIVE_VECTOR(Shader, BindName, PCConstantBufferLayout)& constants,
    VECTOR(Shader, BindName)& textures,
    const Graphics::ShaderProgram *program) {
    const DX11::ShaderProgram *const dx11Program = checked_cast<const DX11::ShaderProgram *>(
        program->DeviceAPIDependantProgram().get());

    ::ID3D11ShaderReflection *dx11Reflector = nullptr;
    if (FAILED(::D3DReflect(
        dx11Program->_entity->GetBufferPointer(),
        dx11Program->_entity->GetBufferSize(),
        ::IID_ID3D11ShaderReflection,
        (void **)&dx11Reflector
        )) ) {
        throw ShaderCompilerEncapsulatorException("failed to reflect shader program", compiler, program, nullptr);
    }
    Assert(dx11Reflector);

    ::D3D11_SHADER_DESC dx11ShaderDesc;
    if (FAILED(dx11Reflector->GetDesc(&dx11ShaderDesc)))
        AssertNotReached();

    constants.reserve(dx11ShaderDesc.ConstantBuffers);
    textures.reserve(dx11ShaderDesc.BoundResources/* more than needed, sadly :( */);

    for (UINT i = 0; i < dx11ShaderDesc.ConstantBuffers; ++i) {
        ::ID3D11ShaderReflectionConstantBuffer *dx11Reflector_constantBuffer = nullptr;
        dx11Reflector_constantBuffer = dx11Reflector->GetConstantBufferByIndex(i);
        Assert(dx11Reflector_constantBuffer);

        ::D3D11_SHADER_BUFFER_DESC dx11ConstantDesc;
        if (FAILED(dx11Reflector_constantBuffer->GetDesc(&dx11ConstantDesc)) )
            AssertNotReached();

        PConstantBufferLayout const layout = new ConstantBufferLayout(checked_cast<size_t>(dx11ConstantDesc.Size));

        for (UINT j = 0; j < dx11ConstantDesc.Variables; ++j) {
            ::ID3D11ShaderReflectionVariable *dx11Reflector_variable = nullptr;
            dx11Reflector_variable = dx11Reflector_constantBuffer->GetVariableByIndex(j);
            Assert(dx11Reflector_variable);

            ::D3D11_SHADER_VARIABLE_DESC dx11VariableDesc;
            if (FAILED(dx11Reflector_variable->GetDesc(&dx11VariableDesc)) )
                AssertNotReached();

            const bool inUse = (::D3D_SVF_USED & dx11VariableDesc.uFlags) == ::D3D_SVF_USED;

            // TODO : handle integral data !
            ConstantFieldType type;
            switch (dx11VariableDesc.Size)
            {
            case sizeof(float)*1:
                type = ConstantFieldType::Float;
                break;
            case sizeof(float)*2:
                type = ConstantFieldType::Float2;
                break;
            case sizeof(float)*3:
                type = ConstantFieldType::Float3;
                break;
            case sizeof(float)*4:
                type = ConstantFieldType::Float4;
                break;
            case sizeof(float)*3*3:
                type = ConstantFieldType::Float3x3;
                break;
            case sizeof(float)*4*3:
                type = ConstantFieldType::Float4x3;
                break;
            case sizeof(float)*4*4:
                type = ConstantFieldType::Float4x4;
                break;

            default: AssertNotImplemented();
            }

            layout->AddField(   dx11VariableDesc.Name,
                                type,
                                dx11VariableDesc.StartOffset,
                                dx11VariableDesc.Size,
                                inUse );
        }

        Assert(layout->Count() > 0);
        constants.Insert_AssertUnique(dx11ConstantDesc.Name, layout);
    }

    for (UINT i = 0; i < dx11ShaderDesc.BoundResources; ++i) {
        ::D3D11_SHADER_INPUT_BIND_DESC dx11ResourceDesc;
        if (FAILED(dx11Reflector->GetResourceBindingDesc(i, &dx11ResourceDesc)) )
            AssertNotReached();

        if (::D3D_SIT_TEXTURE != dx11ResourceDesc.Type)
            continue;

        AssertRelease(1 == dx11ResourceDesc.BindCount);
        AssertRelease(::D3D_SRV_DIMENSION_TEXTURE2D == dx11ResourceDesc.Dimension);

        AssertRelease(dx11ResourceDesc.BindPoint == textures.size());
        textures.emplace_back(dx11ResourceDesc.Name);
    }
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(ShaderProgram, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
UINT ShaderCompilerFlagsToD3D11CompileFlags(ShaderCompilerFlags value) {
    UINT result = 0;
    if (size_t(ShaderCompilerFlags::Debug) == (size_t(ShaderCompilerFlags::Debug) & size_t(value)) )
        result |= D3DCOMPILE_DEBUG;
    if (size_t(ShaderCompilerFlags::Optimize) == (size_t(ShaderCompilerFlags::Optimize) & size_t(value)) )
        result |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    else if (size_t(ShaderCompilerFlags::NoOptimize) == (size_t(ShaderCompilerFlags::NoOptimize) & size_t(value)) )
        result |= D3DCOMPILE_SKIP_OPTIMIZATION;
    if (size_t(ShaderCompilerFlags::Pedantic) == (size_t(ShaderCompilerFlags::Pedantic) & size_t(value)) )
        result |= D3DCOMPILE_ENABLE_STRICTNESS;
    if (size_t(ShaderCompilerFlags::WError) == (size_t(ShaderCompilerFlags::WError) & size_t(value)) )
        result |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
    return result;
}
//----------------------------------------------------------------------------
ShaderCompilerFlags D3D11CompileFlagsToShaderCompilerFlags(UINT  value) {
    ShaderCompilerFlags result = ShaderCompilerFlags::None;
    if (D3DCOMPILE_DEBUG == (D3DCOMPILE_DEBUG & value) )
        result = static_cast<ShaderCompilerFlags>(size_t(result) | size_t(ShaderCompilerFlags::Debug));
    if (D3DCOMPILE_OPTIMIZATION_LEVEL3 == (D3DCOMPILE_OPTIMIZATION_LEVEL3 & value) )
        result = static_cast<ShaderCompilerFlags>(size_t(result) | size_t(ShaderCompilerFlags::Optimize));
    else if (D3DCOMPILE_SKIP_OPTIMIZATION == (D3DCOMPILE_SKIP_OPTIMIZATION & value) )
        result = static_cast<ShaderCompilerFlags>(size_t(result) | size_t(ShaderCompilerFlags::NoOptimize));
    if (D3DCOMPILE_ENABLE_STRICTNESS == (D3DCOMPILE_ENABLE_STRICTNESS & value) )
        result = static_cast<ShaderCompilerFlags>(size_t(result) | size_t(ShaderCompilerFlags::Pedantic));
    if (D3DCOMPILE_WARNINGS_ARE_ERRORS == (D3DCOMPILE_WARNINGS_ARE_ERRORS & value) )
        result = static_cast<ShaderCompilerFlags>(size_t(result) | size_t(ShaderCompilerFlags::WError));
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
LPCSTR ShaderProfileTypeToD3D11Target(ShaderProgramType program, ShaderProfileType profile) {
    switch (profile)
    {
    case Core::Graphics::ShaderProfileType::ShaderModel5:
        switch (program)
        {
        case Core::Graphics::ShaderProgramType::Vertex:
            return "vs_5_0";
        case Core::Graphics::ShaderProgramType::Hull:
            return "hs_5_0";
        case Core::Graphics::ShaderProgramType::Domain:
            return "ds_5_0";
        case Core::Graphics::ShaderProgramType::Pixel:
            return "ps_5_0";
        case Core::Graphics::ShaderProgramType::Geometry:
            return "gs_5_0";
        case Core::Graphics::ShaderProgramType::Compute:
            return "cs_5_0";
        default:
            AssertNotImplemented();
        }
        break;
    case Core::Graphics::ShaderProfileType::ShaderModel4_1:
        switch (program)
        {
        case Core::Graphics::ShaderProgramType::Vertex:
            return "vs_4_1";
        case Core::Graphics::ShaderProgramType::Pixel:
            return "ps_4_1";
        case Core::Graphics::ShaderProgramType::Geometry:
            return "gs_4_1";
        case Core::Graphics::ShaderProgramType::Compute:
            return "cs_4_1";
        default:
            AssertNotImplemented();
        }
        break;
    case Core::Graphics::ShaderProfileType::ShaderModel4:
        switch (program)
        {
        case Core::Graphics::ShaderProgramType::Vertex:
            return "vs_4_0";
        case Core::Graphics::ShaderProgramType::Pixel:
            return "ps_4_0";
        case Core::Graphics::ShaderProgramType::Geometry:
            return "gs_4_0";
        case Core::Graphics::ShaderProgramType::Compute:
            return "cs_4_0";
        default:
            AssertNotImplemented();
        }
        break;
    case Core::Graphics::ShaderProfileType::ShaderModel3:
        switch (program)
        {
        case Core::Graphics::ShaderProgramType::Vertex:
            return "vs_3_0";
        case Core::Graphics::ShaderProgramType::Pixel:
            return "ps_3_0";
        default:
            AssertNotImplemented();
        }
        break;
    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
